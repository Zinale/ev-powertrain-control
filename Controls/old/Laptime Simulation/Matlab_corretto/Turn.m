function [V_t,T_t,dt_t,v_t,t_t,Acc] = Turn(g,aY_max,aX_Lim_acc,L_t,p,V_lim_turn,V_0,N_turn)
% Funzione per il calcolo della velocità limite in curva e tempo di
% percorrenza in base al raggio di curvatura e alla lunghezza della curva
% facendo riferimento ai dati calcolati in GGV_Diagram
V_t = V_lim_turn;
T_t = L_t/V_t;
dt_t = T_t/(L_t/p);

% se ad inzio curva non sono ancora alla velocità limite continuo ad
% accelerare durante la curva fin quando non mi porto a v lim
if V_0 < V_lim_turn
    v_t(1,1) = V_0;
    dt_t(1,1) = 0;
    t_t(1,1) = 0;
    Acc(1,1) = aX_Lim_acc(fix(v_t(1,1))*10)*g;

    dt_t(2,1) = (-v_t(1,1) + ((v_t(1,1)^2)-2*Acc(1,1)*(-p))^(1/2))/Acc(1,1);              
    v_t(2,1) = v_t(1,1) + Acc(1,1) * dt_t(2,1);
    t_t(2,1) = t_t(1,1) + dt_t(2,1);
    Acc(2,1) = aX_Lim_acc(fix(v_t(2,1))*10)*g;
% moto acclerato fin quando non arrivo alla v lim della curva
    for i=3:fix(L_t/p)+1
        Acc(i,1) = aX_Lim_acc(fix(v_t(i-1,1))*10)*g;
        dt_t(i,1) = (-v_t(i-1,1) + ((v_t(i-1,1)^2)-2*Acc(i,1)*(-p))^(1/2))/Acc(i,1);              
        v_t(i,1) = v_t(i-1,1) + Acc(i,1) * dt_t(i,1);
        t_t(i,1) = t_t(i-1,1) + dt_t(i,1);
        if v_t(i) > 29
            break
        end
    end
% curva a velocità costante uguale alla velocità limite
for j=i:fix(L_t/p)+1
    t_t(j,1) = j;
    v_t(j,1) = v_t(i);
    dt_t(j,1) = p/v_t(i); 
end
% caso in cui arrivo con velocità superiore alla velocità limite della
% curva
else 
    t_t = zeros(fix(L_t/p)+1,1);
    v_t = zeros(fix(L_t/p)+1,1);
    Acc = zeros(fix(L_t/p)+1,1);

    V_t = V_lim_turn;
    dt_t = T_t/(L_t/p);
    for i=1:fix(L_t/p)+1
        Acc(i,1) = aY_max(N_turn);
        t_t(i,1) = i;
        v_t(i,1) = V_t;
        dt_t(i,1) = p/V_t;
    end

end