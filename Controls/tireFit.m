clc;
clear all;
close all;

disp("Caricamento del modello Simulink in corso...");
load_system("peacockElettricaSimulink.slx");

disp("Lettura del file FSAE_Tire_Combined.tir...");
tm = tireModel.import("data/FSAE_Tire_Combined.tir");

disp("Esportazione dei parametri nel blocco Simulink...");
set(tm, "peacockElettricaSimulink/Combined Slip Wheel 2DOF", ...
    "VariableName", "tireParameters", ...
    "Overwrite", true);

disp("Importazione completata con successo!");
disp("Variabile 'tireParameters' creata nel Workspace e collegata al blocco.");

disp("Generazione dei grafici in corso...");
plot(tm);


plot(tm, Direction="alpha");
sgtitle('Risposta vs Slip angle \alpha - Magic Formula');
set(gcf, 'Name', 'Sweep alpha');

plot(tm, Direction="kappa");
sgtitle('Risposta vs Longitudinal slip \kappa - Magic Formula');
set(gcf, 'Name', 'Sweep kappa');

plot(tm, Direction="Fz");
sgtitle('Risposta vs Carico verticale Fz - Magic Formula');
set(gcf, 'Name', 'Sweep Fz');

plot(tm, Direction="gamma");
sgtitle('Risposta vs Camber angle \gamma - Magic Formula');
set(gcf, 'Name', 'Sweep gamma');


disp("Calcolo manuale Magic Formula PAC2002...");

Fz0     = 1112;                         % Fz nominale [N]
kappa   = linspace(-0.3, 0.3, 300);
alpha   = linspace(-0.5, 0.5, 300);
Fz_vals = [400, 800, 1112, 1400, 1600]; % range realistico
colors  = lines(numel(Fz_vals));

% Coefficienti longitudinali (dal file .tir)
PCX1=1.98430; PDX1=1.00347; PDX2=-0.10000;  % PDX2 corretto: era -0.99198
PKX1=14.41148; PKX2=12.31364; PKX3=-2.82405;
PEX1=-1.36507; PEX2=0.87772; PEX3=-0.57209; PEX4=0.01475;
PHX1=0.00178;  PHX2=-0.02561;
PVX1=0.09914;  PVX2=0.04065;

figure('Name', 'Fx MF PAC2002 - range FSAE'); hold on;
for i = 1:numel(Fz_vals)
    Fz  = Fz_vals(i);  dfz = (Fz - Fz0) / Fz0;
    Cx  = PCX1;
    Dx  = (PDX1 + PDX2*dfz) * Fz;              % gamma=0
    Kx  = Fz * (PKX1 + PKX2*dfz) * exp(PKX3*dfz);
    Bx  = Kx / (Cx * Dx + eps);
    Ex  = (PEX1 + PEX2*dfz + PEX3*dfz^2) .* (1 - PEX4*sign(kappa));
    SHx = PHX1 + PHX2*dfz;
    SVx = Fz * (PVX1 + PVX2*dfz);
    kx  = kappa + SHx;
    Fx  = Dx .* sin(Cx .* atan(Bx.*kx - Ex.*(Bx.*kx - atan(Bx.*kx)))) + SVx;
    plot(kappa, Fx, 'Color', colors(i,:), 'LineWidth', 1.5, ...
        'DisplayName', sprintf('Fz = %d N', Fz));
end
hold off; legend('Location', 'best');
title('Forza longitudinale Fx - Magic Formula PAC2002');
xlabel('\kappa [-]'); ylabel('Fx [N]'); grid on;

% Coefficienti laterali (dal file .tir)
PCY1=1.80950; PDY1=2.63285; PDY2=-0.49999;
PKY1=-21.27292; PKY2=0.77835;
PEY1=1.68127; PEY2=0.14432; PEY3=-0.02731;
PHY1=-0.00502; PHY2=-0.00932;
PVY1=-0.08608; PVY2=0.01447;

figure('Name', 'Fy MF PAC2002 - range FSAE'); hold on;
for i = 1:numel(Fz_vals)
    Fz  = Fz_vals(i);  dfz = (Fz - Fz0) / Fz0;
    Cy  = PCY1;
    Dy  = (PDY1 + PDY2*dfz) * Fz;              % gamma=0
    Ky  = PKY1 * Fz0 * sin(2 * atan(Fz / (PKY2 * Fz0 + eps)));
    By  = Ky / (Cy * Dy + eps);
    Ey  = (PEY1 + PEY2*dfz) .* (1 - PEY3*sign(alpha));
    SHy = PHY1 + PHY2*dfz;
    SVy = Fz * (PVY1 + PVY2*dfz);
    ay  = alpha + SHy;
    Fy  = Dy .* sin(Cy .* atan(By.*ay - Ey.*(By.*ay - atan(By.*ay)))) + SVy;
    plot(alpha, Fy, 'Color', colors(i,:), 'LineWidth', 1.5, ...
        'DisplayName', sprintf('Fz = %d N', Fz));
end
hold off; legend('Location', 'best');
title('Forza laterale Fy - Magic Formula PAC2002');
xlabel('\alpha [rad]'); ylabel('Fy [N]'); grid on;

disp("Tutti i grafici generati.");