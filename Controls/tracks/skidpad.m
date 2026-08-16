%% Tracciato FSAE SKIDPAD 
% clear; clc; close all;

d_centers = 18.25;                  % [m] Distanza esatta tra i centri
R_in      = 15.25 / 2;              % [m] 7.625 m (Raggio interno)
track_w   = 3.00;                   % [m] Larghezza corsia
R_out     = 21.25 / 2;              % [m] 10.625 m (Raggio esterno)
R_center  = d_centers / 2;          % [m] 9.125 m (Raggio traiettoria mezzeria)

% Centri dei due cerchi disposti lungo Y
% Cerchio Superiore (+Y / Sinistro), Cerchio Inferiore (-Y / Destro)
center_top = [15,  R_center];       % [15,  9.125]
center_bot = [15, -R_center];       % [15, -9.125]

ay_max = 1.2 * 9.81;                % [m/s^2] Stima aderenza laterale
ds = 0.1;                           % [m] Risoluzione spaziale
dtheta = ds / R_center;

% 1. Traiettoria Ideale (Start in 0,0 lungo +X -> 2 giri DX -> 2 giri SX -> Uscita)
% A. Rettilineo di Ingresso (da X=0 a X=15 su Y=0)
x_in = 0:ds:15;
y_in = zeros(size(x_in));

% B. 2 Giri Cerchio Inferiore / Destro (Senso Orario)
% Parte da (15, 0) con angolo pi/2 e decresce di 4*pi
th_bot = linspace(pi/2, pi/2 - 4*pi, round(4*pi/dtheta));
x_circ_bot = center_bot(1) + R_center * cos(th_bot);
y_circ_bot = center_bot(2) + R_center * sin(th_bot);

% C. 2 Giri Cerchio Superiore / Sinistro (Senso Antiorario)
% Parte da (15, 0) con angolo -pi/2 e cresce di 4*pi
th_top = linspace(-pi/2, -pi/2 + 4*pi, round(4*pi/dtheta));
x_circ_top = center_top(1) + R_center * cos(th_top);
y_circ_top = center_top(2) + R_center * sin(th_top);

% D. Rettilineo di Uscita (da X=15 a X=35 lungo +X su Y=0)
x_out = 15:ds:35;
y_out = zeros(size(x_out));

% Concatenazione percorso continuo
X_ref = [x_in, x_circ_bot(2:end), x_circ_top(2:end), x_out(2:end)];
Y_ref = [y_in, y_circ_bot(2:end), y_circ_top(2:end), y_out(2:end)];

% 2. Posizionamento Coni (vedere regolamento)
% A. Coni Interni 
th_in = linspace(0, 2*pi, 18); th_in(end) = []; % 17 angoli esatti
cones_in_top = [center_top(1) + R_in * cos(th_in)', center_top(2) + R_in * sin(th_in)'];
cones_in_bot = [center_bot(1) + R_in * cos(th_in)', center_bot(2) + R_in * sin(th_in)'];

% B. Coni Esterni
th_out_top = linspace(pi/6, 5*pi/6, 13);
cones_out_top = [center_top(1) + R_out * cos(th_out_top)', center_top(2) + R_out * sin(th_out_top)'];

th_out_bot = linspace(-5*pi/6, -pi/6, 13);
cones_out_bot = [center_bot(1) + R_out * cos(th_out_bot)', center_bot(2) + R_out * sin(th_out_bot)'];

% C. Coni Cancello Ingresso e Uscita (Larghezza 3m, Y = +/- 1.5m)
x_gates = [0, 5, 10, 20, 25, 30];
cones_gate_top = [x_gates',  1.5 * ones(size(x_gates))'];
cones_gate_bot = [x_gates', -1.5 * ones(size(x_gates))'];

% D. 4 Coni di Start/Finish
cones_center_line = [15,  1.5; 
                     15, -1.5];

% Raggruppamento per colore (Blu SX / Rossi DX)
cones_left  = [cones_in_top; cones_out_bot; cones_gate_top];
cones_right = [cones_in_bot; cones_out_top; cones_gate_bot];

% 3. Segnali e Struttura Unificata per Simulink
ds_arr = sqrt(diff(X_ref).^2 + diff(Y_ref).^2);
s_ref  = [0, cumsum(ds_arr)];

dX = gradient(X_ref, s_ref);
dY = gradient(Y_ref, s_ref);
ddX = gradient(dX, s_ref);
ddY = gradient(dY, s_ref);

curvature = abs(dX .* ddY - dY .* ddX) ./ max((dX.^2 + dY.^2).^(3/2), 1e-6);
curvature(curvature < 1e-4) = 1e-4;
psi_ref = unwrap(atan2(dY, dX));

% Profilo di velocità teorico costante sui cerchi
V_ref = min(sqrt(ay_max ./ curvature), 12.0);
V_ref = movmean(V_ref, 5);

trackData.s     = s_ref';
trackData.X     = X_ref';
trackData.Y     = Y_ref';
trackData.psi   = psi_ref';
trackData.V     = V_ref';
trackData.kappa = curvature';
trackData.Ltot  = s_ref(end);

% 4. Render Grafico Mappa
figure('Name', 'FSAE Skidpad Track', 'Color', 'w', 'Position', [100, 100, 950, 700]);
plot(X_ref, Y_ref, 'k--', 'LineWidth', 1.4); hold on;
plot(cones_left(:,1),  cones_left(:,2),  'b^', 'MarkerFaceColor', 'b', 'MarkerSize', 5.5);
plot(cones_right(:,1), cones_right(:,2), 'r^', 'MarkerFaceColor', 'r', 'MarkerSize', 5.5);
plot(cones_center_line(:,1), cones_center_line(:,2), 'y^', 'MarkerFaceColor', 'y', 'MarkerSize', 6);
plot([15, 15], [-1.5, 1.5], 'm-', 'LineWidth', 2); % Linea Start/Finish
quiver(0, 0, 8, 0, 0, 'g', 'LineWidth', 2.5, 'MaxHeadSize', 0.8);

axis equal; grid on;
xlabel('X [m]'); ylabel('Y [m]');
title('FSAE Skidpad');
legend('Ideal Trajectory', 'Blue Cones', 'Red Cones', 'Middle Cones', 'Start/Finish Line', 'Start', 'Location', 'northeast');
drawnow;

% 5. Generazione Mesh STL per il Blocco 3D Actor in Unreal
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

disp('Skidpad Regolamentare (D4) generato: mesh STL aggiornate!');