function [T_regen_RL, T_regen_RR, regen_active] = regen_fcn( ...
        throttle_pct, n_motorRL, n_motorRR, V, Ts, ...
        T_TVC_RL, T_TVC_RR, ...
        regen_T_max, regen_pedal_thr, regen_speed_min_rpm, regen_speed_crit_rpm, ...
        regen_I_peak_A, regen_I_cont_A, regen_I_peak_dur_s, ...
        regen_P_batt_max_W, V_dc)
%REGEN_FCN  Frenata rigenerativa con Torque Vectoring integrato.
%
%  Specchio del codice STM32 (Regen.c) con estensione differenziale TVC.
%  Usare come MATLAB Function block nel DriveController.
%
%  Architettura:
%    1. Calcola T_base (coppia regen simmetrica, negativa) tramite gli
%       stessi 5 stage di Regen.c — limiti BMS applicati sul base.
%    2. Estrae il componente differenziale del TVC:
%         delta = (T_TVC_RL - T_TVC_RR) / 2
%    3. Applica il differenziale ai due motori:
%         T_regen_RL = T_base + delta
%         T_regen_RR = T_base - delta
%    4. Clamp individuale per motore: [-regen_T_max, 0]
%       Il limite BMS è preservato sull'average (T_base), il differenziale
%       redistribuisce la coppia senza aumentare la potenza totale.
%
%  Vincolo BMS (TOTALE, non per motore):
%    Peak:  regen_I_peak_A [A] per i primi regen_I_peak_dur_s [s]
%    Cont:  regen_I_cont_A [A] dopo la finestra peak
%  Il timer si azzera quando il pilota NON richiede regen.
%
%  Stage 4 (derating tensione DC) omesso: V_dc non è dinamico nel modello.
%
%  INPUT:
%    T_TVC_RL/RR  : uscite torque vectoring per i due motori [Nm]
%                   (riutilizzate dal blocco TVC — stessa sorgente del modo drive)
%  OUTPUT:
%    T_regen_RL/RR : coppia finale per motore [Nm], negativa o zero
%    regen_active  : flag 0/1

    % ---- stato persistente (equivalente alle static C) ------------------
    persistent t_elapsed_s;
    if isempty(t_elapsed_s)
        t_elapsed_s = 0.0;
    end

    % ---- velocità media motori [rpm] ------------------------------------
    speed_avg_rpm = (abs(n_motorRL) + abs(n_motorRR)) * 0.5;

    % ---- condizione di abilitazione regen --------------------------------
    % Entra se:  pedal < soglia  E  speed > crit  E  veicolo in moto
    % Reset se:  pedal >= soglia  O  speed < crit  O  veicolo fermo
    should_regen = (throttle_pct <= regen_pedal_thr) && ...
                   (speed_avg_rpm > regen_speed_crit_rpm) && ...
                   (V > 0.5);

    if ~should_regen
        t_elapsed_s  = 0.0;   % timer si azzera → prossima frenata parte da peak
        T_regen_RL   = 0.0;
        T_regen_RR   = 0.0;
        regen_active = 0.0;
        return;
    end

    % ---- timer discreto (equivalente a HAL_GetTick diff) ----------------
    t_elapsed_s = t_elapsed_s + Ts;

    % ---- Stage 1: coppia base pedal-dipendente [Nm, negativa] -----------
    % pedal = 0         → T_base = -regen_T_max  (regen massima)
    % pedal = soglia    → T_base = 0             (nessuna regen)
    pedal_norm = throttle_pct / regen_pedal_thr;
    T_s1 = -regen_T_max * (1.0 - pedal_norm);

    % ---- Stage 2: fade-out velocità [k_vel ∈ [0,1]] --------------------
    if speed_avg_rpm >= regen_speed_min_rpm
        k_vel = 1.0;
    else
        k_vel = speed_avg_rpm / regen_speed_min_rpm;
    end
    T_s2 = T_s1 * k_vel;

    % ---- Stage 3: limite potenza batteria (per motore = totale/2) -------
    % |T_base| <= P_max_per_mot / omega_mot
    omega_rad_s = (2.0 * pi * speed_avg_rpm) / 60.0;
    omega_safe  = max(omega_rad_s, 1.0);     % evita div/0
    T_P_lim     = (regen_P_batt_max_W * 0.5) / omega_safe;
    T_s3        = max(T_s2, -T_P_lim);      % T_s2 negativo → clamp inferiore

    % ---- Stage 5: limite corrente BMS (peak → cont dopo 4 s) -----------
    % Limite TOTALE / 2 = per motore
    % T_I_lim = (I_lim_per_mot * V_dc) / omega_mot
    if t_elapsed_s < regen_I_peak_dur_s
        I_lim_per_mot = regen_I_peak_A * 0.5;   % 21/2 = 10.5 A  (burst)
    else
        I_lim_per_mot = regen_I_cont_A  * 0.5;  % 14/2 = 7.0  A  (cont)
    end
    T_I_lim = (I_lim_per_mot * V_dc) / omega_safe;
    T_base  = max(T_s3, -T_I_lim);              % coppia base BMS-safe, negativa

    % ---- Integrazione TVC differenziale ---------------------------------
    % delta = metà del differenziale TVC (già calcolato per il modo drive).
    % Aumenta la frenata sul motore esterno alla curva, riduce sull'interno
    % → il veicolo ruota correttamente anche a pedale alzato.
    % Il differenziale NON cambia la potenza totale assorbita dalla batteria:
    %   (T_base + delta) + (T_base - delta) = 2 * T_base  (invariante).
    delta = (T_TVC_RL - T_TVC_RR) * 0.5;

    T_out_RL = T_base + delta;
    T_out_RR = T_base - delta;

    % ---- Clamp individuale per motore: [-regen_T_max, 0] ----------------
    % Il clamp a 0 impedisce che TVC "spinga" in trazione durante la regen.
    % Il clamp a -regen_T_max protegge termicamente il singolo motore.
    T_regen_RL  = min(max(T_out_RL, -regen_T_max), 0.0);
    T_regen_RR  = min(max(T_out_RR, -regen_T_max), 0.0);
    regen_active = 1.0;
end
