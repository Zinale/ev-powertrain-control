%% plot_signals_skidpad.m
% Config per il test di Torque Vectoring su skidpad (Sezione 4.4.2).
% ORDINE CONFERMATO (verificato confrontando i dati plottati con i valori
% attesi, non solo con le etichette): ds{i} nel Dataset ScopeData4
% (accessibile via out.ScopeData4) segue QUESTO ordine, diverso da
% quello dei pannelli nello Scope live di Simulink:
%   [1] Steering Angle
%   [2] Driver Input   (Throttle Raw, Throttle MCU Mapped, Brake)
%   [3] Torque Motors  (T_RL, T_RR)
%   [4] Speed          (Vehicle Speed, Vx_real)
%   [5] Yaw Rate       (yaw_th, yaw_real)
%   [6] Yaw Rate Error (e_yaw_rate)
%
% Se in futuro cambi il modello (aggiungi/rimuovi segnali nel Mux che
% alimenta lo Scope), questo ordine puo' cambiare di nuovo: ricontrolla
% con list_logsout_signals(out.ScopeData4) prima di fidarti ciecamente.
%
% USO:
%   plot_signals_skidpad(out.ScopeData4)
function plot_signals_skidpad(ds, t_max)
% t_max (opzionale): se specificato, taglia la visualizzazione a questo
% istante. Utile per escludere la coda finale della simulazione dove il
% veicolo esce dal tracciato definito e il Pure Pursuit va in wraparound
% (vedi bug discusso su pure_pursuit.m). NON modifica i dati sottostanti,
% solo cosa viene disegnato: il bug nella simulazione resta, questo serve
% solo a non mostrarne l'effetto in figura.
if nargin < 2
    t_max = Inf;
end
% ===================== CONFIGURAZIONE (modifica qui) =====================
    config = {
        % [1] Steering Angle -- 1 colonna
        struct('title',  'Steering Angle', ...
               'legend', {{'\delta_{volante}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250]}}, ...
               'ylabel', 'Angolo [°]')

        % [2] Driver Input -- 3 colonne (Throttle_raw, Throttle_mcu, Brake)
        struct('title',  'Driver Input', ...
               'legend', {{'Throttle Raw', 'Throttle MCU Mapped', 'Brake'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
               'ylabel', '[%]')

        % [3] Torque Motors -- 2 colonne (T_RL, T_RR)
        struct('title',  'Torque Motors', ...
               'legend', {{'T_{RL}', 'T_{RR}'}}, ...
               'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
               'ylabel', 'Torque [Nm]')

        % [4] Speed -- colonne (Vehicle Speed, Vx_real, [eventuale Vx_target])
        struct('title',  'Speed', ...
               'legend', {{'Vehicle Speed', 'Vx_{real}', 'Vx_{target}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
               'ylabel', 'V [km/h]')

        % [5] Yaw Rate -- 2 colonne (yaw_rate_ref, yaw_rate_real)
        struct('title',  'Yaw Rate', ...
               'legend', {{'\psi\prime_{th}', '\psi\prime_{real}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410]}}, ...
               'ylabel', 'Yaw rate [rad/s]')

        % [6] Yaw Rate Error -- 1 colonna
        struct('title',  'Yaw Rate Error', ...
               'legend', {{'e_\psi\prime'}}, ...
               'colors', {{[0.4660 0.6740 0.1880]}}, ...
               'ylabel', 'Errore [rad/s]')
    };
% ===========================================================================
    figure('Color', 'w', 'Name', 'Segnali Skidpad - vista personalizzata');
    tiledlayout(2, 3, 'TileSpacing', 'compact', 'Padding', 'compact');

    for i = 1:numel(config)
        cfg = config{i};
        el  = ds{i};
        t = el.Values.Time;
        y = el.Values.Data;
        if isvector(y)
            y = y(:);
        end
        keep = t <= t_max;
        t = t(keep);
        y = y(keep, :);

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
        if isfinite(t_max)
            xlim([0 t_max]);   % niente spazio vuoto oltre il taglio
        end
        if nColsData <= numel(cfg.legend)
            legend(cfg.legend(1:nColsData), 'Location', 'best', 'FontSize', 14);
        else
            legend(cfg.legend, 'Location', 'best', 'FontSize', 14); % meno etichette che colonne: MATLAB avvisa, non si blocca
        end
    end
end
%% ============ Esportazione in alta risoluzione ============
% exportgraphics(gcf, 'segnali_skidpad_tvc.png', 'Resolution', 300);