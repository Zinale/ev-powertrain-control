% Loads FSAE lap telemetry from a MoTeC-exported .mat file.
% Trims pre-race idle, normalizes time to t=0, and exports channels
% to the base workspace as column vectors for Simulink.

fprintf('[Telemetry] Loading %s...\n', 'test_trial_lap (1).mat');

% -------------------------------------------------------------------------
% 1. Load file and unwrap nested struct if necessary
% -------------------------------------------------------------------------
filename    = 'test_trial_lap (1).mat';
raw_load    = load(filename);
fields      = fieldnames(raw_load);

if isscalar(fields) && isstruct(raw_load.(fields{1}))
    data_source = raw_load.(fields{1});
else
    data_source = raw_load;
end

% -------------------------------------------------------------------------
% 2. Extract channels, trim idle, and normalize
% -------------------------------------------------------------------------
try
    % Core channels
    raw_time  = data_source.Throttle_Pos.Time;
    raw_accel = data_source.Throttle_Pos.Value / 100;
    raw_brake = data_source.Brake_Pos.Value    / 100;
    
    % Steering Angle
    if isfield(data_source, 'Steering_Angle')
        raw_steer = data_source.Steering_Angle.Value;
    else
        raw_steer = zeros(size(raw_time));
        warning('[Telemetry] Steering_Angle channel not found. Using zeros.');
    end

    % Trim pre-race idle: find first sample with throttle activity
    start_idx = find(raw_accel > 0.01, 1, 'first');
    if isempty(start_idx)
        start_idx = 1;
        warning('[Telemetry] No throttle input detected. Loading full dataset.');
    else
        fprintf('[Telemetry] Pre-race idle trimmed. First active sample at t = %.2f s\n', ...
                raw_time(start_idx));
    end

    % Slice all channels from first active sample and FORCE AS COLUMN VECTORS
    % L'operatore (:) assicura che siano tutti vettori verticali Nx1
    Time_vec  = raw_time(start_idx:end);     Time_vec  = Time_vec(:);
    Accel_vec = raw_accel(start_idx:end);    Accel_vec = Accel_vec(:);
    Brake_vec = raw_brake(start_idx:end);    Brake_vec = Brake_vec(:);
    Steer_vec = raw_steer(start_idx:end);    Steer_vec = Steer_vec(:);

    % Normalize time to start at t = 0
    Time_vec = Time_vec - Time_vec(1);
    Lap_Time_Total = Time_vec(end);

    fprintf('[Telemetry] Simulated duration: %.3f s\n', Lap_Time_Total);

    % Create Simulink-ready matrices directly (Time in col 1, Data in col 2)
    SimInput_Accel = [Time_vec, Accel_vec];
    SimInput_Brake = [Time_vec, Brake_vec];
    SimInput_Steer = [Time_vec, Steer_vec];

catch ME
    error('[Telemetry] Channel extraction failed. Verify .mat file structure.\n%s', ME.message);
end

% -------------------------------------------------------------------------
% 3. Export to base workspace
% -------------------------------------------------------------------------
% Esportazione dei vettori singoli
assignin('base', 'Time_vec',  Time_vec);
assignin('base', 'Accel_vec', Accel_vec);
assignin('base', 'Brake_vec', Brake_vec);
assignin('base', 'Steer_vec', Steer_vec);

% Esportazione delle matrici GIÀ PRONTE per il blocco "From Workspace"
assignin('base', 'SimInput_Accel', SimInput_Accel);
assignin('base', 'SimInput_Brake', SimInput_Brake);
assignin('base', 'SimInput_Steer', SimInput_Steer);
assignin('base', 'Lap_Time_Total', Lap_Time_Total);

fprintf('[Telemetry] All channels loaded. Simulink matrices ready.\n');