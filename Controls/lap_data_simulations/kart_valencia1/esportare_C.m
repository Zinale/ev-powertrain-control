file_input = 'telemetria_esportata.csv';
file_output = 'throttle_data.h';
target_dt_ms = 40;

disp('Caricamento dati in corso...');
dati = readtable(file_input, 'VariableNamingRule', 'preserve');

tempo = dati.Time;
pedale = dati.Throttle_Pos;

%% Analisi e Filtraggio
dt_originale_ms = mean(diff(tempo)) * 1000;
disp(['Il file originale ha un dato ogni ~', num2str(dt_originale_ms, '%.1f'), ' ms']);

fattore_riduzione = round(target_dt_ms / dt_originale_ms);

if fattore_riduzione > 1
    disp(['Applicazione FILTRO e adattamento a ', num2str(target_dt_ms), ' ms...']);
    % La funzione 'decimate' APPLICA UN FILTRO PASSA-BASSO 
    pedale_filtrato = decimate(pedale, fattore_riduzione);
    
    nuovo_dt_s = (dt_originale_ms * fattore_riduzione) / 1000;
    tempo_filtrato = tempo(1) + (0:length(pedale_filtrato)-1)' * nuovo_dt_s;
else
    disp('La frequenza richiesta è uguale (o superiore) all''originale. Nessun filtro applicato.');
    pedale_filtrato = pedale;
    tempo_filtrato = tempo;
end

pedale_filtrato(pedale_filtrato < 0) = 0;
pedale_filtrato(pedale_filtrato > 100) = 100;

pedale_intero = uint8(round(pedale_filtrato));

%% GRAFICO 
figure('Name', 'Confronto Simulazione Acceleratore', 'NumberTitle', 'off');
hold on; grid on;

plot(tempo, pedale, 'b-', 'LineWidth', 1, 'DisplayName', 'Originale (Alta Freq.)');

plot(tempo_filtrato, pedale_intero, 'r.-', 'LineWidth', 1.5, 'MarkerSize', 10, 'DisplayName', sprintf('Simulazione C (%d ms)', target_dt_ms));

% Estetica del grafico
title(sprintf('Pedale Acceleratore: Originale vs Simulazione (dt = %d ms)', target_dt_ms));
xlabel('Tempo [s]');
ylabel('Posizione Pedale [%]');
legend('Location', 'best');
xlim([tempo(1) tempo(end)]);
ylim([-5 105]); 
hold off;

%% C Header

disp(['Valore medio pedale: ', num2str(mean(pedale_intero)), ' %']);


disp('Generazione del file Header (.h) per il C...');
fileID = fopen(file_output, 'w');

fprintf(fileID, '#ifndef THROTTLE_DATA_H\n');
fprintf(fileID, '#define THROTTLE_DATA_H\n\n');
fprintf(fileID, '// File generato automaticamente da MATLAB\n');
fprintf(fileID, '// Tempo di campionamento simulato: %d ms\n', target_dt_ms);
fprintf(fileID, '// Numero totale di campioni: %d\n\n', length(pedale_intero));

fprintf(fileID, '#include <stdint.h>\n\n');
fprintf(fileID, 'const uint8_t throttle_sim[%d] = {\n    ', length(pedale_intero));

% 15 valori per riga così il file è leggibile
for i = 1:length(pedale_intero)
    fprintf(fileID, '%3d, ', pedale_intero(i));
    if mod(i, 15) == 0 && i ~= length(pedale_intero)
        fprintf(fileID, '\n    ');
    end
end

fprintf(fileID, '\n};\n\n');
fprintf(fileID, '#endif // THROTTLE_DATA_H\n');
fclose(fileID);

disp(['Finito! File esportato: ', file_output]);