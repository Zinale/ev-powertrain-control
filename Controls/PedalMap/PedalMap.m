% Inizializzazione della figura con sfondo bianco
figure('Color', 'white', 'Position', [100, 100, 700, 450]);

% Vettore della corsa del pedale (da 0 a 100%)
x = linspace(0, 1, 200);

% Parametri di shaping (esponenti per le curve)
a_eco = 1.6;     % Risposta concava (più graduale)
a_normal = 1.0;  % Risposta lineare
a_gas = 0.6;     % Risposta convessa (aggressiva)

% Calcolo delle curve (percentuale)
y_eco = (x.^a_eco) * 100;
y_normal = (x.^a_normal) * 100;
y_gas = (x.^a_gas) * 100;
x_pct = x * 100;

% Conversione dei colori HEX in vettori RGB normalizzati [0, 1]
c_eco = [68, 223, 49] / 255;       % #44df31 (Verde)
c_normal = [11, 117, 247] / 255;   % #0b75f7 (Blu)
c_gas = [250, 10, 10] / 255;       % #fa0a0a (Rosso)

% Plot delle curve con linee spesse e marcate
plot(x_pct, y_eco, 'Color', c_eco, 'LineWidth', 3);
hold on;
plot(x_pct, y_normal, 'Color', c_normal, 'LineWidth', 3);
plot(x_pct, y_gas, 'Color', c_gas, 'LineWidth', 3);

% Linea di riferimento tratteggiata 1:1 (lineare pura)
plot(x_pct, x_pct, '--', 'Color', [0.6, 0.6, 0.6], 'LineWidth', 1.2);

hold off;

% Decorazioni e stile degli assi
grid on;
grid minor;

xlim([0 100]);
ylim([0 100]);

xlabel('APPs Pedal Stroke [%]', 'FontSize', 12, 'FontWeight', 'bold');
ylabel('Torque Request [%]', 'FontSize', 12, 'FontWeight', 'bold');
title('Powertrain E-MAPs Pedal Shaping', 'FontSize', 14, 'FontWeight', 'bold');

% Legenda con i nomi e i rispettivi parametri 'a' espliciti
legend({sprintf('ECO (a = %.1f)', a_eco), ...
        sprintf('NORMAL (a = %.1f)', a_normal), ...
        sprintf('GAS (a = %.1f)', a_gas), ...
        'Linear 1:1'}, ...
        'Location', 'northwest', 'FontSize', 11);

set(gca, 'FontSize', 11, 'LineWidth', 1.2, 'Box', 'on', 'TickDir', 'in');