% Inizializzazione della figura con sfondo bianco
figure('Color', 'white', 'Position', [100, 100, 700, 450]);

% Vettore della corsa del pedale normalizzata (da 0 a 1)
p = linspace(0, 1, 200);

% Parametri di shaping: T_req = T_max * p^gamma
gamma_eco    = 1.6;   % Risposta concava (più graduale)
gamma_normal = 1.0;   % Risposta lineare
gamma_gas    = 0.6;   % Risposta convessa (aggressiva)

% Calcolo delle curve (in percentuale di T_max)
y_eco    = (p.^gamma_eco)    * 100;
y_normal = (p.^gamma_normal) * 100;
y_gas    = (p.^gamma_gas)    * 100;
p_pct = p * 100;

% Colori coerenti con la palette delle mappe dell'HMI
c_eco    = [68, 223, 49]  / 255;   % #44df31 (Verde)
c_normal = [11, 117, 247] / 255;   % #0b75f7 (Blu)
c_gas    = [250, 10, 10]  / 255;   % #fa0a0a (Rosso)

% Plot delle curve
plot(p_pct, y_eco,    'Color', c_eco,    'LineWidth', 3);
hold on;
plot(p_pct, y_normal, 'Color', c_normal, 'LineWidth', 3);
plot(p_pct, y_gas,    'Color', c_gas,    'LineWidth', 3);

% Riferimento tratteggiato 1:1 (lineare pura)
plot(p_pct, p_pct, '--', 'Color', [0.6, 0.6, 0.6], 'LineWidth', 1.2);
hold off;

% Decorazioni e stile degli assi
grid on;
grid minor;
xlim([0 100]);
ylim([0 100]);
xlabel('APPs Pedal Stroke [%]', 'FontSize', 12, 'FontWeight', 'bold');
ylabel('Torque Request [% of T_{max}]', 'FontSize', 12, 'FontWeight', 'bold', ...
       'Interpreter', 'tex');
title('Powertrain E-MAPs Pedal Shaping', 'FontSize', 14, 'FontWeight', 'bold');

% Legenda con i parametri gamma espliciti
legend({sprintf('ECO (\\gamma = %.1f)',    gamma_eco), ...
        sprintf('NORMAL (\\gamma = %.1f)', gamma_normal), ...
        sprintf('GAS (\\gamma = %.1f)',    gamma_gas), ...
        'Linear 1:1'}, ...
       'Location', 'northwest', 'FontSize', 11, 'Interpreter', 'tex');

set(gca, 'FontSize', 11, 'LineWidth', 1.2, 'Box', 'on', 'TickDir', 'in');

% Export per la tesi
exportgraphics(gcf, 'PedalMap.png', 'Resolution', 300);