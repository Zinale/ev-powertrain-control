%% setup_simulazione_telemetria.m
% Carica i dati di telemetria dal CSV e prepara le timeseries per Simulink.
%
% UTILIZZO:
%   1. Esegui questo script in MATLAB (cwd = cartella del file .m)
%   2. Nel modello Simulink:
%        - Sostituisci "Signal Throttle Builder"  con un blocco "From Workspace"
%          che legge la variabile  throttle_ts
%        - Sostituisci "Signal Steering Builder"  con un blocco "From Workspace"
%          che legge la variabile  steering_ts
%        - Sostituisci la costante 0 del freno    con un blocco "From Workspace"
%          che legge la variabile  brake_ts
%        - Imposta SimulationInput StopTime = t_stop (vedere valore stampato)
%   3. Esegui setupParams.m per caricare i parametri del veicolo
%   4. Avvia la simulazione

clear; clc;

%% --- Parametri -----------------------------------------------------------
CSV_FILE  = fullfile(fileparts(mfilename('fullpath')), 'telemetria_esportata.csv');
% NOTA: Ts NON viene definito qui per non sovrascrivere quello di setupParams.m.
%       Il blocco "From Workspace" interpola automaticamente al passo del modello.

%% --- Caricamento CSV -----------------------------------------------------
fprintf('Caricamento "%s"...\n', CSV_FILE);
dati = readtable(CSV_FILE, 'VariableNamingRule', 'preserve');

t_raw        = dati.Time;           % [s]
throttle_raw = dati.Throttle_Pos;   % [%]  0-100
brake_raw    = dati.Brake_Pos;      % [%]  0-100
steering_raw = dati.Steering_Angle; % [°]

fprintf('  Durata registrazione : %.2f s\n', t_raw(end) - t_raw(1));
fprintf('  Campioni originali   : %d  (%.1f ms)\n', numel(t_raw), mean(diff(t_raw))*1000);

%% --- Saturazione valori fisici -------------------------------------------
throttle_raw = max(0, min(100, throttle_raw));
brake_raw    = max(0, min(100, brake_raw));

%% --- Ricampionamento a Ts ------------------------------------------------
% Ts deve essere già nel workspace (messo da setupParams.m).
% NON lo ridefinisco qui per non sovrascriverlo.
t_start = t_raw(1);
t_stop  = t_raw(end);

if ~exist('Ts', 'var')
    warning(['Ts non trovato nel workspace. ' ...
             'Esegui setupParams.m prima di questo script. ' ...
             'Uso la frequenza originale del CSV (~%.1f ms).'], ...
             mean(diff(t_raw))*1000);
    t_sim        = t_raw - t_start;
    throttle_uni = throttle_raw;
    brake_uni    = brake_raw;
    steering_uni = steering_raw;
else
    % Griglia uniforme esattamente a Ts: nessuna ambiguita' con il solvere discreto
    t_uni        = (t_start : Ts : t_stop)';
    t_sim        = t_uni - t_start;
    throttle_uni = interp1(t_raw, throttle_raw, t_uni, 'linear');
    brake_uni    = interp1(t_raw, brake_raw,    t_uni, 'linear');
    steering_uni = interp1(t_raw, steering_raw, t_uni, 'linear');
    fprintf('  Ricampionamento a Ts = %.4f s (%d Hz): %d campioni\n', ...
            Ts, round(1/Ts), numel(t_sim));
end

% Cast a single per compatibilita' con il modello Simulink
throttle_ts = timeseries(single(throttle_uni), t_sim);
throttle_ts.Name = 'throttle_ts';

brake_ts = timeseries(single(brake_uni), t_sim);
brake_ts.Name = 'brake_ts';

steering_ts = timeseries(single(steering_uni), t_sim);
steering_ts.Name = 'steering_ts';

%% --- Riepilogo -----------------------------------------------------------
fprintf('\n--- Variabili create nel workspace ---\n');
fprintf('  throttle_ts  : Throttle_Pos  [%%]    min=%.1f  max=%.1f\n', min(throttle_uni), max(throttle_uni));
fprintf('  brake_ts     : Brake_Pos     [%%]    min=%.1f  max=%.1f\n', min(brake_uni),    max(brake_uni));
fprintf('  steering_ts  : Steering_Angle [deg]  min=%.1f  max=%.1f\n', min(steering_uni), max(steering_uni));
fprintf('\n  t_stop (StopTime simulazione) = %.4f s\n\n', t_sim(end));

%% --- Preview rapida ------------------------------------------------------
figure('Name', 'Telemetria per simulazione', 'NumberTitle', 'off');

subplot(3,1,1);
plot(t_sim, throttle_uni, 'g'); grid on;
ylabel('Throttle [%]'); title('Dati telemetria – pronto per Simulink');

subplot(3,1,2);
plot(t_sim, brake_uni, 'r'); grid on;
ylabel('Brake [%]');

subplot(3,1,3);
plot(t_sim, steering_uni, 'b'); grid on;
ylabel('Steering [°]'); xlabel('Tempo [s]');
