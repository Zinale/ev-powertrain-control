%% IMPORTO DATI LATERALI PNEUMATICI NORMALIZZATI
    % Valori andamento Cornering Stiffness (C_alpha.vs.Fz)
Fz_lat = xlsread('Hoosier_18x6x10R25B.xlsx','C:C');
C_alpha = xlsread('Hoosier_18x6x10R25B.xlsx','D:D'); 
u_y = 1.55;

    % Valori coefficienti interpolazione Magic Formula
Coeff_Blat = xlsread('Hoosier_18x6x10R25B.xlsx','L:L');
Coeff_Clat = xlsread('Hoosier_18x6x10R25B.xlsx','O:O');
Coeff_Dlat = xlsread('Hoosier_18x6x10R25B.xlsx','R:R');
Coeff_Elat = xlsread('Hoosier_18x6x10R25B.xlsx','U:U');
    
IA_lat = Coeff_Blat(7,1);    % Inclination Angle (Camber)
P_lat = Coeff_Blat(8,1);     % Pressione
    % B
% p00_Blat = Coeff_Blat(1,1);
% p10_Blat = Coeff_Blat(2,1);
% p01_Blat = Coeff_Blat(3,1);
% p20_Blat = Coeff_Blat(4,1);
% p11_Blat = Coeff_Blat(5,1);
% p02_Blat = Coeff_Blat(6,1);
B_lat = Coeff_Blat(9,1);
    % C
% p00_Clat = Coeff_Clat(1,1);
% p10_Clat = Coeff_Clat(2,1);
% p01_Clat = Coeff_Clat(3,1);
% p20_Clat = Coeff_Clat(4,1);
% p11_Clat = Coeff_Clat(5,1);
% p02_Clat = Coeff_Clat(6,1);
C_lat = Coeff_Clat(9,1);
    % D
% p00_Dlat = Coeff_Dlat(1,1);
% p10_Dlat = Coeff_Dlat(2,1);
% p01_Dlat = Coeff_Dlat(3,1);
% p20_Dlat = Coeff_Dlat(4,1);
% p11_Dlat = Coeff_Dlat(5,1);
% p02_Dlat = Coeff_Dlat(6,1);
D_lat = Coeff_Dlat(9,1);
    % E
% p00_Elat = Coeff_Elat(1,1);
% p10_Elat = Coeff_Elat(2,1);
% p01_Elat = Coeff_Elat(3,1);
% p20_Elat = Coeff_Elat(4,1);
% p11_Elat = Coeff_Elat(5,1);
% p02_Elat = Coeff_Elat(6,1);
E_lat = Coeff_Elat(9,1);

    % Valori normalizzati di slip angle e Fy calcolati con Magic Formula
SA_norm = xlsread('Hoosier_18x6x10R25B.xlsx','G:G');
Fy_norm = xlsread('Hoosier_18x6x10R25B.xlsx','H:H');

%% IMPORTO DATI LONGITUDINALI PNEUMATICI NORMALIZZATI
    % Valori andamento Stiffness (Kx.vs.Fz)
Fz_long = xlsread('Hoosier_18x6x10R25B.xlsx','AC:AC');
Kx = xlsread('Hoosier_18x6x10R25B.xlsx','AD:AD'); 
u_x = 1.5;

    % Valori coefficienti interpolazione Magic Formula
Coeff_Blong = xlsread('Hoosier_18x6x10R25B.xlsx','AL:AL');
Coeff_Clong = xlsread('Hoosier_18x6x10R25B.xlsx','AO:AO');
Coeff_Dlong = xlsread('Hoosier_18x6x10R25B.xlsx','AR:AR');
Coeff_Elong = xlsread('Hoosier_18x6x10R25B.xlsx','AU:AU');
    
IA_long = Coeff_Blong(7,1);    % Inclination Angle (Camber)
P_long = Coeff_Blong(8,1);     % Pressione
    % B
% p00_Blong = Coeff_Blong(1,1);
% p10_Blong = Coeff_Blong(2,1);
% p01_Blong = Coeff_Blong(3,1);
% p20_Blong = Coeff_Blong(4,1);
% p11_Blong = Coeff_Blong(5,1);
% p02_Blong = Coeff_Blong(6,1);
B_long = Coeff_Blong(9,1);
    % C
% p00_Clong = Coeff_Clong(1,1);
% p10_Clong = Coeff_Clong(2,1);
% p01_Clong = Coeff_Clong(3,1);
% p20_Clong = Coeff_Clong(4,1);
% p11_Clong = Coeff_Clong(5,1);
% p02_Clong = Coeff_Clong(6,1);
C_long = Coeff_Clong(9,1);
    % D
% p00_Dlong = Coeff_Dlong(1,1);
% p10_Dlong = Coeff_Dlong(2,1);
% p01_Dlong = Coeff_Dlong(3,1);
% p20_Dlong = Coeff_Dlong(4,1);
% p11_Dlong = Coeff_Dlong(5,1);
% p02_Dlong = Coeff_Dlong(6,1);
D_long = Coeff_Dlong(9,1);
    % E
% p00_Elong = Coeff_Elong(1,1);
% p10_Elong = Coeff_Elong(2,1);
% p01_Elong = Coeff_Elong(3,1);
% p20_Elong = Coeff_Elong(4,1);
% p11_Elong = Coeff_Elong(5,1);
% p02_Elong = Coeff_Elong(6,1);
E_long = Coeff_Elong(9,1);

    % Valori normalizzati di slip ratio e Fx calcolati con Magic Formula
SL_norm = xlsread('Hoosier_18x6x10R25B.xlsx','AG:AG');
Fx_norm = xlsread('Hoosier_18x6x10R25B.xlsx','AH:AH');

%% PLOT DEI DATI

% figure;
% plot(SA_norm,Fy_norm);
% title ('Normalized Lateral Data');
% xlabel('SA');
% ylabel('Fy');
% 
% figure;
% plot(SL_norm,Fx_norm);  hold on
% title ('Normalized Longitudinal Data');
% xlabel('SL');
% ylabel('Fx');

