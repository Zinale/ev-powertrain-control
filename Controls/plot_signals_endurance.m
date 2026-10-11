%% plot_signals_endurance.m
% Due figure distinte per il giro di Endurance (Sezione 4.4.x):
%   Figura A: Dinamica e Controllo (narrativa principale del capitolo)
%   Figura B: Dettaglio Powertrain (contesto, quadro completo)
%
% ATTENZIONE: gli indici [i] qui sotto sono PLACEHOLDER basati sulla sola
% disposizione visiva dello Scope (Display grid, non l'ordine reale nel
% Dataset). Esegui prima:
%   list_logsout_signals(out.NomeVariabile)
% e correggi l'ordine dei blocchi in ENTRAMBE le config sotto perché
% corrispondano ai nomi/colonne reali, PRIMA di fidarti dell'output.
%
% USO:
%   plot_signals_endurance(out.NomeVariabile)              % tutto il giro
%   plot_signals_endurance(out.NomeVariabile, 30)           % taglia a 30s
function plot_signals_endurance(ds, t_max)
    if nargin < 2
        t_max = Inf;
    end

    % ===================== FIGURA A: Dinamica e Controllo =====================
    configA = {
        struct('title',  'Torque Motors', ...
                'legend', {{'T_{RL}', 'T_{RR}'}}, ...
                'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
               'ylabel', 'Torque [Nm]')

       struct('title',  'Yaw Rate', ...
           'legend', {{'\psi\prime_{th}', '\psi\prime_{real}'}}, ...
           'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410]}}, ...
           'ylabel', 'Yaw rate [rad/s]')

        struct('title',  'Steering Angle', ...
               'legend', {{'\delta_{volante}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250]}}, ...
               'ylabel', 'Angle [°]')

       struct('title',  'Driver Input', ...
           'legend', {{'Throttle Raw', 'Throttle MCU Mapped', 'Brake'}}, ...
           'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
           'ylabel', '[%]')

        struct('title',  'Yaw Rate Error', ...
               'legend', {{'e_\psi\prime'}}, ...
               'colors', {{[0.4660 0.6740 0.1880]}}, ...
               'ylabel', 'Error [rad/s]')

       struct('title',  'Speed', ...
           'legend', {{'Vehicle Speed', 'Vx_{real}', 'Vx_{ref}'}}, ...
           'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
           'ylabel', 'V [km/h]')
        % struct('title',  'Slip Ratio (\lambda)', ...
        %        'legend', {{'\lambda_{RL,target}', '\lambda_{RR,target}', '\lambda_{RL,real}', '\lambda_{RR,real}'}}, ...
        %        'colors', {{[0 0.4470 0.7410], [0.8500 0.3250 0.0980], [0.9290 0.6940 0.1250], [0.4940 0.1840 0.5560]}}, ...
        %        'ylabel', '\lambda [-]')
    };
    idxA = [1, 2, 3, 4, 5, 7];   % <-- DA VERIFICARE con list_logsout_signals

    % ================ FIGURA B: Dettaglio Powertrain ==========================
    configB = {
        struct('title',  'Currents Estimated', ...
               'legend', {{'I_{dis, limit}', 'I_{RL}', 'I_{RR}','I_{chg, limit}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980], [0.4660 0.6740 0.1880]}}, ...
               'ylabel', '[A]')

        struct('title',  'Regen & Derating Flags', ...
               'legend', {{'Regen_{active}', 'Der_{RL}', 'Der_{RR}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980]}}, ...
               'ylabel', 'State [-]')

        struct('title',  '\lambda_{slip}', ...
               'legend', {{'\lambda_{RL, ref}','\lambda_{RR, ref}','\lambda_{RL, real}','\lambda_{RR, real}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980], [0.4660 0.6740 0.1880]}}, ...
               'ylabel', '\lambda [-]')

        struct('title',  'Vertical Load (F_z)', ...
               'legend', {{'F_{z,FL}', 'F_{z,FR}', 'F_{z,RL}', 'F_{z,RR}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410], [0.8500 0.3250 0.0980], [0.4660 0.6740 0.1880]}}, ...
               'ylabel', 'F_z [N]')

        struct('title',  'RPM Motors', ...
               'legend', {{'rpm_{RL}', 'rpm_{RR}'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410]}}, ...
               'ylabel', 'State [-]')

        struct('title',  'Acceleration', ...
               'legend', {{'a_x','a_y'}}, ...
               'colors', {{[0.9290 0.6940 0.1250], [0 0.4470 0.7410]}}, ...
               'ylabel', 'a [g]')
    };
    idxB = [13, 8, 9, 10, 11, 12];   % <-- DA VERIFICARE con list_logsout_signals

    plot_grid(ds, configA, idxA, 2, 3, 'Endurance - Dinamica e Controllo', t_max);
    plot_grid(ds, configB, idxB, 2, 3, 'Endurance - Dettaglio Powertrain', t_max);
end

function plot_grid(ds, config, idx, nrows, ncols, figname, t_max)
    figure('Color', 'w', 'Name', figname);
    tiledlayout(nrows, ncols, 'TileSpacing', 'compact', 'Padding', 'compact');

    for k = 1:numel(config)
        cfg = config{k};
        el  = ds{idx(k)};
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
                col = lines(1);
            end
            plot(t, y(:,c), 'Color', col, 'LineWidth', 2.2);
        end
        grid on;
        xlabel('Time [s]');
        ylabel(cfg.ylabel);
        title(cfg.title);
        if isfinite(t_max)
            xlim([0 t_max]);   % niente spazio vuoto oltre il taglio
        end
        if nColsData <= numel(cfg.legend)
            legend(cfg.legend(1:nColsData), 'Location', 'best', 'FontSize', 11);
        else
            legend(cfg.legend, 'Location', 'best', 'FontSize', 11);
        end
    end
end
%% ============ Esportazione in alta risoluzione ============
% [Le due figure vengono aperte come Figure separate: usa gcf su ciascuna]
% exportgraphics(figure(1), 'endurance_dinamica_controllo.png', 'Resolution', 300);
% exportgraphics(figure(2), 'endurance_powertrain.png', 'Resolution', 300);