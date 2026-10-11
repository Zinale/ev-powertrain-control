%% Tracciato FSAE SKIDPAD
% clear; clc; close all;

d_centers = 18.25;                  % [m] Distanza esatta tra i centri
R_in      = 15.25 / 2;              % [m] 7.625 m (Raggio interno)
track_w   = 3.50;                   % [m] Larghezza corsia
R_out     = 21.25 / 2;              % [m] 10.625 m (Raggio esterno)
R_center  = d_centers / 2;          % [m] 9.125 m (Raggio traiettoria mezzeria)

% Centri dei due cerchi disposti lungo Y
% Cerchio Superiore (+Y / Sinistro), Cerchio Inferiore (-Y / Destro)
center_top = [15,  R_center];       % [15,  9.125]
center_bot = [15, -R_center];       % [15, -9.125]

% ay_max deve restare SOTTO il limite fisico di aderenza (mu*g), con un
% margine di sicurezza: un fattore > 1 chiede al modello gomma piu'
% accelerazione laterale di quanta ne possa realmente fornire (stesso
% errore, piu' lieve, del tentativo con 1.4g che causava la spirale
% incontrollata sullo skidpad).
ay_max = 1.3 * mu * 9.81;           % [m/s^2] Stima aderenza laterale
ds = 0.1;                           % [m] Risoluzione spaziale
dtheta = ds / R_center;

%% 1. Traiettoria Ideale (Start in 0,0 lungo +X -> 2 giri DX -> 2 giri SX -> Uscita)
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

%% 2. Posizionamento Coni (vedere regolamento)
% A. Coni Interni
th_in = linspace(0, 2*pi, 18); th_in(end) = []; % 17 angoli esatti
cones_in_top = [center_top(1) + R_in * cos(th_in)', center_top(2) + R_in * sin(th_in)'];
cones_in_bot = [center_bot(1) + R_in * cos(th_in)', center_bot(2) + R_in * sin(th_in)'];

% B. Coni Esterni (copertura quasi completa, gap solo verso il punto
% di tangenza tra i due cerchi, dove i boundary esterni si sovrappongono)
% Gap minimo geometrico: i due cerchi esterni (raggio R_out) si
% intersecano a (15 +- h, 0), con h = sqrt(R_out^2 - (d_centers/2)^2);
% un gap troppo stretto lascia gli estremi dei due archi vicinissimi
% tra loro proprio in quella zona (da cui il cluster visto in figura).
% outer_gap_deg=110 porta gli estremi a y~=3m, ben sopra/sotto i coni
% del cancello (y=+-1.5m), eliminando la sovrapposizione.
outer_gap_deg = 110;   % [gradi] ampiezza del gap, da verificare sul regolamento/diagramma ufficiale
cone_spacing_deg = 10; % stessa spaziatura angolare usata finora (13 coni su 120°)

arc_span_deg = 360 - outer_gap_deg;
N_out = round(arc_span_deg / cone_spacing_deg) + 1;

% Cerchio superiore: gap centrato a -90° (verso il cerchio inferiore)
start_top = -90 + outer_gap_deg/2;
end_top   = start_top + arc_span_deg;
th_out_top = linspace(start_top*pi/180, end_top*pi/180, N_out);
cones_out_top = [center_top(1) + R_out * cos(th_out_top)', center_top(2) + R_out * sin(th_out_top)'];

% Cerchio inferiore: gap centrato a +90° (verso il cerchio superiore)
start_bot = 90 + outer_gap_deg/2;
end_bot   = start_bot + arc_span_deg;
th_out_bot = linspace(start_bot*pi/180, end_bot*pi/180, N_out);
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

%% 3. Segnali e Struttura Unificata per Simulink
ds_arr = sqrt(diff(X_ref).^2 + diff(Y_ref).^2);
s_ref  = [0, cumsum(ds_arr)];

dX = gradient(X_ref, s_ref);
dY = gradient(Y_ref, s_ref);
ddX = gradient(dX, s_ref);
ddY = gradient(dY, s_ref);

psi_ref = unwrap(atan2(dY, dX));

% Curvatura con segno (necessaria per il feedforward del Pure Pursuit,
% che deve rinforzare lo sterzo nella direzione corretta, non sempre
% nella stessa): positiva in senso antiorario, negativa in orario.
curvature_signed = (dX .* ddY - dY .* ddX) ./ max((dX.^2 + dY.^2).^(3/2), 1e-6);

% Magnitudine, usata SOLO per il profilo di velocita' (ay_max/curvature
% deve restare un rapporto positivo sotto radice)
curvature = abs(curvature_signed);
curvature(curvature < 1e-4) = 1e-4;

% Velocita' massima ammessa dalla sola aderenza laterale
V_lat_max = min(sqrt(ay_max ./ curvature), 12.0);

% Profilo a "doppia passata" (quasi-steady-state lap simulation): impone
% anche i limiti di accelerazione/frenata reali, cosi' il riferimento
% non chiede mai una variazione di velocita' piu' rapida di quella che
% il veicolo puo' fisicamente ottenere. Sostituisce lo smoothing
% arbitrario (movmean) con un vincolo fisico esplicito, ed elimina le
% lunghe fasi "senza acceleratore" dovute a un gradino non anticipato.
a_accel_max = 6.0;   % [m/s^2] accelerazione longitudinale disponibile [TUNABLE]
a_brake_max = 6.0;   % [m/s^2] decelerazione massima disponibile [TUNABLE]

N = numel(V_lat_max);

% Passata all'indietro: limita quanto velocemente si puo' FRENARE prima
% di un tratto piu' lento (deve iniziare a rallentare in anticipo)
V_bwd = V_lat_max;
for i = N-1:-1:1
    V_bwd(i) = min(V_lat_max(i), sqrt(V_bwd(i+1)^2 + 2*a_brake_max*ds));
end

% Passata in avanti: limita quanto velocemente si puo' ACCELERARE,
% partendo da fermo (v0=0) come il veicolo reale al via
V_ref = V_bwd;
V_ref(1) = min(V_bwd(1), sqrt(2*a_accel_max*ds));
for i = 2:N
    V_ref(i) = min(V_bwd(i), sqrt(V_ref(i-1)^2 + 2*a_accel_max*ds));
end

trackData.s     = s_ref';
trackData.X     = X_ref';
trackData.Y     = Y_ref';
trackData.psi   = psi_ref';
trackData.V     = V_ref';
trackData.kappa = curvature_signed';   % con segno, per il feedforward del Pure Pursuit
trackData.Ltot  = s_ref(end);
trackData.is_closed = false;   % tracciato aperto: rettilineo-cerchi-rettilineo
%% 4. Render Grafico Mappa
figure('Name', 'Formula Student Skidpad Track', 'Color', 'w', 'Position', [100, 100, 950, 700]);
plot(X_ref, Y_ref, 'k--', 'LineWidth', 1.4); hold on;
plot(cones_left(:,1),  cones_left(:,2),  'b^', 'MarkerFaceColor', 'b', 'MarkerSize', 5.5);
plot(cones_right(:,1), cones_right(:,2), 'r^', 'MarkerFaceColor', 'r', 'MarkerSize', 5.5);
plot(cones_center_line(:,1), cones_center_line(:,2), 'y^', 'MarkerFaceColor', 'y', 'MarkerSize', 6);
plot([15, 15], [-1.5, 1.5], 'm-', 'LineWidth', 2); % Linea Start/Finish
quiver(0, 0, 8, 0, 0, 'g', 'LineWidth', 2.5, 'MaxHeadSize', 0.8);

axis equal; grid on;
xlabel('X [m]'); ylabel('Y [m]');
title('Formula Student Skidpad Track');
legend('Ideal Trajectory', 'Blue Cones', 'Red Cones', 'Middle Cones', 'Start/Finish Line', 'Start', 'Location', 'northeast');
drawnow;

%% 5. Generazione Mesh STL per il Blocco 3D Actor in Unreal
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