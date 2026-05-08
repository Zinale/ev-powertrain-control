dati_mat = load('test_trial_lap.mat');

nomi_variabili = fieldnames(dati_mat);

tabella_export = table(dati_mat.(nomi_variabili{1}).Time', 'VariableNames', {'Time'});

for i = 1:length(nomi_variabili)
    nome_var = nomi_variabili{i};
    % Aggiungi il valore della variabile come nuova colonna
    tabella_export.(nome_var) = dati_mat.(nome_var).Value';
end

writetable(tabella_export, 'telemetria_esportata.csv');
disp('Esportazione CSV completata con successo!');