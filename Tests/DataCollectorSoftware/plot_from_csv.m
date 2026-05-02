% --- Selezione file tramite finestra di dialogo ---
[filename, filepath] = uigetfile('*.csv', 'Seleziona il file dati sensori');
if isequal(filename, 0)
    disp('Nessun file selezionato. Uscita.');
    return;
end
fullpath = fullfile(filepath, filename);

% --- Lettura CSV: salta righe non numeriche (es. "ESP32 Bridge connesso") ---
% Leggi tutte le righe come testo
fid = fopen(fullpath, 'r');
rawLines = {};
while ~feof(fid)
    ln = fgetl(fid);
    if ischar(ln)
        rawLines{end+1} = ln; %#ok<AGROW>
    end
end
fclose(fid);

% Tieni header (riga 1) + righe che iniziano con una cifra
keep = false(1, numel(rawLines));
keep(1) = true;
for i = 2:numel(rawLines)
    keep(i) = ~isempty(regexp(rawLines{i}, '^\d', 'once'));
end

% Scrivi su file temporaneo pulito e leggi con readtable
tmpFile = [tempname '.csv'];
fid = fopen(tmpFile, 'w');
for i = find(keep)
    fprintf(fid, '%s\n', rawLines{i});
end
fclose(fid);
data = readtable(tmpFile);
delete(tmpFile);

% --- Asse temporale in secondi ---
% Supporta sia CSV Feather (Time_s) sia CSV ESP32 (Time_ms)
cols = data.Properties.VariableNames;
if ismember('Time_ms', cols)
    tempo = data.Time_ms / 1000;
elseif ismember('Time_s', cols)
    tempo = data.Time_s;
else
    error('Colonna temporale non trovata (atteso Time_ms o Time_s).');
end

% --- Estrazione colonne obbligatorie ---
% Colonne CSV: Time_ms/Time_s, TempMotor, TempInverter, TempIGBT,
%              Voltage, Speed, Iq, Id, TorqueMotor, PedalPerc,
%              InvState (*), ErrCode, StatusWord, ErrInfo1,
%              PhaseU_mA, PhaseV_mA, PhaseW_mA, Power_W
%              NTC1, NTC2, NTC3 (*) — solo backend Feather
%  (*) = colonna opzionale

tempMotor    = data.TempMotor;
tempInverter = data.TempInverter;
tempIGBT     = data.TempIGBT;
voltage      = data.Voltage;
speed        = data.Speed;
iq           = data.Iq;
id           = data.Id;
torque       = data.TorqueMotor;
pedal        = data.PedalPerc;

errCode   = data.ErrCode;
statusWord = data.StatusWord;
errInfo1  = data.ErrInfo1;
phaseU_mA = data.PhaseU_mA / 1000;   % mA -> A
phaseV_mA = data.PhaseV_mA / 1000;
phaseW_mA = data.PhaseW_mA / 1000;
power_W   = data.Power_W;

% InvState: presente solo nel backend ESP32
if ismember('InvState', cols)
    invState = data.InvState;
else
    invState = [];
end

% NTC: presenti solo nel backend Feather
hasNTC = ismember('NTC1', cols);
if hasNTC
    ntc1 = data.NTC1;
    ntc2 = data.NTC2;
    ntc3 = data.NTC3;
    % Sostituisci valori sentinella (-999) con NaN
    ntc1(ntc1 == -999) = NaN;
    ntc2(ntc2 == -999) = NaN;
    ntc3(ntc3 == -999) = NaN;
end

% =========================================================================
% FIGURA 1 - Temperature
% =========================================================================
figure('Name', ['Temperature - ' filename]);
subplot(2,1,1);
hold on; grid on;
plot(tempo, tempMotor,    'r',  'LineWidth', 1.5, 'DisplayName', 'T Motore');
plot(tempo, tempInverter, 'b',  'LineWidth', 1.5, 'DisplayName', 'T Inverter');
plot(tempo, tempIGBT,     'm',  'LineWidth', 1.5, 'DisplayName', 'T IGBT');
title('Temperature Motore / Inverter / IGBT');
xlabel('Tempo (s)'); ylabel('Temperatura (°C)');
legend('Location', 'best'); hold off;

if hasNTC
    subplot(2,1,2);
    hold on; grid on;
    plot(tempo, ntc1, 'r',  'LineWidth', 1.5, 'DisplayName', 'NTC 1');
    plot(tempo, ntc2, 'g',  'LineWidth', 1.5, 'DisplayName', 'NTC 2');
    plot(tempo, ntc3, 'b',  'LineWidth', 1.5, 'DisplayName', 'NTC 3');
    title('Temperature NTC');
    xlabel('Tempo (s)'); ylabel('Temperatura (°C)');
    legend('Location', 'best'); hold off;
end

% =========================================================================
% FIGURA 2 - Segnali motore
% =========================================================================
figure('Name', ['Segnali Motore - ' filename]);
subplot(3,1,1);
hold on; grid on;
plot(tempo, speed, 'b', 'LineWidth', 1.5, 'DisplayName', 'Speed');
title('Velocità Motore'); xlabel('Tempo (s)'); ylabel('RPM');
legend('Location', 'best'); hold off;

subplot(3,1,2);
hold on; grid on;
plot(tempo, iq, 'r',  'LineWidth', 1.5, 'DisplayName', 'Iq');
plot(tempo, id, 'b',  'LineWidth', 1.5, 'DisplayName', 'Id');
title('Correnti Iq / Id'); xlabel('Tempo (s)'); ylabel('Corrente (A)');
legend('Location', 'best'); hold off;

subplot(3,1,3);
hold on; grid on;
plot(tempo, torque, 'g', 'LineWidth', 1.5, 'DisplayName', 'Coppia');
title('Coppia Motore'); xlabel('Tempo (s)'); ylabel('Coppia (Nm)');
legend('Location', 'best'); hold off;

% =========================================================================
% FIGURA 3 - Tensione e Pedale
% =========================================================================
figure('Name', ['Tensione e Pedale - ' filename]);
subplot(2,1,1);
hold on; grid on;
plot(tempo, voltage, 'r', 'LineWidth', 1.5, 'DisplayName', 'Tensione');
title('Tensione Bus DC'); xlabel('Tempo (s)'); ylabel('Tensione (V)');
legend('Location', 'best'); hold off;

subplot(2,1,2);
hold on; grid on;
plot(tempo, pedal, 'c', 'LineWidth', 1.5, 'DisplayName', 'Pedale (%)');
title('Posizione Pedale'); xlabel('Tempo (s)'); ylabel('%');
legend('Location', 'best'); hold off;

% =========================================================================
% FIGURA 4 - Correnti di fase e Potenza
% =========================================================================
figure('Name', ['Correnti di Fase e Potenza - ' filename]);
subplot(2,1,1);
hold on; grid on;
plot(tempo, phaseU_mA, 'r',  'LineWidth', 1.5, 'DisplayName', 'Fase U');
plot(tempo, phaseV_mA, 'g',  'LineWidth', 1.5, 'DisplayName', 'Fase V');
plot(tempo, phaseW_mA, 'b',  'LineWidth', 1.5, 'DisplayName', 'Fase W');
title('Correnti di Fase'); xlabel('Tempo (s)'); ylabel('Corrente (A)');
legend('Location', 'best'); hold off;

subplot(2,1,2);
hold on; grid on;
plot(tempo, power_W, 'k', 'LineWidth', 1.5, 'DisplayName', 'Potenza');
title('Potenza Inverter'); xlabel('Tempo (s)'); ylabel('Potenza (W)');
legend('Location', 'best'); hold off;

% =========================================================================
% FIGURA 5 - Stato Inverter ed Errori
% =========================================================================
figure('Name', ['Stato Inverter - ' filename]);

if ~isempty(invState)
    nRows = 4;
else
    nRows = 3;
end
spIdx = 1;

if ~isempty(invState)
    subplot(nRows,1,spIdx); spIdx = spIdx + 1;
    hold on; grid on;
    stairs(tempo, invState, 'b', 'LineWidth', 1.5, 'DisplayName', 'Stato Inverter');
    title('Stato Inverter (InvState)');
    xlabel('Tempo (s)'); ylabel('Stato');
    yticks([-1 0 1 2 3 4 5]);
    yticklabels({'OFF','IDLE','LV\_ACTIVE','HV\_ACTIVE','READY','RUNNING','ERROR'});
    legend('Location', 'best'); hold off;
end

subplot(nRows,1,spIdx); spIdx = spIdx + 1;
hold on; grid on;
stairs(tempo, errCode,   'r', 'LineWidth', 1.5, 'DisplayName', 'ErrCode');
stairs(tempo, errInfo1,  'm', 'LineWidth', 1.0, 'DisplayName', 'ErrInfo1');
title('Codici Errore'); xlabel('Tempo (s)'); ylabel('Codice');
legend('Location', 'best'); hold off;

subplot(nRows,1,spIdx); spIdx = spIdx + 1;
hold on; grid on;
stairs(tempo, bitand(statusWord, 256),  'b',  'LineWidth', 1.0, 'DisplayName', 'SystemReady (b8)');
stairs(tempo, bitand(statusWord, 512),  'r',  'LineWidth', 1.0, 'DisplayName', 'Error (b9)');
stairs(tempo, bitand(statusWord, 1024), 'm',  'LineWidth', 1.0, 'DisplayName', 'Warn (b10)');
stairs(tempo, bitand(statusWord, 32768),'k',  'LineWidth', 1.0, 'DisplayName', 'Derating (b15)');
title('Status Word — bit significativi'); xlabel('Tempo (s)'); ylabel('bit × valore');
legend('Location', 'best'); hold off;

subplot(nRows,1,spIdx);
hold on; grid on;
stairs(tempo, statusWord, 'k', 'LineWidth', 1.2, 'DisplayName', 'StatusWord (raw)');
title('Status Word (raw hex)'); xlabel('Tempo (s)'); ylabel('Valore');
legend('Location', 'best'); hold off;