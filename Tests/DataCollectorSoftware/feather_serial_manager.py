"""
Gestione seriale Feather M4 CAN logger.

Funzioni principali:
- Lettura live dei record inviati dalla scheda
- Comandi memoria flash (STATUS, LIST, READ, DELETE, NEWLOG, FORMAT)
- Utility per cancellare automaticamente i file piu vecchi

Protocollo firmware atteso (quello Feather aggiornato):
- LIST -> FILES_START ... FILES_END con righe: /logs/log_000.csv,1234
- READ:<file> -> DATA_START ... DATA_END
- STATUS -> FLASH:..., FLASH_SIZE:..., ACTIVE_LOG:...

Il parser live e volutamente generico: inoltra la riga seriale non-protocollo
cosi il file puo essere riutilizzato in applicazioni diverse.
"""

from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime
import argparse
from pathlib import Path
from queue import Empty, Queue
import re
import threading
import time
from typing import Callable, Dict, List, Optional

import serial
import serial.tools.list_ports


@dataclass
class LogFileInfo:
    path: str
    size_bytes: int

    @property
    def name(self) -> str:
        if "/" in self.path:
            return self.path.rsplit("/", 1)[1]
        return self.path


@dataclass
class LiveRecord:
    raw_line: str
    timestamp: datetime


class FeatherSerialManager:
    def __init__(self, port: str, baudrate: int = 115200, timeout_s: float = 0.15) -> None:
        self.port = port
        self.baudrate = baudrate
        self.timeout_s = timeout_s

        self._ser: Optional[serial.Serial] = None
        self._io_lock = threading.Lock()

        self._reader_thread: Optional[threading.Thread] = None
        self._reader_running = False
        self._reader_paused = False

        self._on_live_record: Optional[Callable[[LiveRecord], None]] = None
        self._on_reader_error: Optional[Callable[[str], None]] = None

    # --------------------------- connection ---------------------------
    @staticmethod
    def list_serial_ports() -> List[str]:
        return [p.device for p in serial.tools.list_ports.comports()]

    def connect(self) -> None:
        if self._ser and self._ser.is_open:
            return
        self._ser = serial.Serial(self.port, self.baudrate, timeout=self.timeout_s)
        self._ser.reset_input_buffer()
        self._ser.reset_output_buffer()

    def disconnect(self) -> None:
        self.stop_live_reader()
        if self._ser and self._ser.is_open:
            self._ser.close()
        self._ser = None

    def is_connected(self) -> bool:
        return bool(self._ser and self._ser.is_open)

    # -------------------------- live stream ---------------------------
    def start_live_reader(
        self,
        on_live_record: Callable[[LiveRecord], None],
        on_error: Optional[Callable[[str], None]] = None,
    ) -> None:
        self._require_connection()

        self._on_live_record = on_live_record
        self._on_reader_error = on_error

        if self._reader_running:
            return

        self._reader_running = True
        self._reader_paused = False
        self._reader_thread = threading.Thread(target=self._reader_loop, daemon=True)
        self._reader_thread.start()

    def stop_live_reader(self) -> None:
        self._reader_running = False
        if self._reader_thread and self._reader_thread.is_alive():
            self._reader_thread.join(timeout=1.0)
        self._reader_thread = None

    def set_live_reader_paused(self, paused: bool) -> None:
        self._reader_paused = paused

    def _reader_loop(self) -> None:
        while self._reader_running:
            try:
                if self._reader_paused:
                    time.sleep(0.01)
                    continue

                line = self._readline_nonblocking()
                if not line:
                    continue

                record = self._parse_live_line(line)
                if record and self._on_live_record:
                    self._on_live_record(record)
            except (serial.SerialException, OSError) as exc:
                self._reader_running = False
                if self._on_reader_error:
                    self._on_reader_error(f"Errore seriale: {exc}")
            except Exception as exc:  # noqa: BLE001
                if self._on_reader_error:
                    self._on_reader_error(f"Errore reader: {exc}")

    def _parse_live_line(self, line: str) -> Optional[LiveRecord]:
        # Ignora righe di protocollo comandi o righe incomplete.
        if not line or line.startswith(("FILES_", "DATA_", "FLASH:", "ACTIVE_LOG:", "LOG_OPEN:", "OK:", "ERROR:", "UNKNOWN:")):
            return None

        return LiveRecord(
            raw_line=line,
            timestamp=datetime.now(),
        )

    # ----------------------- command interface ------------------------
    def get_status(self, timeout_s: float = 1.5) -> Dict[str, str]:
        lines = self._run_command_collect("STATUS", timeout_s=timeout_s)
        out: Dict[str, str] = {}
        for line in lines:
            if ":" in line:
                k, v = line.split(":", 1)
                out[k.strip()] = v.strip()
        return out

    def list_logs(self, timeout_s: float = 2.0) -> List[LogFileInfo]:
        lines = self._run_command_collect("LIST", timeout_s=timeout_s)

        files: List[LogFileInfo] = []
        in_block = False
        for line in lines:
            if line == "FILES_START":
                in_block = True
                continue
            if line == "FILES_END":
                break
            if not in_block:
                continue

            if "," not in line:
                continue
            path, size_s = line.rsplit(",", 1)
            try:
                size = int(size_s.strip())
            except ValueError:
                continue
            files.append(LogFileInfo(path=path.strip(), size_bytes=size))

        return files

    def read_log(self, file_name_or_path: str, timeout_s: float = 5.0) -> str:
        target = self._normalize_file_name(file_name_or_path)
        lines = self._run_command_collect(f"READ:{target}", timeout_s=timeout_s)

        content_lines: List[str] = []
        in_data = False
        for line in lines:
            if line == "DATA_START":
                in_data = True
                continue
            if line == "DATA_END":
                break
            if in_data:
                content_lines.append(line)

        return "\n".join(content_lines)

    def delete_log(self, file_name_or_path: str, timeout_s: float = 2.0) -> bool:
        target = self._normalize_file_name(file_name_or_path)
        lines = self._run_command_collect(f"DELETE:{target}", timeout_s=timeout_s)
        return any(line.startswith("OK:") for line in lines)

    def create_new_log(self, filename: Optional[str] = None, timeout_s: float = 2.0) -> str:
        command = "NEWLOG" if not filename else f"NEWLOG:{filename}"
        lines = self._run_command_collect(command, timeout_s=timeout_s)

        for line in lines:
            if line.startswith("LOG_OPEN:"):
                return line.split(":", 1)[1].strip()
        return ""

    def format_logs(self, timeout_s: float = 5.0) -> str:
        lines = self._run_command_collect("FORMAT", timeout_s=timeout_s)
        for line in lines:
            if line.startswith(("OK:", "ERROR:")):
                return line
        return ""

    def cleanup_old_logs_keep_latest(self, keep_latest: int = 10, timeout_s: float = 2.0) -> List[str]:
        if keep_latest < 0:
            raise ValueError("keep_latest deve essere >= 0")

        logs = self.list_logs(timeout_s=timeout_s)

        # Ordina per indice numerico se file in formato log_XXX.csv, altrimenti per nome.
        def sort_key(item: LogFileInfo):
            match = re.search(r"log_(\d+)", item.name)
            if match:
                return (0, int(match.group(1)))
            return (1, item.name)

        logs_sorted = sorted(logs, key=sort_key)
        to_delete = logs_sorted[:-keep_latest] if keep_latest < len(logs_sorted) else []

        deleted: List[str] = []
        for log in to_delete:
            if self.delete_log(log.name, timeout_s=timeout_s):
                deleted.append(log.name)

        return deleted

    # --------------------------- internals ----------------------------
    def _require_connection(self) -> None:
        if not (self._ser and self._ser.is_open):
            raise RuntimeError("Connessione seriale non attiva")

    def _normalize_file_name(self, file_name_or_path: str) -> str:
        text = file_name_or_path.strip()
        if not text:
            raise ValueError("Nome file vuoto")
        if "/" in text:
            return text.rsplit("/", 1)[1]
        return text

    def _run_command_collect(self, command: str, timeout_s: float) -> List[str]:
        self._require_connection()
        self.set_live_reader_paused(True)

        lines: List[str] = []
        with self._io_lock:
            assert self._ser is not None
            self._ser.reset_input_buffer()
            self._ser.write((command + "\n").encode("utf-8"))

            deadline = time.time() + timeout_s
            while time.time() < deadline:
                line = self._readline_nonblocking()
                if line is None:
                    continue
                if line:
                    lines.append(line)

                # Chiusura anticipata sui pattern noti.
                if line in ("FILES_END", "DATA_END"):
                    break
                if line.startswith(("OK:", "ERROR:", "LOG_OPEN:")):
                    # STATUS non termina con marker, quindi continua.
                    if command != "STATUS":
                        break

        self.set_live_reader_paused(False)
        return lines

    def _readline_nonblocking(self) -> Optional[str]:
        if not self._ser:
            return None
        raw = self._ser.readline()
        if not raw:
            return None
        return raw.decode("utf-8", errors="replace").strip()


def run_cli_once(port: Optional[str]) -> int:
    ports = FeatherSerialManager.list_serial_ports()
    print("Porte disponibili:", ports)

    if not ports:
        print("Nessuna porta seriale trovata")
        return 1

    selected_port = port or ports[0]
    mgr = FeatherSerialManager(port=selected_port)

    try:
        mgr.connect()
        print("Connesso a", selected_port)
        print("STATUS:", mgr.get_status())
        logs = mgr.list_logs()
        print("LOGS:", [f"{x.name} ({x.size_bytes}B)" for x in logs])
        return 0
    except Exception as exc:  # noqa: BLE001
        print("Errore:", exc)
        return 2
    finally:
        mgr.disconnect()


def run_gui() -> int:
    try:
        from PyQt6.QtCore import QTimer
        from PyQt6.QtWidgets import (
            QApplication,
            QComboBox,
            QFileDialog,
            QHBoxLayout,
            QLabel,
            QListWidget,
            QMainWindow,
            QMessageBox,
            QPushButton,
            QTextEdit,
            QVBoxLayout,
            QWidget,
        )
    except ImportError:
        print("PyQt6 non installato. Installa con: pip install pyqt6")
        return 1

    class MainWindow(QMainWindow):
        def __init__(self) -> None:
            super().__init__()
            self.setWindowTitle("Feather Serial Manager")
            self.resize(900, 560)

            self.manager: Optional[FeatherSerialManager] = None
            self.live_queue: Queue[LiveRecord] = Queue()

            root = QWidget()
            self.setCentralWidget(root)
            main_layout = QVBoxLayout(root)

            # Top controls
            top_row = QHBoxLayout()
            top_row.addWidget(QLabel("Porta:"))
            self.port_combo = QComboBox()
            top_row.addWidget(self.port_combo)

            self.refresh_btn = QPushButton("Aggiorna")
            self.refresh_btn.clicked.connect(self.refresh_ports)
            top_row.addWidget(self.refresh_btn)

            self.connect_btn = QPushButton("Connetti")
            self.connect_btn.clicked.connect(self.toggle_connection)
            top_row.addWidget(self.connect_btn)
            top_row.addStretch()
            main_layout.addLayout(top_row)

            # Command buttons
            cmd_row = QHBoxLayout()
            self.status_btn = QPushButton("Status")
            self.status_btn.clicked.connect(self.read_status)
            self.status_btn.setEnabled(False)
            cmd_row.addWidget(self.status_btn)

            self.newlog_btn = QPushButton("Nuovo Log")
            self.newlog_btn.clicked.connect(self.new_log)
            self.newlog_btn.setEnabled(False)
            cmd_row.addWidget(self.newlog_btn)

            self.list_btn = QPushButton("Lista File")
            self.list_btn.clicked.connect(self.list_logs)
            self.list_btn.setEnabled(False)
            cmd_row.addWidget(self.list_btn)

            self.read_btn = QPushButton("Leggi Selezionato")
            self.read_btn.clicked.connect(self.read_selected)
            self.read_btn.setEnabled(False)
            cmd_row.addWidget(self.read_btn)

            self.save_btn = QPushButton("Salva Selezionato su PC")
            self.save_btn.clicked.connect(self.save_selected_to_pc)
            self.save_btn.setEnabled(False)
            cmd_row.addWidget(self.save_btn)

            self.save_all_btn = QPushButton("Scarica Tutti su PC")
            self.save_all_btn.clicked.connect(self.save_all_to_pc)
            self.save_all_btn.setEnabled(False)
            cmd_row.addWidget(self.save_all_btn)

            self.delete_btn = QPushButton("Elimina Selezionato")
            self.delete_btn.clicked.connect(self.delete_selected)
            self.delete_btn.setEnabled(False)
            cmd_row.addWidget(self.delete_btn)

            main_layout.addLayout(cmd_row)

            # Files + output
            mid_row = QHBoxLayout()
            self.file_list = QListWidget()
            mid_row.addWidget(self.file_list, 1)

            self.output_text = QTextEdit()
            self.output_text.setReadOnly(True)
            mid_row.addWidget(self.output_text, 2)
            main_layout.addLayout(mid_row)

            self.statusBar().showMessage("Pronto")
            self.refresh_ports()

            self.live_timer = QTimer(self)
            self.live_timer.setInterval(100)
            self.live_timer.timeout.connect(self.drain_live_queue)
            self.live_timer.start()

        def log(self, text: str) -> None:
            self.output_text.append(text)

        def set_connected_ui(self, connected: bool) -> None:
            self.connect_btn.setText("Disconnetti" if connected else "Connetti")
            self.status_btn.setEnabled(connected)
            self.newlog_btn.setEnabled(connected)
            self.list_btn.setEnabled(connected)
            self.read_btn.setEnabled(connected)
            self.save_btn.setEnabled(connected)
            self.save_all_btn.setEnabled(connected)
            self.delete_btn.setEnabled(connected)

        def refresh_ports(self) -> None:
            self.port_combo.clear()
            for p in FeatherSerialManager.list_serial_ports():
                self.port_combo.addItem(p)

        def toggle_connection(self) -> None:
            if self.manager and self.manager.is_connected():
                self.manager.disconnect()
                self.manager = None
                self.set_connected_ui(False)
                self.statusBar().showMessage("Disconnesso")
                return

            port = self.port_combo.currentText().strip()
            if not port:
                QMessageBox.warning(self, "Errore", "Seleziona una porta")
                return

            try:
                self.manager = FeatherSerialManager(port=port)
                self.manager.connect()
                self.manager.start_live_reader(self.on_live_record, self.on_reader_error)
                self.set_connected_ui(True)
                self.statusBar().showMessage(f"Connesso a {port}")
                self.log(f"Connesso a {port}")
            except Exception as exc:  # noqa: BLE001
                self.manager = None
                QMessageBox.critical(self, "Errore", str(exc))

        def on_live_record(self, rec: LiveRecord) -> None:
            self.live_queue.put(rec)

        def on_reader_error(self, message: str) -> None:
            self.log(message)

        def drain_live_queue(self) -> None:
            last: Optional[LiveRecord] = None
            while True:
                try:
                    last = self.live_queue.get_nowait()
                except Empty:
                    break

            if last is None:
                return
            self.statusBar().showMessage(f"Live: {last.raw_line[:120]}")

        def require_manager(self) -> FeatherSerialManager:
            if not self.manager or not self.manager.is_connected():
                raise RuntimeError("Nessuna connessione attiva")
            return self.manager

        def read_status(self) -> None:
            try:
                mgr = self.require_manager()
                status = mgr.get_status()
                if not status:
                    self.log("STATUS: nessuna risposta")
                    return
                for k, v in status.items():
                    self.log(f"{k}: {v}")
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def new_log(self) -> None:
            try:
                mgr = self.require_manager()
                path = mgr.create_new_log()
                self.log(f"Nuovo log: {path or 'non disponibile'}")
                self.list_logs()
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def list_logs(self) -> None:
            try:
                mgr = self.require_manager()
                logs = mgr.list_logs()
                self.file_list.clear()
                for item in logs:
                    self.file_list.addItem(f"{item.name} ({item.size_bytes} B)")
                self.log(f"File trovati: {len(logs)}")
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def _selected_file_name(self) -> Optional[str]:
            item = self.file_list.currentItem()
            if not item:
                return None
            text = item.text()
            if " (" in text:
                return text.split(" (", 1)[0]
            return text.strip()

        def read_selected(self) -> None:
            try:
                mgr = self.require_manager()
                name = self._selected_file_name()
                if not name:
                    QMessageBox.information(self, "Info", "Seleziona un file")
                    return
                content = mgr.read_log(name)
                self.log(f"----- {name} -----")
                self.log(content if content else "(vuoto)")
                self.log("-------------------")
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def save_selected_to_pc(self) -> None:
            try:
                mgr = self.require_manager()
                name = self._selected_file_name()
                if not name:
                    QMessageBox.information(self, "Info", "Seleziona un file")
                    return

                content = mgr.read_log(name)
                suggested = name if name.lower().endswith(".csv") else f"{name}.csv"
                file_path, _ = QFileDialog.getSaveFileName(
                    self,
                    "Salva file CSV",
                    suggested,
                    "CSV (*.csv);;Tutti i file (*)",
                )
                if not file_path:
                    return

                Path(file_path).write_text(content, encoding="utf-8")
                self.log(f"Salvato su PC: {file_path}")
                self.statusBar().showMessage("File salvato")
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def save_all_to_pc(self) -> None:
            try:
                mgr = self.require_manager()
                target_dir = QFileDialog.getExistingDirectory(self, "Scegli cartella destinazione")
                if not target_dir:
                    return

                logs = mgr.list_logs()
                if not logs:
                    QMessageBox.information(self, "Info", "Nessun file da scaricare")
                    return

                base = Path(target_dir)
                saved = 0
                for item in logs:
                    content = mgr.read_log(item.name)
                    out_path = base / item.name
                    out_path.write_text(content, encoding="utf-8")
                    saved += 1

                self.log(f"Scaricati {saved} file in: {base}")
                self.statusBar().showMessage(f"Scaricati {saved} file")
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def delete_selected(self) -> None:
            try:
                mgr = self.require_manager()
                name = self._selected_file_name()
                if not name:
                    QMessageBox.information(self, "Info", "Seleziona un file")
                    return
                ok = mgr.delete_log(name)
                self.log(f"Delete {name}: {'OK' if ok else 'FAIL'}")
                self.list_logs()
            except Exception as exc:  # noqa: BLE001
                QMessageBox.critical(self, "Errore", str(exc))

        def closeEvent(self, event) -> None:  # type: ignore[override]
            if self.manager:
                self.manager.disconnect()
                self.manager = None
            event.accept()

    app = QApplication([])
    window = MainWindow()
    window.show()
    return app.exec()


def main() -> int:
    parser = argparse.ArgumentParser(description="Feather serial manager")
    parser.add_argument("--cli", action="store_true", help="Esegue test CLI singolo")
    parser.add_argument("--port", type=str, default=None, help="Porta seriale, es. COM22")
    args = parser.parse_args()

    if args.cli:
        return run_cli_once(args.port)
    return run_gui()


if __name__ == "__main__":
    raise SystemExit(main())
