function [Acc,dt_s,v_s,t_s] = Straight(g,aX_Lim,L_s,p,V_0)
% Funzione per il calcolo della velocità, accelerazione e tempo di
% percorrenza su un tratto rettilineo partedno da una data V in base alla
% lunghezza del tratto facendo riferimento ai dati calcolati in GGV_Diagram

v_s(1,1) = V_0;
dt_s(1,1) = 0;
t_s(1,1) = 0;
Acc(1,1) = aX_Lim(fix(v_s(1,1))*10)*g;

dt_s(2,1) = (-v_s(1,1) + ((v_s(1,1)^2)-2*Acc(1,1)*(-p))^(1/2))/Acc(1,1);              
v_s(2,1) = v_s(1,1) + Acc(1,1) * dt_s(2,1);
t_s (2,1) = t_s(1,1) + dt_s(2,1);
Acc(2,1) = aX_Lim(fix(v_s(2,1))*10)*g;

for i=3:L_s/p
    Acc(i,1) = aX_Lim(fix(v_s(i-1,1))*10)*g;
    dt_s(i,1) = (-v_s(i-1,1) + ((v_s(i-1,1)^2)-2*Acc(i,1)*(-p))^(1/2))/Acc(i,1);              
    v_s(i,1) = v_s(i-1,1) + Acc(i,1) * dt_s(i,1);
    t_s(i,1) = t_s(i-1,1) + dt_s(i,1);
    if v_s(i,1) > 29
       break
    end
end

    for j=i:L_s/p
        Acc(j,1) = Acc(i);
        dt_s(j,1) = p/v_s(j-1);              
        v_s(j,1) = v_s(j-1);
        t_s(j,1) = t_s(j-1) + dt_s(j);
    end

end
