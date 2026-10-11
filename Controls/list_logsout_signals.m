%% list_logsout_signals.m
% Diagnostico: stampa nome, indice e numero di colonne di ciascun elemento
% del Dataset logsout, cosi' confermi la corrispondenza ds{i} <-> pannello
% PRIMA di scrivere la config definitiva in plot_signals_skidpad.m.
% Serve perche' le etichette dello Scope non sono affidabili (viste piu'
% volte ereditate da run precedenti): questo legge i nomi assegnati
% davvero al segnale nel modello, non quello che mostra la legenda.
%
% USO:
%   list_logsout_signals(out.logsout)

function list_logsout_signals(ds)
fprintf('%-4s %-30s %-8s %s\n', 'idx', 'nome segnale', 'colonne', 'lunghezza');
fprintf('%s\n', repmat('-', 1, 60));
for i = 1:numel(ds)
    el = ds{i};
    y = el.Values.Data;
    if isvector(y)
        nCols = 1;
    else
        nCols = size(y, 2);
    end
    nSamples = size(el.Values.Time, 1);
    fprintf('%-4d %-30s %-8d %d\n', i, el.Name, nCols, nSamples);
end
end