% analyze_test_log.m
% Analisi log CSV banco prova EV Powertrain - Polimarcheracing
%
% Formato CSV atteso:
%   Riga 1     : intestazioni (74 colonne)
%   Righe 2..N : campioni numerici (nessuna colonna timestamp)
%
% Gruppi segnali: BattSim | Dyno | Lubrication | Regen | System

clc; clear; close all;

%% ---- Selezione file -------------------------------------------------------
[fname, fpath] = uigetfile( ...
    {'*.csv','File CSV (*.csv)'; '*.*','Tutti i file (*.*)'}, ...
    'Seleziona il file di log CSV', ...
    'c:\Users\zinga\Documents\.UNIVPM\PolimarcheRacingTeam\EV-POWERTRAIN-CONTROL\ev-powertrain-control\Tests\logs\');

if isequal(fname, 0)
    disp('Nessun file selezionato. Uscita.');
    return;
end
filepath = fullfile(fpath, fname);
fprintf('Caricamento: %s\n', filepath);

%% ---- Lettura dati ---------------------------------------------------------
opts = detectImportOptions(filepath, 'NumHeaderLines', 0);
opts.VariableNamesLine = 1;
opts.DataLines         = [2, Inf];
opts.Delimiter         = ',';

data = readtable(filepath, opts);
N    = height(data);
fprintf('Campioni letti: %d  |  Variabili: %d\n', N, width(data));

% Asse temporale (indice campione).
% Se si conosce il periodo di campionamento Ts [s], usare:
%   t = (0:N-1)' * Ts;
t = (0:N-1)';

%% ===========================================================================
%  FIGURA 1 — BattSim (Battery Simulator)
%% ===========================================================================
fig1 = figure('Name', 'BattSim', 'NumberTitle', 'off');

subplot(3,1,1);
plot(t, data.BattSim_VoltageFk,  'b',  'LineWidth', 1.3, 'DisplayName', 'Fbk'); hold on;
plot(t, data.BattSim_VoltageRef, 'b--','LineWidth', 0.9, 'DisplayName', 'Ref');
grid on; ylabel('V  [V]'); title('Tensione BattSim');
legend('Location','best'); hold off;

subplot(3,1,2);
plot(t, data.BattSim_CurrentFbk,  'r',  'LineWidth', 1.3, 'DisplayName', 'Fbk'); hold on;
plot(t, data.BattSim_CurrentRef,  'r--','LineWidth', 0.9, 'DisplayName', 'Ref');
grid on; ylabel('I  [A]'); title('Corrente BattSim');
legend('Location','best'); hold off;

subplot(3,1,3);
plot(t, data.BattSim_PowerFbk,  'g',  'LineWidth', 1.3, 'DisplayName', 'Fbk'); hold on;
plot(t, data.BattSim_PoweRef,   'g--','LineWidth', 0.9, 'DisplayName', 'Ref');
grid on; ylabel('P  [W]'); title('Potenza BattSim');
legend('Location','best'); xlabel('Campione [#]'); hold off;

sgtitle(['BattSim  —  ' fname], 'Interpreter','none');

%% ===========================================================================
%  FIGURA 2 — Dyno (Dinamometro)
%% ===========================================================================
fig2 = figure('Name', 'Dyno', 'NumberTitle', 'off');

subplot(3,2,1);
plot(t, data.Dyno_SpeedFbk, 'b',  'LineWidth', 1.3, 'DisplayName', 'Fbk'); hold on;
plot(t, data.Dyno_SpeedRef, 'b--','LineWidth', 0.9, 'DisplayName', 'Ref');
grid on; ylabel('n  [rpm]'); title('Velocità Dyno');
legend('Location','best'); hold off;

subplot(3,2,2);
plot(t, data.Dyno_TorqueFbk, 'r',  'LineWidth', 1.3, 'DisplayName', 'Fbk'); hold on;
plot(t, data.Dyno_TorqueRef, 'r--','LineWidth', 0.9, 'DisplayName', 'Ref');
grid on; ylabel('\tau  [Nm]'); title('Coppia Dyno');
legend('Location','best'); hold off;

subplot(3,2,3);
plot(t, data.Dyno_CurrentFbk, 'm', 'LineWidth', 1.3);
grid on; ylabel('I  [A]'); title('Corrente Dyno');

subplot(3,2,4);
plot(t, data.Dyno_Winding1Temp,    'r', 'LineWidth', 1.2, 'DisplayName', 'Winding1');   hold on;
plot(t, data.Dyno_Winding2Temp,    'b', 'LineWidth', 1.2, 'DisplayName', 'Winding2');
plot(t, data.Dyno_BearingFrontTemp,'g', 'LineWidth', 1.2, 'DisplayName', 'Bearing Ant.');
plot(t, data.Dyno_BearingRearTemp, 'k', 'LineWidth', 1.2, 'DisplayName', 'Bearing Post.');
grid on; ylabel('T  [°C]'); title('Temperature Dyno');
legend('Location','best'); hold off;

subplot(3,2,[5,6]);
plot(t, data.Dyno_Faults,   'r', 'LineWidth', 1.5, 'DisplayName', 'Faults');   hold on;
plot(t, data.Dyno_Warnings, 'y', 'LineWidth', 1.5, 'DisplayName', 'Warnings');
grid on; yticks([0 1]); ylabel('Flag'); title('Fault / Warning Dyno');
legend('Location','best'); xlabel('Campione [#]'); hold off;

sgtitle(['Dyno  —  ' fname], 'Interpreter','none');

%% ===========================================================================
%  FIGURA 3 — System
%% ===========================================================================
fig3 = figure('Name', 'System', 'NumberTitle', 'off');

subplot(3,2,1);
plot(t, data.System_DCBusVoltage, 'b', 'LineWidth', 1.3);
grid on; ylabel('V_{DC}  [V]'); title('DC Bus Voltage');

subplot(3,2,2);
plot(t, data.System_Torquemeter,    'r',  'LineWidth', 1.3, 'DisplayName', 'Istantaneo'); hold on;
plot(t, data.System_TorquemeterAVG, 'r--','LineWidth', 1.0, 'DisplayName', 'Media');
grid on; ylabel('\tau  [Nm]'); title('Torquemeter');
legend('Location','best'); hold off;

subplot(3,2,3);
plot(t, data.System_InputPressure,  'b', 'LineWidth', 1.3, 'DisplayName', 'Ingresso'); hold on;
plot(t, data.System_OutputPressure, 'r', 'LineWidth', 1.3, 'DisplayName', 'Uscita');
grid on; ylabel('p  [bar]'); title('Pressione Lubrificazione');
legend('Location','best'); hold off;

subplot(3,2,4);
plot(t, data.System_Flowmeter, 'g', 'LineWidth', 1.3);
grid on; ylabel('Q  [l/min]'); title('Portata (Flowmeter)');

subplot(3,2,5);
colsXD_high = {'System_TempXD25','System_TempXD26','System_TempXD27','System_TempXD28', ...
               'System_TempXD29','System_TempXD30','System_TempXD31','System_TempXD32'};
hold on;
for k = 1:numel(colsXD_high)
    plot(t, data.(colsXD_high{k}), 'LineWidth', 1.0, ...
         'DisplayName', strrep(colsXD_high{k}, 'System_', ''));
end
grid on; ylabel('T  [°C]'); title('Temperature XD25-32');
legend('Location','best','NumColumns',2); hold off;

subplot(3,2,6);
colsXD_low = {'System_TempXD11','System_TempXD12','System_TempXD13','System_TempXD14'};
hold on;
for k = 1:numel(colsXD_low)
    plot(t, data.(colsXD_low{k}), 'LineWidth', 1.2, ...
         'DisplayName', strrep(colsXD_low{k}, 'System_', ''));
end
grid on; ylabel('T  [°C]'); title('Temperature XD11-14');
legend('Location','best'); hold off;

sgtitle(['System  —  ' fname], 'Interpreter','none');
xlabel('Campione [#]');

%% ===========================================================================
%  FIGURA 4 — Allarmi, Regen, Lubrificazione
%% ===========================================================================
fig4 = figure('Name', 'Allarmi e stati', 'NumberTitle', 'off');

subplot(3,1,1);
plot(t, data.System_AlarmLevel0, 'r', 'LineWidth', 1.5, 'DisplayName', 'AlarmLevel0'); hold on;
plot(t, data.System_AlarmLevel1, 'm', 'LineWidth', 1.5, 'DisplayName', 'AlarmLevel1');
plot(t, data.System_Emergency,   'k', 'LineWidth', 1.5, 'DisplayName', 'Emergency');
grid on; ylim([-0.1 1.1]); yticks([0 1]); ylabel('Flag');
title('System Alarms'); legend('Location','best'); hold off;

subplot(3,1,2);
plot(t, data.Regen_Enabled_MB1,               'b', 'LineWidth', 1.5, 'DisplayName', 'Enabled\_MB1');      hold on;
plot(t, data.Regen_PrechargeDCOk_MB3,         'g', 'LineWidth', 1.5, 'DisplayName', 'PrechargeDCOk\_MB3');
plot(t, data.Regen_Enable_MB8,                'r', 'LineWidth', 1.5, 'DisplayName', 'Enable\_MB8');
plot(t, data.Regen_EnablePrechargeContactor_QA1,'c','LineWidth', 1.2, 'DisplayName', 'PrechargeContactor');
plot(t, data.Regen_EnablePrechargeDC_QA5,     'm', 'LineWidth', 1.2, 'DisplayName', 'PrechargeDC\_QA5');
grid on; ylim([-0.1 1.1]); yticks([0 1]); ylabel('Flag');
title('Stato Regen'); legend('Location','best','NumColumns',2); hold off;

subplot(3,1,3);
plot(t, data.Lubrication_Alarm,    'r', 'LineWidth', 1.5, 'DisplayName', 'Alarm');    hold on;
plot(t, data.Lubrication_PreAlarm, 'y', 'LineWidth', 1.5, 'DisplayName', 'Pre-Alarm');
plot(t, data.Lubrication_Start,    'g', 'LineWidth', 1.5, 'DisplayName', 'Start');
grid on; ylim([-0.1 1.1]); yticks([0 1]); ylabel('Flag');
title('Lubrificazione'); legend('Location','best'); xlabel('Campione [#]'); hold off;

sgtitle(['Allarmi e stati  —  ' fname], 'Interpreter','none');

%% ---- Riepilogo statistiche chiave -----------------------------------------
fprintf('\n===== STATISTICHE =====\n');
fprintf('  Tensione DC Bus  : min=%.2f V  |  max=%.2f V  |  avg=%.2f V\n', ...
    min(data.System_DCBusVoltage), max(data.System_DCBusVoltage), mean(data.System_DCBusVoltage));
fprintf('  BattSim tensione : min=%.2f V  |  max=%.2f V  |  avg=%.2f V\n', ...
    min(data.BattSim_VoltageFk), max(data.BattSim_VoltageFk), mean(data.BattSim_VoltageFk));
fprintf('  BattSim corrente : min=%.2f A  |  max=%.2f A  |  avg=%.2f A\n', ...
    min(data.BattSim_CurrentFbk), max(data.BattSim_CurrentFbk), mean(data.BattSim_CurrentFbk));
fprintf('  Dyno velocità    : min=%.1f rpm|  max=%.1f rpm|  avg=%.1f rpm\n', ...
    min(data.Dyno_SpeedFbk), max(data.Dyno_SpeedFbk), mean(data.Dyno_SpeedFbk));
fprintf('  Dyno coppia      : min=%.2f Nm |  max=%.2f Nm |  avg=%.2f Nm\n', ...
    min(data.Dyno_TorqueFbk), max(data.Dyno_TorqueFbk), mean(data.Dyno_TorqueFbk));
fprintf('  Torquemeter AVG  : min=%.2f Nm |  max=%.2f Nm |  avg=%.2f Nm\n', ...
    min(data.System_TorquemeterAVG), max(data.System_TorquemeterAVG), mean(data.System_TorquemeterAVG));
fprintf('  Portata          : min=%.3f    |  max=%.3f    |  avg=%.3f l/min\n', ...
    min(data.System_Flowmeter), max(data.System_Flowmeter), mean(data.System_Flowmeter));
fprintf('========================\n');
fprintf('Analisi completata.\n');
