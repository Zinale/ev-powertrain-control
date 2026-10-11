function [Acc_start,dt_start,v_start,t_start] = StartStraight(g,aX_Lim,L_start,p)
% Funzione per il calcolo della velocità, accelerazione e tempo di
% percorrenza su un tratto rettilineo partedno da fermo in base alla
% lunghezza del tratto facendo riferimento ai dati calcolati in GGV_Diagram
v_start(1,1) = 0;
dt_start(1,1) = 0;
t_start(1,1) = 0;
Acc_start(1,1) = 0;

dt_start(2,1) = (2*p/(aX_Lim(1)*g))^(1/2);
v_start(2,1) =  aX_Lim(2)*g*dt_start(2,1);
t_start(2,1) = dt_start(1,1) + dt_start(2,1);
Acc_start(2,1) = aX_Lim(fix(v_start(2,1))*10)*g;

for i=3:L_start/p
    Acc_start(i,1) = aX_Lim(fix(v_start(i-1,1))*10)*g;
    dt_start(i,1) = (-v_start(i-1,1) + ((v_start(i-1,1)^2)-2*Acc_start(i,1)*(-p))^(1/2))/Acc_start(i,1);              
    v_start(i,1) = v_start(i-1,1) + Acc_start(i,1) * dt_start(i,1);
    t_start(i,1) = t_start(i-1,1) + dt_start(i,1);
end

end