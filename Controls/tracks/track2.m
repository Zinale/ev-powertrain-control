%% Generatore Tracciato FSAE Endurance (Track 2 - Con Plot & STL)
% Commentati clear/close all per non chiudere la finestra durante l'InitFcn di Simulink
% clear; clc; close all;

% --- Parametri Regolamento FSAE ---
track_width    = 3.0;        % [m]
slalom_spacing = 14.0;       % [m]
ay_max         = 1.2 * 9.81; % [m/s^2]
ax_max_accel   = 0.8 * 9.81; % [m/s^2]
ax_max_brake   = 1.4 * 9.81; % [m/s^2]
v_max_straight = 22.0;       % [m/s]

% 1. Waypoints Track 2 (Con raccordo pulito a (0,0) lungo +X)
WP = [
     -20,   0;     % Raccordo ingresso rettilineo
       0,   0;     % Start / Finish
      50,   0;     % Fine rettilineo principale
      90,  25;     % Curva veloce
      90,  75;     % Allungo
      65, 115;     % Tornante Dx
      40, 125;     % Apice Tornante
      15, 115;     % Uscita Tornante
      15,  65;     % Discesa
     -10,  30;     % Raccordo
     -50,  30;     % Misto interno
     -90,   0;     % Curvone
     -90, -60;     % Zona Slalom
     -40, -75;     % Curva finale
     -20,   0      % Chiusura
]';

% 2. Interpolazione Spline Periodica e ricampionamento ds = 0.2m
spline_path = cscvn([WP, WP(:, 2)]);
t = linspace(spline_path.breaks(1), spline_path.breaks(end-1), 3500);
path = fnval(spline_path, t);

ds_calc = [0, sqrt(diff(path(1,:)).^2 + diff(path(2,:)).^2)];
s_cum = cumsum(ds_calc);
s_uniform = 0:0.2:s_cum(end);

X_ref = interp1(s_cum, path(1,:), s_uniform, 'spline');
Y_ref = interp1(s_cum, path(2,:), s_uniform, 'spline');

straight_idx = find(X_ref >= -10 & X_ref <= 45 & abs(Y_ref) < 2.5);
Y_ref(straight_idx) = 0;

% 3. Slalom con Finestra di Hanning
slalom_idx = find(X_ref < -85 & Y_ref < -10 & Y_ref > -50);
if ~isempty(slalom_idx)
    s_sl = Y_ref(slalom_idx);
    s_norm = abs(s_sl - s_sl(1));
    hann_win = sin(linspace(0, pi, length(slalom_idx))).^2;
    slalom_wave = 1.6 * sin(2 * pi * s_norm / (2 * slalom_spacing));
    X_ref(slalom_idx) = X_ref(slalom_idx) + (hann_win .* slalom_wave);
end

% 4. Profilo Dinamico e Segnali
ds_array = sqrt(diff(X_ref).^2 + diff(Y_ref).^2);
s_ref = [0, cumsum(ds_array)];

dX  = gradient(X_ref, s_ref);
dY  = gradient(Y_ref, s_ref);
ddX = gradient(dX, s_ref);
ddY = gradient(dY, s_ref);

curvature = abs(dX .* ddY - dY .* ddX) ./ max((dX.^2 + dY.^2).^(3/2), 1e-6);
curvature(curvature < 1e-4) = 1e-4;
theta = atan2(dY, dX);
psi_ref = unwrap(theta);

V_lat = min(sqrt(ay_max ./ curvature), v_max_straight);

V_fwd = zeros(size(V_lat));
V_fwd(1) = 0;
for i = 2:length(V_lat)
    ds = s_ref(i) - s_ref(i-1);
    V_fwd(i) = min(V_lat(i), sqrt(V_fwd(i-1)^2 + 2 * ax_max_accel * ds));
end

V_ref = zeros(size(V_lat));
V_ref(end) = V_fwd(end);
for i = length(V_lat)-1:-1:1
    ds = s_ref(i+1) - s_ref(i);
    V_ref(i) = min(V_fwd(i), sqrt(V_ref(i+1)^2 + 2 * ax_max_brake * ds));
end
V_ref = movmean(V_ref, 7);

% 5. Coni Laterali
Nx = -sin(theta);
Ny =  cos(theta);

X_left  = X_ref + (track_width / 2) * Nx;
Y_left  = Y_ref + (track_width / 2) * Ny;
X_right = X_ref - (track_width / 2) * Nx;
Y_right = Y_ref - (track_width / 2) * Ny;

cone_idx = 1:round(3.0 / 0.2):length(X_ref);
cones_left  = [X_left(cone_idx)',  Y_left(cone_idx)'];
cones_right = [X_right(cone_idx)', Y_right(cone_idx)'];

% Struttura Unificata per Simulink
trackData.s     = s_ref';
trackData.X     = X_ref';
trackData.Y     = Y_ref';
trackData.psi   = psi_ref';
trackData.V     = V_ref';
trackData.kappa = curvature';
trackData.Ltot  = s_ref(end);

% 6. Render e Visualizzazione Grafica Mappa
figure('Name', 'FSAE Endurance Track 2', 'Color', 'w', 'Position', [100, 100, 900, 650]);
plot(X_ref, Y_ref, 'k--', 'LineWidth', 1.3); hold on;
plot(cones_left(:,1),  cones_left(:,2),  'b^', 'MarkerFaceColor', 'b', 'MarkerSize', 5);
plot(cones_right(:,1), cones_right(:,2), 'r^', 'MarkerFaceColor', 'r', 'MarkerSize', 5);
plot(WP(1,:), WP(2,:), 'yo', 'MarkerFaceColor', 'g', 'MarkerSize', 6);
quiver(0, 0, 12, 0, 0, 'g', 'LineWidth', 2.5, 'MaxHeadSize', 0.8);
axis equal; grid on;
xlabel('X [m]'); ylabel('Y [m]');
title('FSAE Endurance Circuit - Track 2');
legend('Traiettoria Ideale', 'Coni SX (Blu)', 'Coni DX (Rossi)', 'Waypoints', 'Start (+X)', 'Location', 'best');
drawnow; % Forza l'aggiornamento grafico immediato

% 7. Generazione Mesh STL per Simulink 3D (Unreal Engine)
cone_h = 0.5; cone_r = 0.175; n_sides = 12;
th_c = linspace(0, 2*pi, n_sides+1); th_c(end) = [];
v_base = [cone_r * cos(th_c)', cone_r * sin(th_c)', zeros(n_sides, 1)];
v_apex = [0, 0, cone_h];
single_cone_verts = [v_base; v_apex];

faces = [];
for k = 1:n_sides
    k_next = mod(k, n_sides) + 1;
    faces = [faces; k, k_next, n_sides+1];
end

verts_all_L = []; faces_all_L = [];
for i = 1:size(cones_left, 1)
    offset_idx = size(verts_all_L, 1);
    v_shifted = single_cone_verts + [cones_left(i, 1), -cones_left(i, 2), 0];
    verts_all_L = [verts_all_L; v_shifted];
    faces_all_L = [faces_all_L; faces + offset_idx];
end
stlwrite(triangulation(faces_all_L, verts_all_L), 'cones_left.stl');

verts_all_R = []; faces_all_R = [];
for i = 1:size(cones_right, 1)
    offset_idx = size(verts_all_R, 1);
    v_shifted = single_cone_verts + [cones_right(i, 1), -cones_right(i, 2), 0];
    verts_all_R = [verts_all_R; v_shifted];
    faces_all_R = [faces_all_R; faces + offset_idx];
end
stlwrite(triangulation(faces_all_R, verts_all_R), 'cones_right.stl');

disp('Track 2 caricato: Mappa visualizzata e STL generati!');