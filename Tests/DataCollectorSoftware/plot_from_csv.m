filename = 'dati_sensori_20260303_133411.csv';

% readtable riconosce automaticamente le intestazioni della prima riga
data = readtable(filename);

% 2. Gestione dell'asse temporale (Timestamp)
% if iscell(data.Timestamp) || isstring(data.Timestamp)
%     tempo = duration(data.Timestamp); 
% else
%     tempo = data.Timestamp;
% end

%            writer.writerow(['TempMotor','TempInverter','TempIGBT','Voltage','Speed','Id','Iq','TorqueMotor','PedalPerc','NTC1', 'NTC2', 'NTC3'])


ntc1 = data.NTC1;
ntc2 = data.NTC2;
ntc3 = data.NTC3;

figure;           
hold on;          % Mantieni il grafico attivo per sovrapporre le linee
grid on;          

% Traccia le tre curve con colori diversi e spessore di linea 1.5

plot(tempo, ntc1, 'r', 'LineWidth', 1.5, 'DisplayName', 'NTC 1');
plot(tempo, ntc2, 'g', 'LineWidth', 1.5, 'DisplayName', 'NTC 2');
plot(tempo, ntc3, 'b', 'LineWidth', 1.5, 'DisplayName', 'NTC 3');

title('Andamento delle Temperature (NTC)');
xlabel('Tempo (ore:min:sec)');
ylabel('Valore Registrato');
legend('Location', 'best'); % Mostra la legenda nella posizione ottimale

hold off;