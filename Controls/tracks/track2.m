% Generatore Tracciato Virtuale FSAE Endurance 
%clear; clc; close all;

% --- Parametri Regolamento FSAE ---
track_width = 4.0;         % [m] Larghezza minima carreggiata
slalom_spacing = 10.0;     % [m] Spaziatura coni slalom (tra 9m e 15m)
ay_max = 1.1 * 9.81;       % [m/s^2] Accelerazione laterale massima stimata
v_max_straight = 28;       % [m/s] Velocità massima limitata per rettilinei lunghi (~100 km/h)

% 1. Definizione Waypoints (Con rettilineo di partenza rigorosamente dritto lungo +X)
WP = [
     -20,   0;     % Pre-start (Garantisce allineamento Y=0)
       0,   0;     % Start / Finish (X=0, Y=0, Yaw = 0 deg)
      50,   1;     % Fine rettilineo principale (Perfettamente dritto Y=0)
      90,  25;     % Curva veloce
      90,  75;     % Breve rettilineo
      65, 115;     % Ingresso Tornante
      40, 125;     % Apice Tornante
      15, 115;     % Uscita Tornante
      15,  65;     % Discesa
     -10,  30;     % Raccordo
     -50,  30;     % Allungo interno
     -90,   0;     % Curvone di ritorno
     -90, -60;     % Rettilineo di ritorno (zona Slalom)
     -40, -80;     % Curva finale
     -20,   0      % Ritorno al pre-start
]';

% Interpolazione spline fluida
spline_path = cscvn(WP);
t = linspace(spline_path.breaks(1), spline_path.breaks(end), 1500);
path = fnval(spline_path, t);

X_ref = path(1,:);
Y_ref = path(2,:);

% FORZATURA STRUTTURALE: I primi metri attorno all'origine (tra X = -20 e X = 40)
% vengono forzati a Y=0 per annullare qualsiasi minima curvatura introdotta dalla spline
straight_start_idx = find(X_ref >= -20 & X_ref <= 45 & Y_ref < 5 & Y_ref > -5);
Y_ref(straight_start_idx) = 0;

% 2. Generazione Sezione Slalom
slalom_idx = find(X_ref < -85 & Y_ref < -10 & Y_ref > -50);
s_slalom = Y_ref(slalom_idx);
X_ref(slalom_idx) = X_ref(slalom_idx) + 1.8 * sin(2*pi*(s_slalom) / (2*slalom_spacing));

% 3. Calcolo Curvatura (kappa) e Profilo di Velocità (V_ref)
dX = gradient(X_ref);
dY = gradient(Y_ref);
ddX = gradient(dX);
ddY = gradient(dY);

% Formula della curvatura
curvature = abs(dX.*ddY - dY.*ddX) ./ (dX.^2 + dY.^2).^(3/2);
curvature(isnan(curvature)) = 0;
curvature(curvature < 1e-4) = 1e-4; 

% Calcolo velocità limite in curva
V_ref = sqrt(ay_max ./ curvature);
V_ref = min(V_ref, v_max_straight); 
V_ref = movmean(V_ref, 30);         

% 4. Render del Tracciato
theta = atan2(dY, dX);
Nx = -sin(theta);
Ny = cos(theta);

% Posizione dei margini della pista (Coni)
X_left = X_ref + (track_width/2) * Nx;
Y_left = Y_ref + (track_width/2) * Ny;
X_right = X_ref - (track_width/2) * Nx;
Y_right = Y_ref - (track_width/2) * Ny;

% Plot grafico
figure('Name', 'FSAE Endurance Track Render', 'Color', 'w', 'Position', [100, 100, 800, 600]);
plot(X_ref, Y_ref, 'k--', 'LineWidth', 1.5); hold on;
plot(X_left, Y_left, 'b.', 'MarkerSize', 8);        % Coni blu (sinistra)
plot(X_right, Y_right, 'r.', 'MarkerSize', 8);      % Coni rossi/arancio (destra)
plot(X_ref(slalom_idx), Y_ref(slalom_idx), 'm-', 'LineWidth', 2); % Slalom
plot(WP(1,:), WP(2,:), 'yo', 'MarkerSize', 5, 'MarkerFaceColor','g'); % Waypoints

% Individua l'indice di partenza esatto (X=0) e traccia la freccia di Start
start_idx = find(abs(X_ref) == min(abs(X_ref(X_ref >= 0 & Y_ref == 0))), 1);
quiver(X_ref(start_idx), Y_ref(start_idx), dX(start_idx), dY(start_idx), 15, 'g', 'LineWidth', 2, 'MaxHeadSize', 2);

axis equal; grid on;
title('FSAE Virtual Endurance Track');
xlabel('X [m]'); ylabel('Y [m]');
legend('Traiettoria Ideale (X\_ref, Y\_ref)', 'Coni Blu (SX)', 'Coni Rossi (DX)', ...
       'Sezione Slalom', 'Waypoints', 'Vettore Partenza (+X)', 'Location', 'best');