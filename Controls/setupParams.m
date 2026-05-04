clear; clc;

% =========================================================================
%  setupParams.m  —  Parametri globali DriveController
%  Esegui questo script PRIMA di simulare o generare codice.
%  I parametri contrassegnati con [TUNABLE] possono essere modificati
%  a runtime tramite Simulink.Parameter con StorageClass 'ExportedGlobal'.
% =========================================================================

%% -- Motore ---------------------------------------------------------------
T_peak      = 21.0;    % Coppia di picco    [Nm]
T_rated    = 9.8;     % Coppia in derating [Nm]  [TUNABLE]
mot_max_rpm = 20000;   % Giri massimi motore [rpm]
derating_on_thresh = 19;
derating_off_thresh = 20;

%% -- Veicolo --------------------------------------------------------------
n_wheels_f  = 2;
n_wheels_r  = 2;
mass        = 320;     % Massa vettura + pilota [kg]
Iz          = 120;     % Momento inerzia yaw    [kg·m²]
l_f         = 0.775;   % Distanza CG-asse ant.  [m]
l_r         = 0.775;   % Distanza CG-asse post. [m]
wheelbase   = l_f + l_r; %Passo del veicolo [m]
h_o         =0.35;      %Distanza verticale centro di massa [m]
track_width_f = 1.2;     % Carreggiata anteriore [m]
track_width_r = 1.2;     % Carreggiata posteriore [m]
track_width = sqrt(track_width_r^2 + track_width_f^2);

Izz         = 200;      %Yaw polar inerzia [kg*m^2]

Af          = 1.1;      %Area longitudinale drag [m^2] 
Cd          = 1.3;      
Cl       = -1;

rid_ratio   = 14.5;    % Rapporto di riduzione [-]
rid_eff     = 0.95;    % Efficienza riduttore  [-]
R_wheel     = 0.225;   % Raggio ruota          [m]

steer_ratio = 1/4.2;   % Steering ratio [rad_ruota / rad_volante]




%% -- Mappa Pedale (lookup table) ------------------------------------------
% Mappatura esponenziale: risposta docile a bassi input, aggressiva al massimo
% Input:  throttle grezzo [0..100] %
% Output: throttle normalizzato [0..1] da moltiplicare per T_peak
pedal_alpha   = 1.0;   % Esponente curva (1=lineare, >1=esponenziale)  [TUNABLE]
pedal_bp      = (0:10:100)';                          % Breakpoints [%]
pedal_map     = (pedal_bp / 100) .^ pedal_alpha;      % Valori normalizzati [0..1]
% Nel blocco Simulink: usare "1-D Lookup Table" con pedal_bp, pedal_map

%% -- TVC 
% PID Yaw --------------------------------------------------------
tvc_Kp      = 22;     % [TUNABLE]
tvc_Ki      = 35;     % [TUNABLE]
tvc_Kd      = 0;    % [TUNABLE]
tvc_sat_dMz = 16.0;    % Saturazione uscita PID [Nm]  (+-)
tvc_up_sat   = 15;
tvc_low_sat   = -15;

% Allocator ------------------------------------------------------
T_headroom_max = 8.0;  % Headroom massimo coppia per TVC  [Nm]  [TUNABLE]
T_headroom_k   = 10.0;  % Guadagno proporzionale          [TUNABLE]
%% -- SLC — PID  --------------------------------------------------------
slc_Kp      = 0.457;     % [TUNABLE]
slc_Ki      = 9.01;     % [TUNABLE]
slc_Kd      = 0;    % [TUNABLE]
slc_up_sat   = 17;
slc_low_sat   = -17;


%% --yaw_th ------------------------------------------------------
angle_limit = 2;



%% -- Mode Manager — soglie ------------------------------------------------
tvc_D_thresh     = 0.052;  % Soglia sterzo per attivare TVC [rad]  [TUNABLE]
tvc_D_thresh_off     = 0.03;  % Soglia sterzo per disattivare TVC [rad]  [TUNABLE]
tvc_yaw_thresh   = 0.1;    % Soglia yaw error per attivare TVC [rad/s]  [TUNABLE]
tvc_yaw_thresh_off   = 0.05;    % Soglia yaw error per disattivareTVC [rad/s]  [TUNABLE]
tvc_V_on         = 4.0;    % Velocità minima attivazione TVC [m/s]
tvc_V_off        = 2.0;    % Velocità massima disattivazione TVC [m/s]
tvc_throttle_on        = 3.0;    % Acceleratore minimo x attivazione TVC [m/s]

%% -- Slip Controller — PI -------------------------------------------------
slip_Kp         = 1;   % [TUNABLE]
slip_Ki         = 1;    % [TUNABLE]
slip_ref        = 0.0;    % Slip ratio di riferimento [-]  [TUNABLE]
slip_up_sat     = 1.0;
slip_low_sat     = 0.0;
slip_V_min      = 0.5;     % Velocità sotto cui disabilitare TCS [m/s]
slip_deadzone   = 0.02;    % Dead-zone su errore slip (noise rejection)  [TUNABLE]

%% -- Filtri ---------------------------------------------------------------
tau_yaw_filter  = 0.03;    % Costante filtro LP su yaw_meas [s]  [TUNABLE]
tau_Fz_filter   = 0.05;    % Costante filtro LP su Fz feedback [s]
tau_V_filter    = 0.02;    % Costante filtro LP su velocità ruote [s]


%% -- Simulazione -------------------------------------------------
Ts              = 0.01;    % Sample time controller [s]  — 100 Hz
air_temp        = 300;      % Temperatura ambiente [K]         
v0_speed        = 0;        %Velocità Vx iniziale [m/s] 
pressure        = 101325; %[Pa]
g               = 9.81;     %[m/s^2]

Kus             = 0.00;     %Gradiente di sottosterzo x yaw_th modello Bicycle Dinamico
mu              = 1.6;      %Coefficiente di attrito strada-ruota


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