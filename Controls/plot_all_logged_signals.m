%% plot_all_logged_signals_grid.m
% Come plot_all_logged_signals.m, ma disegna tutti i segnali in un'UNICA
% figura, organizzata a griglia (default 2 righe x 4 colonne), invece di
% aprire una finestra separata per ciascuno.
%
% USO:
%   plot_all_logged_signals_grid(out.logsout)              % default 2x4
%   plot_all_logged_signals_grid(out.logsout, 2, 4)         % esplicito
%   plot_all_logged_signals_grid(nome_variabile_scope, 3, 3) % altra griglia

function plot_all_logged_signals_grid(ds, nRows, nCols)

    if nargin < 2, nRows = 2; end
    if nargin < 3, nCols = 4; end

    n = ds.numElements;
    if nRows * nCols < n
        warning(['La griglia %dx%d ha solo %d celle ma ci sono %d segnali: ' ...
                 'i segnali in eccesso non verranno mostrati. Aumenta nRows/nCols.'], ...
                 nRows, nCols, nRows*nCols, n);
    end

    fprintf('Trovati %d elementi nel dataset:\n', n);
    for i = 1:n
        el = ds{i};
        sz = size(el.Values.Data);
        if numel(sz) < 2, sz(2) = 1; end
        fprintf('  [%d] "%s"  ->  %d campioni x %d colonna/e\n', i, el.Name, sz(1), sz(2));
    end

    colori = lines(8);

    figure('Color', 'w', 'Name', 'Segnali loggati - vista unificata');
    tiledlayout(nRows, nCols, 'TileSpacing', 'compact', 'Padding', 'compact');

    nPlot = min(n, nRows * nCols);
    for i = 1:nPlot
        el = ds{i};
        t = el.Values.Time;
        y = el.Values.Data;
        if isvector(y)
            y = y(:);
        end

        nexttile;
        hold on;
        nColsData = size(y, 2);
        for c = 1:nColsData
            plot(t, y(:,c), 'Color', colori(mod(c-1,8)+1,:), 'LineWidth', 1.2);
        end
        grid on;
        title(sprintf('[%d] %s', i, el.Name), 'Interpreter', 'none', 'FontSize', 9);
        if nColsData > 1
            legend(arrayfun(@(c) sprintf('col %d', c), 1:nColsData, 'UniformOutput', false), ...
                   'Location', 'best', 'FontSize', 7);
        end
    end
end

%% ============ Esportazione (opzionale) ============
% Ora è tutto in un'unica figura, quindi basta:
%
% exportgraphics(gcf, 'segnali_acceleration.png', 'Resolution', 300);