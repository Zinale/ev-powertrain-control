%% Generatore Tracciato FSAE ACCELERATION (Regole D5 - Start in (0,0) lungo +X)
% clear; clc; close all;

% --- Regolamento D5 ---
track_width    = 4.0;         % [m] Larghezza minima carreggiata (3.0 m)
L_accel        = 75.0;        % [m] Lunghezza prova  (Start -> Finish)
L_runout       = 60.0;        % [m] Spazio di arresto / Runout dopo il traguardo
L_total        = L_accel + L_runout; % [m] 135 m totali di rettilineo
cone_spacing   = 5.0;         % [m] Spaziatura coni lungo il rettilineo (~5 m)

ax_max_accel   = 1.0*1.1 * 9.81;  % [m/s^2] Max accelerazione longitudinale in trazione
v_max_top      = 35.0;        % [m/s] (~126 km/h) Velocità max raggiungibile
ds             = 0.2;         % [m] Risoluzione spaziale

% 1. Definizione Traiettoria (Rettilineo puro lungo +X da 0 a L_total)
X_ref = 0:ds:L_total;
Y_ref = zeros(size(X_ref));

% 2. Generazione Coni di Delimitazione (sx: Y = +1.5m, dx: Y = -1.5m)
x_cones = 0:cone_spacing:L_total;

cones_left  = [x_cones',  (track_width/2) * ones(size(x_cones))'];
cones_right = [x_cones', -(track_width/2) * ones(size(x_cones))'];

% 3. Calcolo Segnali e Profilo Velocità Target
ds_array = sqrt(diff(X_ref).^2 + diff(Y_ref).^2);
s_ref = [0, cumsum(ds_array)];

curvature = zeros(size(X_ref)) + 1e-4; % Curvatura nulla (rettilineo)
psi_ref   = zeros(size(X_ref));        % Heading costante = 0 rad (+X)

% Profilo di Accelerazione Massima (invariato rispetto a prima)
V_fwd = zeros(size(X_ref));
V_fwd(1) = 0.0; % Partenza da fermo
for i = 2:length(X_ref)
    ds_i = s_ref(i) - s_ref(i-1);
    if s_ref(i) <= L_accel
        % Piena accelerazione nei primi 75m
        V_fwd(i) = min(v_max_top, sqrt(V_fwd(i-1)^2 + 2 * ax_max_accel * ds_i));
    else
        % Mantenimento velocità al traguardo
        V_fwd(i) = V_fwd(i-1);
    end
end

% Oltre il traguardo (L_accel) il target è fisso a zero: il PID longitudinale
% (Sezione 4.3.1) vedrà un errore negativo e comanderà il freno a fondo scala
% per tutto il tempo necessario, senza una curva di frenata precalcolata qui.
V_ref = V_fwd;
V_ref(s_ref > L_accel) = 0.0;
V_ref = movmean(V_ref, 5);

% Struttura Unificata per Simulink
trackData.s     = s_ref';
trackData.X     = X_ref';
trackData.Y     = Y_ref';
trackData.psi   = psi_ref';
trackData.V     = V_ref';
trackData.kappa = curvature';
trackData.Ltot  = s_ref(end);
trackData.is_closed = true;    % giro che si richiude su se stesso

% 4. Render Grafico della Pista di Accelerazione
figure('Name', 'Formula Student Acceleration Track', 'Color', 'w', 'Position', [100, 100, 1050, 450]);
plot(X_ref, Y_ref, 'k--', 'LineWidth', 1.4); hold on;
plot(cones_left(:,1),  cones_left(:,2),  'b^', 'MarkerFaceColor', 'b', 'MarkerSize', 5.5);
plot(cones_right(:,1), cones_right(:,2), 'r^', 'MarkerFaceColor', 'r', 'MarkerSize', 5.5);

% Linee Start (0m) e Finish (75m)
plot([0, 0],             [-track_width/2, track_width/2], 'g-', 'LineWidth', 2.5);
plot([L_accel, L_accel], [-track_width/2, track_width/2], 'm-', 'LineWidth', 2.5);
plot([L_total, L_total], [-track_width/2, track_width/2], 'k-', 'LineWidth', 2.0);

quiver(0, 0, 15, 0, 0, 'g', 'LineWidth', 2.5, 'MaxHeadSize', 0.8);
axis equal; grid on;
ylim([-8, 8]);
xlabel('X [m]'); ylabel('Y [m]');
title('Formula Student Acceleration Track');
legend('Trajectory', 'Cones Left (Blu)', 'Cones Right(Red)', ...
       'Start Line (0m)', 'Finish Line (75m)', 'End of Runout', 'Start (+X)', 'Location', 'best');
drawnow;

% 5. Generazione Mesh STL per Simulation 3D Actor (Unreal Engine)
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

% Mesh SX
verts_all_L = []; faces_all_L = [];
for i = 1:size(cones_left, 1)
    offset_idx = size(verts_all_L, 1);
    v_shifted = single_cone_verts + [cones_left(i, 1), -cones_left(i, 2), 0];
    verts_all_L = [verts_all_L; v_shifted];
    faces_all_L = [faces_all_L; faces + offset_idx];
end
stlwrite(triangulation(faces_all_L, verts_all_L), 'cones_left.stl');

% Mesh DX
verts_all_R = []; faces_all_R = [];
for i = 1:size(cones_right, 1)
    offset_idx = size(verts_all_R, 1);
    v_shifted = single_cone_verts + [cones_right(i, 1), -cones_right(i, 2), 0];
    verts_all_R = [verts_all_R; v_shifted];
    faces_all_R = [faces_all_R; faces + offset_idx];
end
stlwrite(triangulation(faces_all_R, verts_all_R), 'cones_right.stl');
xlim([-5 140]);
disp('Acceleration Track generato: cones_left.stl e cones_right.stl aggiornati!');