function [V_start_slaloom,T_start_slaloom,dt_start_slaloom,v_start_slaloom,t_start_slaloom,V_slaloom,T_slaloom,dt_slaloom,v_slaloom,t_slaloom] = Slaloom(L_start_slaloom,L_slaloom,p,V_lim_turn)
% Funzione per il calcolo della velocità limite in uno slaloom e tempo di
% percorrenza in base al raggio di curvatura e alla lunghezza dello stesso
% facendo riferimento ai dati calcolati in GGV_Diagram
V_start_slaloom = V_lim_turn;
T_start_slaloom = L_start_slaloom/V_start_slaloom;
dt_start_slaloom = T_start_slaloom/(L_start_slaloom/p);

for i=1:fix(L_start_slaloom/p)
    t_start_slaloom(i,1) = i;
    v_start_slaloom(i,1) = V_start_slaloom;
end

    %SLALOOM
V_slaloom = V_lim_turn;
T_slaloom = L_slaloom/V_slaloom;
dt_slaloom = T_slaloom/(L_slaloom/p);

for i=1:fix(L_slaloom/p)
    t_slaloom(i,1) = i;
    v_slaloom(i,1) = V_slaloom;
end

end