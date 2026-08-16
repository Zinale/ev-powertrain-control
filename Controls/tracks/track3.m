%% Generatore Tracciato FSAE Endurance (Start in 0,0 lungo +X)
%clear; clc; close all;

% --- Parametri Regolamento FSAE ---
track_width     = 3.0;        % [m] Larghezza pista
slalom_spacing  = 15.0;       % [m] Spaziatura slalom
slalom_amplitude = 1.1; % [m] Ampiezza ridotta da 1.5m a 1.1m (traiettoria morbida)
ay_max          = 0.8 * 9.81; % [m/s^2]
ax_max_accel    = 0.8 * 9.81; % [m/s^2]
ax_max_brake    = 0.9 * 9.81; % [m/s^2]
v_max_straight  = 22.0;       % [m/s] (~80 km/h)

% 1. Definizione Punti (Start fisso in 0,0)
WP_loop = [
       0,   0;   % Start / Origine
      50,   0;   % Fine rettilineo principale
      85,  25;   % Curva 1 veloce
     100,  60;   % Raccordo
      75,  95;   % Chicane Dx
      55,  80;   % Chicane Sx
      35, 110;   % Uscita Chicane
      10, 130;   % Tornante Ingresso
     -15, 130;   % Apice Tornante
     -40, 105;   % Tornante Uscita
     -50,  65;   % Misto
     -85,  40;   % Curva decrescente
     -95,   0;   % Inizio Slalom
     -95, -50;   % Fine Slalom
     -70, -75;   % Curva ritorno
     -45, -50;   % Raccordo
     -25, -15;   % Preparazione ingresso rettilineo
     -15,   0;   % Raccordo orizzontale
       0,   0    % Chiusura esatta
]';

% 2. Interpolazione Spline Periodica con Raccordo Orizzontale
spline_path = cscvn([WP_loop, WP_loop(:, 2)]);
t_eval = linspace(spline_path.breaks(1), spline_path.breaks(end-1), 3500);
path = fnval(spline_path, t_eval);

% Rampionamento a passo spaziale equispaziato ds = 0.2m
ds_calc = [0, sqrt(diff(path(1,:)).^2 + diff(path(2,:)).^2)];
s_cum = cumsum(ds_calc);
s_uniform = 0:0.2:s_cum(end);

X_ref = interp1(s_cum, path(1,:), s_uniform, 'spline');
Y_ref = interp1(s_cum, path(2,:), s_uniform, 'spline');

straight_idx = find(X_ref >= -5 & X_ref <= 45 & abs(Y_ref) < 3);
Y_ref(straight_idx) = 0;

% 3. Sezione Slalom 
slalom_idx = find(X_ref < -90 & Y_ref < -5 & Y_ref > -45);
if ~isempty(slalom_idx)
    s_sl = Y_ref(slalom_idx);
    s_norm = abs(s_sl - s_sl(1));
    hann_win = sin(linspace(0, pi, length(slalom_idx))).^2; % Raccordo C^inf
    slalom_wave = slalom_amplitude * sin(2 * pi * s_norm / (2 * slalom_spacing));
    X_ref(slalom_idx) = X_ref(slalom_idx) + (hann_win .* slalom_wave);
end


% 4. Calcolo Dinamico: Heading, Curvatura e Profilo Velocità
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

slalom_slowdown_idx = find(X_ref < -85 & Y_ref < 10 & Y_ref > -55);
if ~isempty(slalom_slowdown_idx)
    % Impone max ~34 km/h (9.5 m/s) già nei 15 metri di rettilineo prima dello slalom
    V_lat(slalom_slowdown_idx) = min(V_lat(slalom_slowdown_idx), 9.5); 
end

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

% 6. Struttura Unificata per Simulink
trackData.s     = s_ref';
trackData.X     = X_ref';
trackData.Y     = Y_ref';
trackData.psi   = psi_ref';
trackData.V     = V_ref';
trackData.kappa = curvature';
trackData.Ltot  = s_ref(end);

% Coordinate Coni 3D per Unreal Engine
cones_left_pos  = [cones_left,  zeros(size(cones_left, 1), 1)];
cones_right_pos = [cones_right, zeros(size(cones_right, 1), 1)];

% 7. Plot
figure('Name', 'FSAE Endurance Track', 'Color', 'w', 'Position', [100, 100, 900, 650]);
plot(X_ref, Y_ref, 'k--', 'LineWidth', 1.3); hold on;
plot(cones_left(:,1),  cones_left(:,2),  'b^', 'MarkerFaceColor', 'b', 'MarkerSize', 5);
plot(cones_right(:,1), cones_right(:,2), 'r^', 'MarkerFaceColor', 'r', 'MarkerSize', 5);
plot(WP_loop(1,:), WP_loop(2,:), 'yo', 'MarkerFaceColor', 'g', 'MarkerSize', 6);
quiver(0, 0, 12, 0, 0, 'g', 'LineWidth', 2.5, 'MaxHeadSize', 0.8);
axis equal; grid on;
xlabel('X [m]'); ylabel('Y [m]');
title('FSAE Endurance Circuit');
legend('Ideal Trajectory', 'Left Cones(Blu)', 'Right Cones (Red)', 'Waypoints', 'Start');



%% 7. Generazione Geometria Coni per Simulation 3D Actor
% Crea la geometria combinata per tutti i coni SX (Blu)
cone_h = 0.5;   % Altezza cono [m]
cone_r = 0.175; % Raggio base [m]
n_sides = 12;   % Risoluzione circonferenza cono

% Coordinate vertici di un singolo cono
th_c = linspace(0, 2*pi, n_sides+1);
th_c(end) = [];
v_base = [cone_r * cos(th_c)', cone_r * sin(th_c)', zeros(n_sides, 1)];
v_apex = [0, 0, cone_h];
single_cone_verts = [v_base; v_apex];

% Triangolazione facce
faces = [];
for k = 1:n_sides
    k_next = mod(k, n_sides) + 1;
    faces = [faces; k, k_next, n_sides+1]; % Facce laterali
end

% Costruzione mesh per tutti i coni SX
verts_all_L = [];
faces_all_L = [];
for i = 1:size(cones_left, 1)
    offset_idx = size(verts_all_L, 1);
    % Nota: -cones_left(:,2) per convenzione SAE (Y a destra)
    v_shifted = single_cone_verts + [cones_left(i, 1), -cones_left(i, 2), 0];
    verts_all_L = [verts_all_L; v_shifted];
    faces_all_L = [faces_all_L; faces + offset_idx];
end

% Salvataggio mesh STL nella cartella di lavoro
TR_left = triangulation(faces_all_L, verts_all_L);
stlwrite(TR_left, 'cones_left.stl');

% Costruzione mesh per tutti i coni DX (Rossi/Gialli)
verts_all_R = [];
faces_all_R = [];
for i = 1:size(cones_right, 1)
    offset_idx = size(verts_all_R, 1);
    v_shifted = single_cone_verts + [cones_right(i, 1), -cones_right(i, 2), 0];
    verts_all_R = [verts_all_R; v_shifted];
    faces_all_R = [faces_all_R; faces + offset_idx];
end

TR_right = triangulation(faces_all_R, verts_all_R);
stlwrite(TR_right, 'cones_right.stl');

disp('Mesh 3D dei coni generate con successo (cones_left.stl, cones_right.stl)!');