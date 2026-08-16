function [steer_deg, s0, idx_out, Xt, Yt] = pure_pursuit(X, Y, psi, Vx, trackX, trackY, s_ref, idx_in)
    L = 1.55; steer_ratio = 4.2;
    Ld0 = 3.0; k_ld = 0.3;
    N = length(trackX);
    win = 60;
    Ld = Ld0 + k_ld*max(Vx,0);

    offsets = -win:win;                        % dimensione fissa (121), nota a compile-time
    idx_range = mod(idx_in + offsets - 1, N) + 1;  % ora la size è determinabile

    d2 = (trackX(idx_range)-X).^2 + (trackY(idx_range)-Y).^2;
    [~,k] = min(d2);
    idx_near = idx_range(k);

    circ_dist = mod(idx_near - idx_in + N/2, N) - N/2;
    if abs(circ_dist) > 300
        idx_near = idx_in;
    end

    idx_la = idx_near; dist = 0;
    while dist < Ld
        idx_next = mod(idx_la, N) + 1;
        dist = dist + hypot(trackX(idx_next)-trackX(idx_la), trackY(idx_next)-trackY(idx_la));
        idx_la = idx_next;
        if idx_la == idx_near, break; end
    end

    dx = trackX(idx_la)-X; dy = trackY(idx_la)-Y;
    alpha = atan2(dy,dx) - psi;
    alpha = atan2(sin(alpha), cos(alpha));

    delta_road = atan2(2*L*sin(alpha), Ld);
    steer_deg  = delta_road * (180/pi);

    idx_out = idx_near;
    s0 = s_ref(idx_near);
    Xt = trackX(idx_la); Yt = trackY(idx_la);
end