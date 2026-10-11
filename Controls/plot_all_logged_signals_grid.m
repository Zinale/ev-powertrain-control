%% plot_all_logged_signals_grid.m (v2 - diagnostica)
% Come la versione precedente, ma con due differenze pensate per capire
% perché un segnale "sparisce" invece di comparire nella griglia:
%   1) griglia di default 2x3 (6 celle, non più 8)
%   2) ogni elemento è avvolto in un try/catch: se una lettura o un plot
%      fallisce, invece di saltare silenziosamente quell'elemento, la
%      cella corrispondente mostra il messaggio di errore esatto in rosso.
%
% USO:
%   plot_all_logged_signals_grid(out.logsout)          % default 2x3 = 6 celle
%   plot_all_logged_signals_grid(out.logsout, 2, 3)     % esplicito

function plot_all_logged_signals_grid(ds, nRows, nCols)

if nargin < 2, nRows = 2; end
if nargin < 3, nCols = 3; end

n = ds.numElements;
fprintf('Trovati %d elementi nel dataset:\n', n);
for i = 1:n
    try
        el = ds{i};
        sz = size(el.Values.Data);
        if numel(sz) < 2, sz(2) = 1; end
        fprintf('  [%d] "%s"  ->  %d campioni x %d colonna/e\n', i, el.Name, sz(1), sz(2));
    catch ME
        fprintf('  [%d] ERRORE nel leggere questo elemento: %s\n', i, ME.message);
    end
end
if nRows * nCols < n
    fprintf('\nATTENZIONE: la griglia %dx%d ha solo %d celle ma ci sono %d elementi,\n', ...
        nRows, nCols, nRows*nCols, n);
    fprintf('quindi solo i primi %d verranno mostrati. Aumenta nRows/nCols se ti servono tutti.\n\n', nRows*nCols);
end

colori = lines(8);

figure('Color', 'w', 'Name', 'Segnali loggati - vista unificata');
tiledlayout(nRows, nCols, 'TileSpacing', 'compact', 'Padding', 'compact');

nPlot = min(n, nRows * nCols);
for i = 1:nPlot
    nexttile;
    try
        el = ds{i};
        t = el.Values.Time;
        y = el.Values.Data;
        if isvector(y)
            y = y(:);
        end
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
    catch ME
        % Non nascondere l'errore: mostralo direttamente nella cella,
        % così un segnale "che sparisce" diventa visibile e diagnosticabile.
        axis off;
        text(0.5, 0.5, sprintf('[%d] ERRORE:\n%s', i, ME.message), ...
            'Color', 'r', 'FontSize', 8, 'HorizontalAlignment', 'center', ...
            'Interpreter', 'none', 'Units', 'normalized');
    end
end
end