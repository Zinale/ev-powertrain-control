%% plot_signals_custom.m
% Versione "a mano": ora che sai cosa c'è in ciascuno dei 6 pannelli
% (guardando le colonne nella griglia automatica), personalizza qui sotto
% titolo, etichette di legenda e colori per ciascuno. L'ordine dei pannelli
% [1]..[6] è lo stesso che hai già visto nella griglia automatica.
%
% USO:
%   plot_signals_custom(out.logsout)
function plot_signals_custom(ds)
% ===================== CONFIGURAZIONE (modifica qui) =====================
% Per ogni pannello: 'title' (testo pulito per la tesi), 'legend' (una
% stringa per colonna, nello stesso ordine in cui le hai viste nella
% griglia automatica), 'colors' (una tripletta RGB per colonna) e
% 'ylabel'. Aggiungi/togli colonne in 'legend' e 'colors' se un
% pannello non ha il numero di linee che mi aspetto qui sotto.
    config = {
        struct('title',  'Slip Ratio', ...
'legend', {{'\lambda_{RL,target}', '\lambda_{RR,target}', '\lambda_{RL,real}', '\lambda_{RR,real}'}}, ...
'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980], ...
                           [0.9290 0.6940 0.1250], [0.4660 0.6740 0.1880]}}, ...
'ylabel', '\lambda [-]')
        struct('title',  'SRC Cut', ...
'legend', {{'K_{RL}', 'K_{RR}'}}, ...
'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410]}}, ...
'ylabel', 'Torque [Nm]')
        struct('title',  'Driver input', ...
'legend', {{'Throttle Raw', 'Throttle MCU Mapped', 'Brake'}}, ...
'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980], [0.9290 0.6940 0.1250]}}, ...
'ylabel', '[%]')
        struct('title',  'Torque Motors', ...
'legend', {{'T_{RL}', 'T_{RR}'}}, ...
'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
'ylabel', 'Torque [Nm]')
        struct('title',  'Speed', ...
'legend', {{'Vehicle Speed', 'Vx_{real}', 'Vx_{target}'}}, ...
'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
'ylabel', 'V [km/h]')
        struct('title',  'Wheels Speed', ...
'legend', {{'v_{FL}', 'v_{FR}', 'v_{RL}', 'v_{RR}'}}, ...
'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980], ...
                           [0.9290 0.6940 0.1250], [0.4660 0.6740 0.1880]}}, ...
'ylabel', 'v [km/h]')
    };
% ===========================================================================
    figure('Color', 'w', 'Name', 'Segnali - vista personalizzata');
    tiledlayout(2, 3, 'TileSpacing', 'compact', 'Padding', 'compact');
for i = 1:numel(config)
        cfg = config{i};
        el  = ds{i};
        t = el.Values.Time;
        y = el.Values.Data;
if isvector(y)
            y = y(:);
end
        nexttile;
        hold on;
        nColsData = size(y, 2);
for c = 1:nColsData
if c <= numel(cfg.colors)
                col = cfg.colors{c};
else
                col = lines(1); % fallback se mancano colori in config
end
            plot(t, y(:,c), 'Color', col, 'LineWidth', 2.4);
end
        grid on;
        xlabel('Time [s]');
        ylabel(cfg.ylabel);
        title(cfg.title);
if nColsData <= numel(cfg.legend)
            legend(cfg.legend(1:nColsData), 'Location', 'best', 'FontSize', 14);
else
            legend(cfg.legend, 'Location', 'best', 'FontSize', 14); % meno etichette che colonne: MATLAB avvisa, non si blocca
end
end
end
%% ============ Esportazione in alta risoluzione ============
% exportgraphics(gcf, 'segnali_acceleration.png', 'Resolution', 300);