clear; clc; close all;

%% -- Motore ---------------------------------------------------------------
T_peak      = 20;    % Coppia di picco    [Nm]
T_rated    = 9.8;     % Coppia in derating [Nm]  [TUNABLE]
w_engine    = 0.005;    %w motore (1/w_engine*s +1) 
mot_max_rpm = 20000;   % Giri massimi motore [rpm]
derating_on_thresh = 19;
derating_off_thresh = 20;

V_dc                = 540;       % Tensione nominale DC bus [V]  (bench; race = 540)
V_dc_p                = Simulink.Parameter(V_dc);
V_dc_p.StorageClass = 'ExportedGlobal';

I_discharge_max = 90;       %Ampere
I_charge_max    = 21;       %Ampere


rising_slew_rate_torque = 100;
falling_slew_rate_torque = -500;


%% -- Veicolo --------------------------------------------------------------
n_wheels_f  = 2;
n_wheels_r  = 2;
mass        = 320 ;     % Massa vettura + pilota [kg]
l_f         = 0.775;   % Distanza CG-asse ant.  [m]
l_r         = 0.775;   % Distanza CG-asse post. [m]
wheelbase   = l_f + l_r; %Passo del veicolo [m]
h_o         = 0.35;      %Distanza verticale centro di massa [m]
track_width_f = 1.2;     % Carreggiata anteriore [m]
track_width_r = 1.2;     % Carreggiata posteriore [m]
track_width = (track_width_r + track_width_f)/2;

Izz         = 200;      %Yaw polar inerzia [kg*m^2]

Af          = 1.1;      %Area longitudinale drag [m^2] 
Cd          = 1.3;      
Cl       = 1.0;

aero_bal_r = 0.5;       % % di downforce sul posteriore (es. 50%)
K_roll_rear = 0.6;      % % di rigidezza a rollio sul posteriore (es. 50%)


rid_ratio   = 15;    % Rapporto di riduzione [-]
rid_eff     = 0.80;    % Efficienza riduttore  [-]
R_wheel     = 0.2032;   % Raggio ruota          [m]



steer_ratio = 4.2;   % Steering ratio [rad_ruota / rad_volante]

camber_rear = -1;
pa_rear_wheel = 82700;      %12psi [Pa]
camber_front = -1.5;         % Camber anteriore [gradi] (solitamente più negativo del post.)
pa_front_wheel = 82700;      % Pressione anteriore 12psi [Pa]

brake_bias_f = 0.40;         % Ripartizione frenata all'anteriore (es. 60%)
brake_bias_r = 1 - brake_bias_f; % Ripartizione frenata al posteriore (40%)


mu_static =0.4;
mu_kinematic = 0.35;
disk_abore = 0.0348;
Rm = 0.07;
num_pads = 2;


%% -- TVC 
% PID Yaw --------------------------------------------------------
tvc_Kp      = 650;     % [TUNABLE]
tvc_Ki      = 150;     % [TUNABLE]
tvc_Kd      = 100;    % [TUNABLE]
tvc_sat_dMz = 700;    % Saturazione uscita PID [Nm]  (+-)
tvc_tr      =5;
tvc_bc      = 17;
tvc_N_filter = 25;

% Allocator ------------------------------------------------------
T_headroom_max = 9;  % Headroom massimo coppia per TVC  [Nm]  [TUNABLE]
T_headroom_k   = 13.0;  % Guadagno proporzionale          [TUNABLE]
rpm_safe_threshold = 100;


steering_deadband = 1;   % °sterzo

%% -- SLC — PID  --------------------------------------------------------
% slc_Kp      = 2.057;     % [TUNABLE]
% slc_Ki      = 10.01;     % [TUNABLE]
% slc_Kp      = 0.7057;     % [TUNABLE] 
% slc_Ki      = 1.01;     % [TUNABLE]
slc_Kp      = 0;     % [TUNABLE]
slc_Ki      = 0;     % [TUNABLE]
slc_Kd      = 0;    % [TUNABLE]
slc_up_sat   = T_peak;
slc_low_sat   = -T_peak;


%% --yaw_th ------------------------------------------------------
angle_limit_d = 1;   %[°]



%% -- Mode Manager - soglie (2 stati: IDLE_ / TVC_) ------------------------
% IDLE_->TVC_: [V > tvc_V_on && Throttle > tvc_throttle_on && brake == 0]
% TVC_->IDLE_: [brake > 0 || V <= tvc_V_off]
tvc_V_on        = 4.0;   % Velocità minima attivazione TVC  [m/s]  [TUNABLE]
tvc_V_off       = 2.0;   % Velocità disattivazione TVC      [m/s]  [TUNABLE]
tvc_throttle_on = 3.0;   % Throttle minimo attivazione TVC  [%]    [TUNABLE]
% Soglie sterzo/yaw non usate nel Mode Manager (TVC sempre attivo quando V>V_on)
tvc_D_thresh        = 0.060;  % [rad]
tvc_D_thresh_off    = 0.03;   % [rad]
tvc_yaw_thresh      = 0.08;    % [rad/s]
tvc_yaw_thresh_off  = 0.04;   % [rad/s]
e_yaw_deadzone      = 0.03;

%% -- Slip Controller (TCS) — Architettura SOTTRATTIVA ---------------------
slip_Kp         = 50.0;   % 50 [TUNABLE]
slip_Ki         = 13.0;    % [TUNABLE]
slip_Kd         = 0.8;    % [TUNABLE] 
slip_filt_N     = 80;     % Coefficiente filtro derivata (cutoff ≈ N/(2*pi*Ts) ≈ 318 Hz)
slip_ref        = 0.15;   % Slip ratio di riferimento [-]  [TUNABLE]  (ottimale ~0.10-0.20)
overslip_factor = 1.01;   % Fattore di sovraspinta per far innescare lo slip 
slip_up_sat     = 25; % Saturazione superiore PI [Nm]
slip_low_sat    = 0.0;    % Il PI non scende sotto 0 (Delta_T sempre positivo)
slip_bc_coeff   = 10;    % CoefficienteBack-Calculation PID [TUNABLE]
slip_V_min      = 3;    % Velocità sotto cui disabilitare TCS [m/s] 


%Acceleration MAP
% slip_Kp         = 190.0;   % [TUNABLE]
% slip_Ki         = 120.0;    % [TUNABLE]
% slip_Kd         = 5;    % [TUNABLE] 
% slip_filt_N     = 40;     % Coefficiente filtro derivata (cutoff ≈ N/(2*pi*Ts) ≈ 318 Hz)
% slip_ref        = 0.17;   % Slip ratio di riferimento [-]  [TUNABLE]  (ottimale ~0.10-0.20)
% overslip_factor = 1.10;   % Fattore di sovraspinta per far innescare lo slip 
% slip_up_sat     = 21; % Saturazione superiore PI [Nm]
% slip_low_sat    = 0.0;    % Il PI non scende sotto 0 (Delta_T sempre positivo)
% slip_bc_coeff   = 10;    % CoefficienteBack-Calculation PID [TUNABLE]
% slip_V_min      = 1;    % Velocità sotto cui disabilitare TCS [m/s] 
%% -- Mappa Pedale (lookup table) ------------------------------------------
% Input:  throttle grezzo [0..100] %
% Output: throttle normalizzato [0..1] da moltiplicare per T_peak
pedal_alpha   = 1;   % Esponente curva (1=lineare, >1=esponenziale)  [TUNABLE]
pedal_bp      = (0:10:100)';                          % Breakpoints [%]
pedal_map     = (pedal_bp / 100) .^ pedal_alpha;      % Valori normalizzati [0..1]
deathzone_APP = 5; % % sul valore 0-100
% Nel blocco Simulink: usare "1-D Lookup Table" con pedal_bp, pedal_map

%% -- Launch Control — Torque Ramp Limiter ---------------------------------
% Limita la derivata di T_req in uscita dalla pedal map.

launch_V_thresh = 5.0; %[m/s]
launch_ramp_rate = 1000.0;   %[N/s]
nominal_ramp_rate = 1000.0; 
    
%% -- Rigenerazione (specchio del codice STM32) ----------------------------
% Valori sincronizzati con Config.h e BaseControlMotor.h (bench configuration)
%
% Vincolo BMS (TOTALE, non per motore):
%   peak:  21 A per i primi 4 s di regen continua
%   cont:  14 A dopo i 4 s (finché il pilota non rilascia e poi ri-preme)
% Per motore (2 motori, simmetrico): I_lim / 2
%
% Formula coppia limite per corrente:  T_I_lim = (I_lim/2 * V_dc) / omega_mot
% Formula coppia limite per potenza:   T_P_lim = (P_batt_max/2)   / omega_mot

% regen_T_max         = T_rated/2;   % Coppia regen massima per motore [Nm] (50% Mn = 4.9)  [TUNABLE]
regen_T_max         = 9;
regen_pedal_thr     = 5.0;      % Soglia pedale ingresso regen [%]  — da Config.h
regen_pedal_hyst    = 3.0;       % Isteresi pedale [%]               — da Config.h
regen_speed_min_rpm = 4000;      % Velocità fade-out regen [rpm]     — da Config.h
regen_speed_crit_rpm= 1000;      % Velocità no-regen [rpm]           — da Config.h
regen_I_peak_A      = 21.0;      % Corrente batteria peak  [A] TOTALE — da Config.h
regen_I_cont_A      = 14.0;      % Corrente batteria cont  [A] TOTALE — da Config.h
regen_I_peak_dur_s  = 4.0;       % Durata finestra peak    [s]        — da Config.h
regen_P_batt_max_W  = 35000;     % Potenza regen massima   [W] TOTALE — da Config.h (bench 350V)
ay_limit = 15.0; % [m/s^2] Inizia a tagliare regen a circa 1.5g

%% -- Filtri ---------------------------------------------------------------
tau_yaw_filter  = 0.03;    % Costante filtro LP su yaw_meas [s]  [TUNABLE]
tau_Fz_filter   = 0.05;    % Costante filtro LP su Fz feedback [s]
tau_V_filter    = 0.02;    % Costante filtro LP su velocità ruote [s]


%% -- Simulazione -------------------------------------------------
a_lat_max       = 5;        %m/s^2
Ts              = 0.01;    % Sample time controller [s]  — 100 Hz
air_temp        = 300;      % Temperatura ambiente [K]         
v0_speed        = 0.00;        %Velocità Vx iniziale [m/s] 
pressure        = 101325; %[Pa]
g               = 9.81;     %[m/s^2]
rho = 1.225;            % Densità aria [kg/m^3]

Kus             = 0.03;     %Gradiente di sottosterzo x yaw_th modello Bicycle Dinamico
mu              = 1.5;      %Coefficiente di attrito strada-ruota
gnd_displ       = 0.0;      %Ground displacement along tire-fixed z-axis [m]
scale_factor_rear = ones(27, 1);        %da cambiare per simulare altre condizioni
scale_factor_front = ones(27, 1); % Scale factors per Magic Formula anteriore

auto_speed = 0;             %auto_speed = 0 -> Auto Throttle and Auto Brake; 1 Manual.

%% -- Parametri TUNABLE per codegen (Simulink.Parameter) ------------------
% Necessario solo se si genera codice C con Embedded Coder.
% Questi parametri diventano variabili globali modificabili a runtime
% In simulazione normale non serve decommentare questa sezione.
%
% slip_ref_p            = Simulink.Parameter(slip_ref);
% slip_ref_p.StorageClass = 'ExportedGlobal';
%
% tvc_Kp_p              = Simulink.Parameter(tvc_Kp);
% tvc_Kp_p.StorageClass   = 'ExportedGlobal';
%
% pedal_alpha_p           = Simulink.Parameter(pedal_alpha);
% pedal_alpha_p.StorageClass = 'ExportedGlobal';
%
% Nel blocco Simulink usare 'slip_ref_p' invece di 'slip_ref'.

disp('Parametri caricati correttamente!');