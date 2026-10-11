
clc
close all


% Tutte le lunghezze hanno unità di misura mm
% Tutte le masse hanno unità di misura kg
% Il resto delle grandezze presenta l'unità di misura indicato a lato
g = 9.81;         %__________accelerazione gravitazionele (m/s2)
pig = 3.141592;   %__________pi greco

%% INFO CONFIGURAZIONE
% Inverter in configurazione chiusa posizionato sopra il pacco batteria
% nella zona posteriore dell'auto

% Motori outboard

%% GEOMETRIE MODELLO
    
l = 1530;         %__________passo  
t = 1250;         %__________carreggiata 
D_ruota = 457.2;  %__________diametro ruota

%% MASSE e CENTRI di GRAVITA'

m_t = 45;                           %__________massa TELAIO
m_a = 15;                           %__________massa AERO
m_b = 65;                           %__________massa BATTERIE 
m_p = 85;                           %__________massa PILOTA
m_inv = 11;                         %__________massa INVERTER
m_spr = m_t+m_b+m_p+m_inv+m_a;      %__________masse sospese TOT 


m_m = 3.55;                      %__________massa MOTORE
m_e = 1.5;                         %__________massa EPICICLOIDALE
m_s = 2;                         %__________massa SOSPENSIONE + FRENO
m_c = 2;                         %__________massa CERCHIO
m_g = 4;                         %__________massa GOMMA
m_unspr = m_m+m_e+m_s+m_c+m_g;   %__________masse non sospese TOT

m_TOT = m_spr + 4 * m_unspr;           %____________massa TOTALE VEICOLO


xCoG_t = 182.9;                 %__________coordinate CoG TELAIO
yCoG_t = 0;
zCoG_t = 346.98;

xCoG_a = 655.1;                   %__________coordinate CoG AERO
yCoG_a = 0;
zCoG_a = 156.9;

xCoG_b = -505;                %__________coordinate CoG BATTERIE
yCoG_b = 0;
zCoG_b = 136.5;

xCoG_p = 251.35;                %__________coordinate CoG PILOTA
yCoG_p = 0;
zCoG_p = 394.210;

xCoG_inv = -305;                 %___________coordinate Cog INVERTER
yCoG_inv = 0;
zCoG_inv = 317.5;

xCoG_unspr = l/2;                %__________coordinate CoG MASSE NON SOSPESE
yCoG_unspr = t/2;
zCoG_unspr = D_ruota/2;

xCoG_spr = (xCoG_t*m_t + xCoG_a*m_a + xCoG_b*m_b + xCoG_p*m_p + xCoG_inv*m_inv)/m_spr;         %__________coordinate CoG MASSE SOSPESE
yCoG_spr = (yCoG_t*m_t + yCoG_a*m_a + yCoG_b*m_b + yCoG_p*m_p + yCoG_inv*m_inv)/m_spr; 
zCoG_spr = (zCoG_t*m_t+zCoG_a*m_a+zCoG_b*m_b+zCoG_p*m_p+zCoG_inv*m_inv)/m_spr;

xCoG_TOT = (xCoG_spr*m_spr+0*xCoG_unspr*m_unspr)/m_TOT;               %__________coordinate CoG TOTALE VEICOLO
yCoG_TOT = 0;
zCoG_TOT = (zCoG_spr*m_spr+4*zCoG_unspr*m_unspr)/m_TOT;

a = l/2-xCoG_TOT;                  %__________distanza baricientro asse anteriore
b = l-a;                           %__________distanza baricientro asse posteriore
a_s = l/2-xCoG_unspr;              %__________distanza baricientro masse sospese asse anteriore
b_s = l-a_s;                       %__________distanza baricientro masse sospese asse posteriore

m_spr_front = m_spr*b/l;           %__________massa sospesa anteriore
m_spr_rear = m_spr*a/l;            %__________massa sospesa posteriore

xCoG_RA = 765;                    %___________coordinate Cog RUOTA ANTERIORE
yCoG_RA = 0;
zCoG_RA = 228.6;

xCoG_RP = -765;                     %___________coordinate CoG RUOTA POSTERIORE
yCoG_RP = 0;
zCoG_RP = 228.6;
%% AERODINAMICA

rho = 1.225;                       %__________densità aria(kg/m3)

FW_area = 0.312;                   %__________area Front Wing (m2)
FW_df = 70.15;                     %__________downforce Front Wing
FW_dr = 9.05;                     %__________drag Front Wing
xCP_Fw = 1462;                    %__________coordinate Centro di Pressione

RW_area = 0.301;                   %__________area Rear Wing (m2)
RW_df = 59.6;                      %__________downforce Rear Wing
RW_dr = 17.06;                     %__________drag Rear Wing
xCP_Rw = -895;                      %__________coordinate Centro di Pression

FLR_area = 1.015;                  %__________area Fondo (m2)
FLR_df = 36.2;                     %__________downforce Fondo
FLR_dr = 10.5;                     %__________drag fondo
xCP_FLR = 224;                      %__________coordinate Centro di Pressione

P_area = 0.297;                        %__________area Pance(m2)
P_df = -17.76;                      %__________lift Pance
P_dr = 3.13;                       %__________drag Pance
xCP_P = 75;                       %__________coordinate Centro di Pressione

Vv_area = 1;                     %__________area veicolo senza aerodinamica(m2)
Vv_df  = -16.05;                     %__________lift veicolo senza aerodinamica
Vv_dr  = 13;                      %__________drag veicolo senza aerodinamica
xCP_Vv = 447;                       %__________coordinate Centro di Pressione

TOT_area = 1.85;                 %__________area TOT VEICOLO (m2)          
TOT_dr = 52.7;                  %__________drag TOT VEICOLO
TOT_df = 130;                   %__________downforce TOT VEICOLO
                        


%% GEOMETRIE RIGIDEZZE e TRASFERIMENTI DI CARICO (Dinamica Laterale)

h_CR_front = 55;                                                                                %__________altezza centro rollio anteriore                            
h_CR_rear = 50;                                                                                 %__________altezza centro rollio anteriore
Delta_h_CR = -5;                                                                                %__________differenza altezza centro rollio anteriore/posteriore
h_RM = zCoG_TOT-(52.5);                                                                         %__________distanza baricentro asse rollio
Grad_Roll = 0.5;                                                                                %__________gradiente di rollio (deg/g)
Ratio = 0.53;                                                                                   %__________ratio (rapporto ripartizione rigidezza)

F_lat = m_spr * g;                                                                              %__________forza laterale per g masse sospese (N/g)
M_roll = F_lat * h_RM / 1000;                                                                   %__________momento rollio per g masse sospese (Nm/g)
K_roll_tot = (m_spr * g *(h_RM/1000)*(-Grad_Roll*pig/180-1)/(-Grad_Roll*pig/180))/(180/pig);    %__________rigidezza a rollio totale (Nm/deg)
K_roll_front = K_roll_tot * Ratio;                                                              %__________rigidezza a rollio anteriore (Nm/deg)
K_roll_rear = K_roll_tot * (1-Ratio);                                                           %__________rigidezza a rollio posteriore (Nm/deg)
K_ride_front = 2 * 57.3 * K_roll_front/(t/1000)^2/1000;                                         %__________ride rate anteriore (N/mm)
K_ride_rear = 2 * 57.3 * K_roll_rear/(t/1000)^2/1000;                                           %__________ride rate posteriore (N/mm)
omega_front = 1/(2*pig)*((K_ride_front*1000)/(m_spr_front/2))^(1/2);                            %__________frequenza naturale anteriore (Hz)
omega_rear = 1/(2*pig)*((K_ride_rear*1000)/(m_spr_rear/2))^(1/2);                               %__________frequenza naturale posteriore (Hz)

K_F = K_roll_front*(180/pig)-((l/1000)-(a_s/1000))*m_spr*g*(h_RM/1000)/(l/1000);                %__________Parametro 1 (pag 682 Milliken)
K_R = K_roll_rear*(180/pig)-(a_s/1000)*m_spr*g*(h_RM/1000)/(l/1000);                            %__________Parametro 2 (pag 682 Milliken)
DW_front = m_spr*g/(t/1000)*((h_RM/1000)*K_F/(K_roll_front*(180/pig)+K_roll_rear*(180/pig)- ... %__________trasferimento carico anteriore (N/g)
           m_spr*g*(h_RM/1000))+((l/1000)-(a_s/1000))*(h_CR_front/1000)/(l/1000))+ ...
           2*m_unspr*g*(zCoG_unspr/1000)/(t/1000);
DW_rear = m_spr*g/(t/1000)*((h_RM/1000)*K_R/(K_roll_front*(180/pig)+K_roll_rear*(180/pig)- ...  %__________trasferimento carico posteriore (N/g)
           m_spr*g*(h_RM/1000))+(a_s/1000)*(h_CR_rear/1000)/(l/1000))+ ...
           2*m_unspr*g*(zCoG_unspr/1000)/(t/1000);

%% GEOMETRIE RIGIDEZZE e TRASFERIMENTI DI CARICO (Dinamica Longitudinale)

Max_Dec = 1.3;                                                                                  %__________decelerazione massima (m/s2)
BrakeRatio_front = 0.7321;                                                                      %__________percentuale front braking
BrakeRatio_rear = 1-BrakeRatio_front;                                                           %__________percentuale rear braking
AccRatio_front = 0.2714;                                                                        %__________percentuale forza longitudinale sull'anteriore
AccRatio_rear = 1-AccRatio_front;                                                               %__________percentuale forza longitudinale sul posteriore
FxBrake_front = m_TOT*g*Max_Dec*BrakeRatio_front;                                               %__________forza frenante sull'anteriore (N)
FxBrake_rear = m_TOT*g*Max_Dec*BrakeRatio_rear;                                                 %__________forza frenante sul posteriore (N)

DW_long = (m_spr*zCoG_spr+4*m_unspr*zCoG_unspr)*g/l;                                            %__________trasferimento carico longitudinale (N/g)
Anti_Dive = 0.3;                                                                                %__________percentuale di Anti-Dive
Anti_Lift = 0.3;                                                                                %__________percentuale di Anti-Lift
phi_F = atan(Anti_Dive*DW_long/(FxBrake_front/Max_Dec));
phi_R = atan(Anti_Lift*DW_long/(FxBrake_rear/Max_Dec));
Lsvsa_front = 1550;
Lsvsa_rear = 1550;
Hsvsa_front = Lsvsa_front*tan(phi_F);
Hsvsa_rear = Lsvsa_rear*tan(phi_R);
x_pc = l/2-l*tan(phi_R)/(tan(phi_F)+tan(phi_R));
z_pc = l*tan(phi_F)*tan(phi_R)/(tan(phi_F)+tan(phi_R));
Delta_h_pc = zCoG_spr-z_pc;

K_pitch = 740;
Grag_Pitch = m_spr*g*(Delta_h_pc/1000)/K_pitch;

%% MOTORE ELETTRICO AMK e RIDUTTORE

P = 600000;                                         %__________potenza
Rpm_E = (0:1:20000);                                %__________giri motore
w_E = 2*pig * Rpm_E/60;                             %__________velocità angolare motore
C_E = P./((Rpm_E*2)*pig);                           %__________coppia motore
C_E(C_E>21) = 21;

tau = 13.33;                                        %__________rapporto trasmissione riduttore
% w_r = w_E/tau;                                    %__________velocità angolare ruota
% C_r = C_E*tau;                                    %__________coppia alla ruota

% plot(Rpm_E, C_E); hold on
% plot(Rpm_E, C_ruota);
  













