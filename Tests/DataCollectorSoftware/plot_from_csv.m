clear;      
clc;        
close all; 


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

% Rimuovi eventuali \r residui (file Windows letti su qualsiasi piattaforma)
rawLines = cellfun(@(l) strrep(l, char(13), ''), rawLines, 'UniformOutput', false);

% --- Determine expected column count from first valid data row -------------
% Accetta: numeri interi/float (con segno, punto decimale, NaN, notaz. sci.)
validNumPat  = '^-?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$|^NaN$|^nan$';
% Pattern per campo testuale (Source, SourceFile, ecc.) — qualsiasi stringa
% non vuota che NON sia un numero e NON inizi con cifra o segno
textFieldPat = '^[A-Za-z_]';

headerCols  = numel(strsplit(rawLines{1}, ','));
headerNames = strsplit(strtrim(rawLines{1}), ',');
headerNames = strtrim(headerNames);

% Determina quali indici di colonna sono numerici scansionando le prime
% righe dati (salta righe con meno di headerCols campi o con campi vuoti)
numericColIdx = [];
for i = 2:min(numel(rawLines), 30)
    fields = strsplit(strtrim(rawLines{i}), ',');
    nf_i = numel(fields);
    if nf_i < 10, continue; end  % riga troppo corta per essere dati
    nCheck = min(nf_i, headerCols);  % tolera file merged con trailing-empty strippati
    % Colonne testuali = non-vuote E non-numeriche (es. Source, SourceFile)
    isText = cellfun(@(f) ~isempty(strtrim(f)) && isempty(regexp(strtrim(f), validNumPat, 'once')), fields(1:nCheck));
    if sum(isText) <= 3   % tollera max 3 col. testuali
        numericColIdx = find(~isText);  % include numerici E vuoti (=NaN)
        break;
    end
end
if isempty(numericColIdx)
    numericColIdx = 1:headerCols;  % fallback: considera tutto numerico
end

% Trova il numero di colonne dalle prime righe valide (gestisce NTC extra)
expectedCols = headerCols;
for i = 2:min(numel(rawLines), 20)
    fields = strsplit(strtrim(rawLines{i}), ',');
    if numel(fields) < max(numericColIdx), continue; end   % riga troppo corta
    numOk = all(cellfun(@(f) isempty(strtrim(f)) || ~isempty(regexp(strtrim(f), validNumPat, 'once')), fields(numericColIdx)));
    if numOk && numel(fields) >= max(numericColIdx)
        expectedCols = numel(fields);
        break;
    end
end

% Aggiungi NTC1,NTC2,... all'header se le righe dati hanno colonne extra
extra = expectedCols - headerCols;
if extra > 0
    ntcNames = arrayfun(@(n) sprintf('NTC%d', n), 1:extra, 'UniformOutput', false);
    rawLines{1} = [rawLines{1}, sprintf(',%s', ntcNames{:})];
end

% Tieni header + righe valide:
%   - numero di campi = expectedCols
%   - tutti i campi numerici (per gli indici in numericColIdx) sono validi
%   - campo vuoto = NaN (accettato)
keep = false(1, numel(rawLines));
keep(1) = true;
nBad  = 0;
nPad  = 0;
for i = 2:numel(rawLines)
    fields = strsplit(strtrim(rawLines{i}), ',');
    nf = numel(fields);

    % Scarta righe troppo corte (sicuramente corrotte/vuote) o enormi
    if nf < 14 || nf > expectedCols + 5
        nBad = nBad + 1; continue;
    end

    % Controlla solo i campi numerici effettivamente presenti
    presentNumIdx = numericColIdx(numericColIdx <= nf);
    numOk = all(cellfun(@(f) isempty(strtrim(f)) || ...
        ~isempty(regexp(strtrim(f), validNumPat, 'once')), fields(presentNumIdx)));

    if numOk
        % Padda con virgole vuote se la riga è più corta dell'atteso
        if nf < expectedCols
            rawLines{i} = [rawLines{i}, repmat(',', 1, expectedCols - nf)];
            nPad = nPad + 1;
        % Tronca se leggermente più lunga (es. NTC extra non previsti)
        elseif nf > expectedCols
            parts = strsplit(rawLines{i}, ',');
            rawLines{i} = strjoin(parts(1:expectedCols), ',');
        end
        keep(i) = true;
    else
        nBad = nBad + 1;
    end
end
if nBad > 0
    fprintf('[WARN] Scartate %d righe corrotte (errori UART / merge di campi)\n', nBad);
end
if nPad > 0
    fprintf('[INFO] Paddato %d righe con campi mancanti (colonne NTC/extra assenti)\n', nPad);
end

% Estrai nomi colonne dall'header (rimuovi BOM UTF-8 e altri ctrl chars iniziali)
headerLine = regexprep(rawLines{1}, '^[^\x21-\x7E]+', '');
varNames   = strtrim(strsplit(strtrim(headerLine), ','));
varNamesOk = matlab.lang.makeValidName(varNames);

% Scrivi su file temporaneo SOLO le righe dati (senza header)
% così readtable non può confondersi su quale riga sia l'header
tmpFile = [tempname '.csv'];
fid = fopen(tmpFile, 'w');
dataIdx = find(keep);
dataIdx(dataIdx == 1) = [];   % escludi riga header (i=1)
for i = dataIdx
    fprintf(fid, '%s\n', rawLines{i});
end
fclose(fid);

data = readtable(tmpFile, 'Delimiter', ',', 'ReadVariableNames', false);
delete(tmpFile);

% Assegna nomi colonne (gestisci eventuale mismatch)
nDataCols = width(data);
nNameCols = numel(varNamesOk);
if nDataCols <= nNameCols
    data.Properties.VariableNames = varNamesOk(1:nDataCols);
else
    extraNames = arrayfun(@(n) sprintf('Extra%d',n), 1:(nDataCols-nNameCols), 'UniformOutput', false);
    data.Properties.VariableNames = [varNamesOk, extraNames];
end

% Rimuovi colonne di metadati non-sensore presenti nei file merged (Source, SourceFile)
for mc_ = {'Source', 'SourceFile'}
    if ismember(mc_{1}, data.Properties.VariableNames)
        data = removevars(data, mc_{1});
    end
end

% --- Rimuovi righe con Time_ms non crescente (riconnessioni / timestamp UART corrotti) ---
tColFilt = find(strcmpi(data.Properties.VariableNames, 'Time_ms'), 1);
if ~isempty(tColFilt)
    t_raw_filt = data.(data.Properties.VariableNames{tColFilt});
    bad_time   = [false; diff(t_raw_filt) < 0];
    if any(bad_time)
        fprintf('[WARN] Rimossi %d campioni con Time_ms non monotono (riconnessioni/timestamp corrotti)\n', ...
                sum(bad_time));
        data = data(~bad_time, :);
    end
end

% --- Asse temporale in secondi ---
% Supporta sia CSV Feather (Time_s) sia CSV ESP32 (Time_ms)
cols = data.Properties.VariableNames;
timeColMs = find(strcmpi(cols, 'Time_ms'), 1);
timeColS  = find(strcmpi(cols, 'Time_s'),  1);
if ~isempty(timeColMs)
    tempo = data.(cols{timeColMs}) / 1000;
elseif ~isempty(timeColS)
    tempo = data.(cols{timeColS});
else
    fprintf('[ERR] Colonne trovate: %s\n', strjoin(cols, ', '));
    error('Colonna temporale non trovata (atteso Time_ms o Time_s).');
end

% --- Estrazione colonne obbligatorie ---
% Colonne CSV: Time_ms/Time_s, TempMotor, TempInverter, TempIGBT,
%              Voltage, Speed, Iq, Id, TorqueMotor, PedalPerc,
%              InvState (*), ErrCode, StatusWord, ErrInfo1,
%              PhaseU_mA, PhaseV_mA, PhaseW_mA, Power_W,
%              TorqueSetpoint (*), TorqueLimitDyn (*) — solo backend ESP32
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

% TorqueSetpoint / TorqueLimitDyn: presenti solo nel backend ESP32
hasTorqueControl = ismember('TorqueSetpoint', cols) && ismember('TorqueLimitDyn', cols);
if hasTorqueControl
    torqueSetpoint  = data.TorqueSetpoint;
    torqueLimitDyn  = data.TorqueLimitDyn;
end

% NTC: presenti solo nel backend Feather
% Controlla ogni canale separatamente (il file merged può avere NTC1 ma non NTC2/3)
hasNTC = ismember('NTC1', cols);
nanCol = nan(height(data), 1);
if hasNTC
    ntc1 = data.NTC1;
    if ismember('NTC2', cols); ntc2 = data.NTC2; else; ntc2 = nanCol; end
    if ismember('NTC3', cols); ntc3 = data.NTC3; else; ntc3 = nanCol; end
    % Sostituisci valori sentinella (-999) con NaN
    ntc1(ntc1 == -999) = NaN;
    ntc2(ntc2 == -999) = NaN;
    ntc3(ntc3 == -999) = NaN;
    % Clipping fisico NTC: range plausibile -40..200°C
    ntc1(ntc1 < -40 | ntc1 > 200) = NaN;
    ntc2(ntc2 < -40 | ntc2 > 200) = NaN;
    ntc3(ntc3 < -40 | ntc3 > 200) = NaN;
end

% ---- Clamping fisico globale (valori fuori range → clampati al limite) ---
tempMotor    = max(-40, min(200,  tempMotor));
tempInverter = max(-40, min(150,  tempInverter));
tempIGBT     = max(-40, min(200,  tempIGBT));
voltage      = max(-50, min(650,  voltage));    % bus DC: -50..650 V
speed(abs(speed) > 30000) = sign(speed(abs(speed) > 30000)) .* 30000;
power_W      = max(-100000, min(100000, power_W));
pedal        = max(0,   min(100,  pedal));      % pedale: 0..100 %

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
subplot(4,1,1);
hold on; grid on;
plot(tempo, speed, 'b', 'LineWidth', 1.5, 'DisplayName', 'Speed');
title('Velocità Motore'); xlabel('Tempo (s)'); ylabel('RPM');
legend('Location', 'best'); hold off;

subplot(4,1,2);
hold on; grid on;
plot(tempo, iq, 'r',  'LineWidth', 1.5, 'DisplayName', 'Iq');
plot(tempo, id, 'b',  'LineWidth', 1.5, 'DisplayName', 'Id');
title('Correnti Iq / Id'); xlabel('Tempo (s)'); ylabel('Corrente (A)');
legend('Location', 'best'); hold off;

subplot(4,1,3);
hold on; grid on;
plot(tempo, torque, 'g', 'LineWidth', 1.5, 'DisplayName', 'Coppia misurata');
if hasTorqueControl
    plot(tempo, torqueLimitDyn, 'r:',  'LineWidth', 1.2, 'DisplayName', 'Limite dinamico');
end
title('Coppia Motore'); xlabel('Tempo (s)'); ylabel('Coppia (Nm)');
legend('Location', 'best'); hold off;

subplot(4,1,4);
hold on; grid on;
if hasTorqueControl
    plot(tempo, torqueSetpoint, 'b--', 'LineWidth', 1.2, 'DisplayName', 'Setpoint richiesto');
end
title('Setpoint Coppia'); xlabel('Tempo (s)'); ylabel('Coppia (Nm)');
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
% --- Status Word: tutti i bit b8-b15 come segnali digitali 0/1 impilati ---
sw_bit_masks = [256, 512, 1024, 2048, 4096, 8192, 16384, 32768];
sw_bit_names = {'SysRdy(b8)','Err(b9)','Warn(b10)','QDCon(b11)',...
                'DCon(b12)','QInvOn(b13)','InvOn(b14)','Der(b15)'};
sw_colors    = {[0 0.45 0.74],[0.85 0.33 0.10],[0.49 0.18 0.56],...
                [0 0.75 0.75],[0.47 0.67 0.19],[0.64 0.08 0.18],...
                [0.93 0.69 0.13],[0.30 0.30 0.30]};
sw_offset    = 1.3;   % separazione verticale tra i bit
hold on; grid on;
sw_u32 = uint32(statusWord);
for k = 1:8
    bit_01 = double(bitand(sw_u32, uint32(sw_bit_masks(k))) > 0) + (k-1)*sw_offset;
    stairs(tempo, bit_01, 'Color', sw_colors{k}, 'LineWidth', 1.2, ...
           'DisplayName', sw_bit_names{k});
end
yticks((0:7)*sw_offset + 0.5);
yticklabels(sw_bit_names);
ylim([-0.2, 8*sw_offset]);
title('Status Word — bit b8-b15 (0/1, impilati)');
xlabel('Tempo (s)'); ylabel('Bit');
legend('Location', 'eastoutside', 'NumColumns', 1); hold off;

subplot(nRows,1,spIdx);
hold on; grid on;
stairs(tempo, statusWord, 'k', 'LineWidth', 1.2, 'DisplayName', 'StatusWord (raw)');
title('Status Word (valore grezzo)'); xlabel('Tempo (s)'); ylabel('Valore');
legend('Location', 'best'); hold off;