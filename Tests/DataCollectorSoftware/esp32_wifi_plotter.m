% esp32_wifi_plotter.m
% Visualizzazione log CSV acquisiti via ESP32 Wi-Fi Bridge
% Polimarcheracing EV Powertrain — formato dati_sensori_YYYYMMDD_HHmmss.csv
%
% Colonne attese (23):
%   Time_ms | TempMotor | TempInverter | TempIGBT | Voltage | Speed |
%   Iq | Id | TorqueMotor | PedalPerc | InvState | ErrCode | StatusWord |
%   ErrInfo1 | PhaseU_mA | PhaseV_mA | PhaseW_mA | Power_W |
%   TorqueSetpoint | TorqueLimitDyn | NTC1 | NTC2 | NTC3

clc; clear; close all;

%% ---- Selezione file -------------------------------------------------------
[fname, fpath] = uigetfile( ...
    {'dati_sensori_*.csv','Log ESP32 (dati_sensori_*.csv)'; ...
     '*.csv','Tutti i file CSV (*.csv)'}, ...
    'Seleziona il file log ESP32', ...
    fullfile(fileparts(mfilename('fullpath')), 'logs'));

if isequal(fname, 0)
    disp('Nessun file selezionato. Uscita.');
    return;
end
fullpath = fullfile(fpath, fname);
fprintf('Caricamento: %s\n', fullpath);

%% ---- Lettura robusta del CSV ---------------------------------------------
% Alcune righe possono contenere testo (es. "ESP32 Bridge connesso");
% vengono scartate automaticamente.

fid = fopen(fullpath, 'r');
rawLines = {};
while ~feof(fid)
    ln = fgetl(fid);
    if ischar(ln)
        rawLines{end+1} = ln; %#ok<AGROW>
    end
end
fclose(fid);

% Rimuovi \r residui (file prodotti su Windows)
rawLines = cellfun(@(l) strrep(l, char(13), ''), rawLines, 'UniformOutput', false);

% Intestazioni dalla prima riga
headerNames = strtrim(strsplit(strtrim(rawLines{1}), ','));
nCols       = numel(headerNames);

% Pattern per numeri validi (intero, float, notaz. scientifica, NaN)
validNumPat = '^-?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$|^NaN$|^nan$';

% Filtra righe: tiene solo quelle con almeno nCols-2 campi tutti numerici
keep = false(1, numel(rawLines));
keep(1) = true;  % intestazione
for i = 2:numel(rawLines)
    fields = strtrim(strsplit(strtrim(rawLines{i}), ','));
    if numel(fields) < nCols - 2
        continue;   % riga troppo corta o testuale
    end
    nCheck = min(numel(fields), nCols);
    allNum = all(cellfun(@(f) isempty(f) || ...
        ~isempty(regexp(f, validNumPat, 'once')), fields(1:nCheck)));
    if allNum
        keep(i) = true;
    end
end

% Ricostruisci buffer filtrato e passa a readtable tramite file temporaneo
filteredLines = rawLines(keep);
tmpFile = [tempname, '.csv'];
fid = fopen(tmpFile, 'w');
fprintf(fid, '%s\n', filteredLines{:});
fclose(fid);

opts = detectImportOptions(tmpFile, 'NumHeaderLines', 0);
opts.VariableNamesLine = 1;
opts.DataLines         = [2, Inf];
opts.Delimiter         = ',';
opts                   = setvartype(opts, opts.VariableNames, 'double');
data                   = readtable(tmpFile, opts);
delete(tmpFile);

N = height(data);
fprintf('Campioni validi: %d  |  Variabili: %d\n', N, width(data));

%% ---- Asse temporale -------------------------------------------------------
% Time_ms è il timestamp in millisecondi (0 per i primi frame prima del boot)
t_ms = data.Time_ms;

% Se la colonna è interamente zero, usa l'indice campione × 500 ms (default rate)
if all(t_ms == 0)
    warning('Time_ms è tutto zero: uso indice campione × 500 ms come fallback.');
    t_ms = (0:N-1)' * 500;
end

t_s = t_ms / 1e3;   % secondi

%% ===========================================================================
%  FIGURA 1 — Velocità & Coppia
%% ===========================================================================
figure('Name', 'Velocità & Coppia', 'NumberTitle', 'off', 'Color', 'w');

subplot(3,1,1);
plot(t_s, data.Speed, 'b', 'LineWidth', 1.3);
grid on; ylabel('n  [rpm]'); title('Velocità Motore');

subplot(3,1,2);
plot(t_s, data.TorqueMotor,    'r',  'LineWidth', 1.3, 'DisplayName', 'Coppia motore'); hold on;
plot(t_s, data.TorqueSetpoint, 'r--','LineWidth', 0.9, 'DisplayName', 'Setpoint');
plot(t_s, data.TorqueLimitDyn, 'k:', 'LineWidth', 0.9, 'DisplayName', 'Limite dyn.');
grid on; ylabel('\tau  [Nm]'); title('Coppia');
legend('Location', 'best'); hold off;

subplot(3,1,3);
plot(t_s, data.PedalPerc, 'm', 'LineWidth', 1.3);
grid on; ylabel('Pedale  [%]'); title('Posizione Pedale');
xlabel('Tempo  [s]');

sgtitle(['Velocità & Coppia  —  ' fname], 'Interpreter', 'none');

%% ===========================================================================
%  FIGURA 2 — Correnti
%% ===========================================================================
figure('Name', 'Correnti', 'NumberTitle', 'off', 'Color', 'w');

subplot(3,1,1);
plot(t_s, data.Iq, 'b', 'LineWidth', 1.3, 'DisplayName', 'Iq'); hold on;
plot(t_s, data.Id, 'r', 'LineWidth', 1.3, 'DisplayName', 'Id');
grid on; ylabel('I  [A]'); title('Correnti d-q');
legend('Location', 'best'); hold off;

subplot(3,1,2);
plot(t_s, data.PhaseU_mA / 1e3, 'r', 'LineWidth', 1.2, 'DisplayName', 'U'); hold on;
plot(t_s, data.PhaseV_mA / 1e3, 'g', 'LineWidth', 1.2, 'DisplayName', 'V');
plot(t_s, data.PhaseW_mA / 1e3, 'b', 'LineWidth', 1.2, 'DisplayName', 'W');
grid on; ylabel('I  [A]'); title('Correnti di Fase (U, V, W)');
legend('Location', 'best'); hold off;

subplot(3,1,3);
plot(t_s, data.Power_W, 'k', 'LineWidth', 1.3);
grid on; ylabel('P  [W]'); title('Potenza Elettrica');
xlabel('Tempo  [s]');

sgtitle(['Correnti  —  ' fname], 'Interpreter', 'none');

%% ===========================================================================
%  FIGURA 3 — Tensione Bus DC
%% ===========================================================================
figure('Name', 'Tensione Bus DC', 'NumberTitle', 'off', 'Color', 'w');

plot(t_s, data.Voltage, 'b', 'LineWidth', 1.5);
grid on;
ylabel('V  [V]');
xlabel('Tempo  [s]');
title(['Tensione Bus DC  —  ' fname], 'Interpreter', 'none');

%% ===========================================================================
%  FIGURA 4 — Temperature
%% ===========================================================================
figure('Name', 'Temperature', 'NumberTitle', 'off', 'Color', 'w');

subplot(2,1,1);
plot(t_s, data.TempMotor,    'r',  'LineWidth', 1.3, 'DisplayName', 'Motore');    hold on;
plot(t_s, data.TempInverter, 'b',  'LineWidth', 1.3, 'DisplayName', 'Inverter');
plot(t_s, data.TempIGBT,     'g',  'LineWidth', 1.3, 'DisplayName', 'IGBT');
grid on; ylabel('T  [°C]'); title('Temperature Inverter');
legend('Location', 'best'); hold off;

subplot(2,1,2);
hasNTC = @(name) ismember(name, data.Properties.VariableNames) && ...
                 any(~isnan(data.(name)));
ntcColors = {'r','b','g','m','c','k'};
ntcPlotted = false;
for k = 1:6
    ntcName = sprintf('NTC%d', k);
    if hasNTC(ntcName)
        plot(t_s, data.(ntcName), ntcColors{k}, ...
            'LineWidth', 1.2, 'DisplayName', ntcName); hold on;
        ntcPlotted = true;
    end
end
if ntcPlotted
    grid on; ylabel('T  [°C]'); title('Temperature NTC');
    legend('Location', 'best'); hold off;
    xlabel('Tempo  [s]');
else
    text(0.5, 0.5, 'Nessun sensore NTC', 'Units','normalized', ...
        'HorizontalAlignment','center');
    hold off;
end

sgtitle(['Temperature  —  ' fname], 'Interpreter', 'none');

%% ===========================================================================
%  FIGURA 5 — Stato Inverter & Fault
%% ===========================================================================
figure('Name', 'Stato Inverter', 'NumberTitle', 'off', 'Color', 'w');

subplot(4,1,1);
stairs(t_s, data.InvState, 'b', 'LineWidth', 1.3);
grid on; ylabel('Stato'); title('InvState');
yticks(unique(data.InvState(~isnan(data.InvState))));

subplot(4,1,2);
stairs(t_s, data.ErrCode, 'r', 'LineWidth', 1.3);
grid on; ylabel('ErrCode'); title('Codice Errore');

subplot(4,1,3);
stairs(t_s, data.StatusWord, 'k', 'LineWidth', 1.3);
grid on; ylabel('StatusWord'); title('Status Word');

subplot(4,1,4);
stairs(t_s, data.ErrInfo1, 'm', 'LineWidth', 1.3);
grid on; ylabel('ErrInfo1'); title('Info Errore 1');
xlabel('Tempo  [s]');

sgtitle(['Stato Inverter  —  ' fname], 'Interpreter', 'none');

fprintf('Plot completati.\n');
