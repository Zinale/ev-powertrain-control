%% 
%% 
clc
close all

%% SELEZIONE MODELLO AUTO e PNEUMATICI

CAR_P5_outboard_Nuovo_datiaggiornati
TIRE_Hoosier_18x6x8R25B;

%% CALCOLO FORZE AERODINAMICHE TOTALI

v = [0:1:300];                                               % velocità effettiva dell'auto (m/s) 
v_rif = 16;                                                  % velocità di riferimento per calcolo cl e cd (m/s)

FW_cl = FW_df/(0.5*rho*(v_rif)^2*FW_area);                   % cl ALA ANTERIORE
FW_cd = FW_dr/(0.5*rho*(v_rif)^2*FW_area);                   % cd ALA ANTERIORE
FW_down = 0.5*rho*(v/10).^2*FW_area*FW_cl;                   % Downforce ALA ANTERIORE
FW_drag = 0.5*rho*(v/10).^2*FW_area*FW_cd;                   % Drag ALA ANTERIORE

% P_cl = P_df/(0.5*rho*(v_rif)^2*P_area);                      % cl PANCE
% P_cd = P_dr/(0.5*rho*(v_rif)^2*P_area);                      % cd PANCE
% P_down = 0.5*rho*(v/10).^2*P_area*P_cl;                      % Downforce PANCE
% P_drag = 0.5*rho*(v/10).^2*P_area*P_cl;                      % Drag PANCE
% 
% RW_cl = RW_df/(0.5*rho*(v_rif)^2*RW_area);                   % cl ALA POSTERIORE
% RW_cd = RW_dr/(0.5*rho*(v_rif)^2*RW_area);                   % cd ALA POSTERIORE
% RW_down = 0.5*rho*(v/10).^2*RW_area*RW_cl;                   % Downforce ALA POSTERIORE
% RW_drag = 0.5*rho*(v/10).^2*RW_area*RW_cd;                   % Drag ALA POSTERIORE

FLR_cl = FLR_df/(0.5*rho*(v_rif)^2*FLR_area);                % cl FONDO
FLR_cd = FLR_dr/(0.5*rho*(v_rif)^2*FLR_area);                % cd FONDO
FLR_down = 0.5*rho*(v/10).^2*FLR_area*FLR_cl;                % Downforce FONDO
FLR_drag = 0.5*rho*(v/10).^2*FLR_area*FLR_cd;                % Drag FONDO

Vv_cl = Vv_df/(0.5*rho*(v_rif)^2*Vv_area);                   % cl VEICOLO SENZA AERODINAMICA
Vv_cd = Vv_dr/(0.5*rho*(v_rif)^2*Vv_area);                   % cd VEICOLO SENZA AERODINAMICA
Vv_down = 0.5*rho*(v/10).^2*Vv_area*Vv_cl;                   % Downforce VEICOLO SENZA AERODINAMICA
Vv_drag = 0.5*rho*(v/10).^2*Vv_area*Vv_cd;                   % Drag VEICOLO SENZA AERODINAMICA

FLR_front = ((xCP_FLR+xCoG_RA)/l)*FLR_down;
FW_front = ((xCP_Fw+xCoG_RA)/l)*FW_down;
% P_front = ((xCP_P+xCoG_RA)/l)*P_down;                 %___________downforce che agiscono sull'asse anteriore
Vv_front = ((xCP_Vv+xCoG_RA)/l)*Vv_down;

% TOT_cl = TOT_df/(0.5*rho*(v_rif)^2*TOT_area);                % cl TOT VEICOLO
% TOT_cd = TOT_dr/(0.5*rho*(v_rif)^2*TOT_area);                % cd TOT VEICOLO
% L = 0.5*rho*(v/10).^2*TOT_area*TOT_cl;                       % Dowforce TOT VEICOLO 
% D = 0.5*rho*(v/10).^2*TOT_area*TOT_cd;                       % Drag TOT VEICOLO

DFront = FLR_front+Vv_front+FW_front;                  %___________downforce totale sull'asse anteriore

FLR_rear = FLR_down-FLR_front;
FW_rear = FW_down-FW_front;
% P_rear = P_down-P_front;                              %___________downforce che agiscono sull'asse posteriore
Vv_rear = Vv_down-Vv_front;

DRear = FLR_rear+Vv_rear+FW_rear;                      %___________downforce totale sull'asse posteriore

   

%% TROVO VALORE LIMITE DI aX IN FRENATA

    % Ripartizione forza frenante
X_front = 0.77;
X_rear = 0.23;

    % SL limite
SL_front_lim = 0.9;                                                                              % Valori limite di SL                 
SL_rear_lim = 0.9;

for v = 0:300 
    % Il valore di aX va fatto variare fra -0.1g e -2.0g
    aX = (1:3000)/1000;
    % Carichi verticali sugli assi
W_front(:,v+1) = m_TOT * g * (b/l) + DFront(1,v+1)  - DW_long * (-aX);
W_rear(:,v+1) = m_TOT * g *(a/l) + DRear(1,v+1) + DW_long * (-aX);
%W_long_tot = W_front + W_rear(aX);

    % Forze longitudinali richieste per asse
Fx_tot_dem(:,v+1) = m_TOT * (-aX) * g; 
Fx_front_axis_dem(:,v+1) = Fx_tot_dem(:,v+1) * X_front;                                % Calcolo forze longitudinali richieste per asse   
Fx_rear_axis_dem(:,v+1) = Fx_tot_dem(:,v+1) * X_rear;

    % Forze longitudinali richieste per ruota
Fx_front_dem(:,v+1) = Fx_front_axis_dem(:,v+1)/2;                                              % Si assume che il carico sulla ruota sx sia uguale a quello sulla dx               
Fx_rear_dem(:,v+1) = Fx_rear_axis_dem(:,v+1)/2;

    % Valore Kx
Kx_front(:,v+1) = interp1  (Fz_long,Kx,W_front(:,v+1)/2);                                      % Entro nella curva e tiro fuori il valore per il W front
Kx_rear(:,v+1) = interp1  (Fz_long,Kx,W_rear(:,v+1)/2);

    % SL limite normalizzati
SL_n_front(:,v+1) = Kx_front(:,v+1) * SL_front_lim ./ (u_x * W_front(:,v+1)/2);
SL_n_rear(:,v+1) = Kx_rear(:,v+1) * SL_rear_lim ./ (u_x * W_rear(:,v+1)/2);

     % Froza longitudinale disponibile normalizzata
Fx_n_front_av(:,v+1) = D_long*sin(C_long*atan(B_long*(1-E_long)*SL_n_front(:,v+1)+E_long*atan(B_long*SL_n_front(:,v+1))));   % Calcolo forze longitudinali normalizzate disponibili per ruota   
Fx_n_rear_av(:,v+1) = D_long*sin(C_long*atan(B_long*(1-E_long)*SL_n_rear(:,v+1)+E_long*atan(B_long*SL_n_rear(:,v+1))));      % usando magic formula

     % Forze longitudinali effettivamente disponibili
Fx_front_av(:,v+1) =  Fx_n_front_av(:,v+1) .* u_x .* W_front(:,v+1)/2;                                            
Fx_rear_av(:,v+1) =   Fx_n_rear_av(:,v+1) .* u_x .* W_rear(:,v+1)/2; 
Fx_tot_av(:,v+1) = 2* Fx_front_av(:,v+1) + 2* Fx_rear_av(:,v+1);
Delta_Fx(:,v+1) = Fx_tot_av(:,v+1)+Fx_tot_dem(:,v+1);
 
     % Individuo accelerazioni limite esaminando la differenza Fx_av-Fx_dem
 Delta_logic_long(:,v+1) = Delta_Fx(:,v+1)<0;
    
 aX_Lim_brake(v+1) = find(Delta_logic_long(:,v+1),1)/1000;

end

%  GRAFICO CHE COMPARA LA FORZA DISPONIBILE CON QUELLA RICHIESTA AL VARIARE
%   DELL'ACCELERAZIONE LIMITE
% aX = 1:2100;
% figure;plot(-aX/1000,Fx_tot_av(:,101),'b'); hold on;
%        plot(-aX/1000,-Fx_tot_dem(:,101),'r'); hold on;
%        plot(-aX/1000,Delta_Fx(:,101),'g--');
%        title ('Fx available .vs. Fx demanded @ 10 m/s');
%        xlabel('aX (g)');
%        ylabel('Fx (N)');
%        legend ('Fx_a_v','Fx_d_e_m','Delta_F_x','southeast')

% GRAFICO VARIAZIONE aX_Lim IN FRENATA IN FUNZIONE DELLA VELOCITA'
%  v = 0:300;
%  figure; plot (v/10,aX_Lim_brake);hold on;
%          title ('Longitudinal Acceleration .vs. Speed');
%          xlabel('v (m/s)');
%          ylabel('aX lim (g)');
 
%% TROVO LIMITE DI aX IN ACCELERAZIONE

%    Limite dato dalla coppia massima dei motori
    R_ruota = D_ruota/2/1000;                             % Raggio ruota (m)

v=0:300;
    w_ruota = v/10/R_ruota;
    w_mot = w_ruota*tau;
    rpm_mot = w_mot*60/(2*pig);
    C_max = interp1(Rpm_E,C_E,rpm_mot)*tau;                   % Coppia massima offerta da un singolo motore (Nm)
    Fx_max = C_max/R_ruota;                          % Forza longitudinale massima offerta da un singolo motore (N)
    aX_Lim_acc = (2 * Fx_max - (FW_drag+Vv_drag+FLR_drag))/m_TOT/g;      % Calcolo accelerazione massima possibile in base alla coppia massima erogabile (g)


% VERIFICA ADERENZA GOMMME
Delta_Fx_acc = Fx_front_av(aX_Lim_brake(1)*1000,1)-Fx_max;


% GRAFICO VARIAZIONE aX_Lim IN ACCELERAZIONE IN FUNZIONE DELLA VELOCITA'
%  v = 0:300;
%   plot (v/10,aX_Lim_acc); hold on;
%           title ('Longitudinal Acceleration .vs. Speed');
%           xlabel('v (m/s)');
%           ylabel('aX lim (g)');


 %% TROVO VALORE LIMITE DI aY
     % Il valore limite va calcolato per la curva con R minimo
     % All curva con R minimo corrisponde l'accelerazione laterale massima
                 % R sarà uguale alla componente minima del vettore contenente tutti i raggi di curvatura del tracciato
 
 % Raggio di curvatura percorribile
 R = [12 ; 80 ; 15; 15; 40; 10; 28; 12; 12; 15; 26; 70; 18];           % Possibilità di fare ci clo con r che varia fra i valori dei raggi di curvatura
 N_turn = length(R);

 for i=1:N_turn
     for v=0:300
 
    % MODELLO a BICICLETTA
     % Calcolo carichi verticali su assi
        W_front_by(v+1,i) = m_TOT * g * (b/l) + DFront(v+1);
        W_rear_by(v+1,i) = m_TOT * g * (a/l) + DRear(v+1); 
         
     % Accelerazione laterale
        aY_R(v+1,i) =(v/10)^2/R(i,1)/g;

      % Forze laterali richieste
        Fy_front_by(v+1,i) = m_TOT * aY_R(v+1,i) * g * (b/l);                                                  % Calcolo forze laterali richieste   
        Fy_rear_by(v+1,i) = m_TOT * aY_R(v+1,i) * g * (a/l);
        Fy_tot_dem(v+1,i) = Fy_front_by(v+1,i) + Fy_rear_by(v+1,i);

    % Fy normalizzata
        Fy_n_front_by(v+1,i) = Fy_front_by(v+1,i)/(u_y * W_front_by(v+1,i));                                 % Normalizzo le forze laterali richieste   
        Fy_n_rear_by(v+1,i) = Fy_rear_by(v+1,i)/(u_y * W_rear_by(v+1,i)); 

        Fy_n_front_by(Fy_n_front_by>1) = 0.9987;         % Limite forza laterale                       
        Fy_n_rear_by(Fy_n_rear_by>1) = 0.9987;

    % Slip angle normalizzati
        SA_n_front_by(v+1,i) = interp1  (Fy_norm,SA_norm,Fy_n_front_by(v+1,i));                              % Entro nella curva e tiro fuori il valore di SA per il Fy
        SA_n_rear_by(v+1,i) = interp1  (Fy_norm,SA_norm,Fy_n_rear_by(v+1,i));
    
    % Valore Cornering Stiffness
        C_alpha_front_by(v+1,i) = interp1  (Fz_lat,C_alpha,W_front_by(v+1,i));                               % Entro nella curva e tiro fuori il valore per il W front
        C_alpha_rear_by(v+1,i) = interp1  (Fz_lat,C_alpha,W_rear_by(v+1,i));

    % Slip angle richiesti
        SA_front_by(v+1,i) = atan((SA_n_front_by(v+1,i) * u_y * W_front_by(v+1,i))/C_alpha_front_by(v+1,i));      % Ricavo i valori effettivi di SA dai valori normalizzati trovati
        SA_rear_by(v+1,i) = atan((SA_n_rear_by(v+1,i) * u_y * W_rear_by(v+1,i))/C_alpha_rear_by(v+1,i));

        v_med(v+1,i) = v/10;

    % Yaw Rate
        r(v+1,i) = v_med(v+1,i)/R(i,1);

    % Angolo di sterzo
        delta(v+1,i) = l/1000/R(i,1) - SA_front_by(v+1,i) + SA_rear_by(v+1,i);    

    % Vehicle Side Slip 
        Beta(v+1,i) = 0.5 * (SA_front_by(v+1,i) - a/1000*r(v+1,i)/v_med(v+1,i) + delta(v+1,i) + SA_rear_by(v+1,i) + b/1000*r(v+1,i)/v_med(v+1,i));

    % Velocità laterale
        v_lat(v+1,i) = v_med(v+1,i)*sin(Beta(v+1,i));

   % MODELLO a 4 RUOTE    
    % Slip angle delle singole ruote        
        SA_front_in(v+1,i) = - delta(v+1,i) + atan((v_lat(v+1,i)+r(v+1,i)*a/1000)/(v_med(v+1,i)-r(v+1,i)*t/1000/2));
        SA_front_out(v+1,i) = - delta(v+1,i) + atan((v_lat(v+1,i)+r(v+1,i)*a/1000)/(v_med(v+1,i)+r(v+1,i)*t/1000/2));
        SA_rear_in(v+1,i) = atan((v_lat(v+1,i)-r(v+1,i)*b/1000)/(v_med(v+1,i)-r(v+1,i)*t/1000/2));
        SA_rear_out(v+1,i) = atan((v_lat(v+1,i)-r(v+1,i)*b/1000)/(v_med(v+1,i)+r(v+1,i)*t/1000/2));

    % Carichi verticali sulle 4 ruote
        W_front_in_ver(v+1,i) = m_TOT * g *(b/l)/2 + DFront(v+1)/2 - DW_front * aY_R(v+1,i);                         
        W_front_out_ver(v+1,i) = m_TOT * g *(b/l)/2 + DFront(v+1)/2 + DW_front * aY_R(v+1,i);                        
        W_rear_in_ver(v+1,i) = m_TOT * g *(a/l)/2 + DRear(v+1)/2 - DW_rear * aY_R(v+1,i);                              
        W_rear_out_ver(v+1,i) = m_TOT * g *(a/l)/2 + DRear(v+1)/2 + DW_rear * aY_R(v+1,i);                          

    % Cornening Stiffnes delle 4 ruote
        C_alpha_front_in(v+1,i) = interp1  (Fz_lat,C_alpha,W_front_in_ver(v+1,i));  
        C_alpha_front_out(v+1,i) = interp1  (Fz_lat,C_alpha,W_front_out_ver(v+1,i)); 
        C_alpha_rear_in(v+1,i) = interp1  (Fz_lat,C_alpha,W_rear_in_ver(v+1,i));
        C_alpha_rear_out(v+1,i) = interp1  (Fz_lat,C_alpha,W_rear_out_ver(v+1,i));

    % Slipl angle normalizzati
        SA_front_in_norm(v+1,i) = C_alpha_front_in(v+1,i) * tan(SA_front_in(v+1,i))/(u_y * W_front_in_ver(v+1,i));
        SA_front_out_norm(v+1,i) = C_alpha_front_out(v+1,i) * tan(SA_front_out(v+1,i))/(u_y * W_front_out_ver(v+1,i));
        SA_rear_in_norm(v+1,i) = C_alpha_rear_in(v+1,i) * tan(SA_rear_in(v+1,i))/(u_y * W_rear_in_ver(v+1,i));
        SA_rear_out_norm(v+1,i) = C_alpha_rear_out(v+1,i) * tan(SA_rear_out(v+1,i))/(u_y * W_rear_out_ver(v+1,i));
        
        SA_front_in_norm(301,i) = 2.51;
        SA_front_out_norm(301,i) = 2.51;
        SA_rear_in_norm(301,i) = 2.51;
        SA_rear_out_norm(301,i) = 2.51;

    % Impongo condizione SA_norm > - 3
        SA_front_in_logic(v+1,i) = SA_front_in_norm(v+1,i) > 2.50;                                      % Individuo tutti i valori di SA norm superiori a 2.50, per quei valori sono in slittamento                  
        SA_front_out_logic(v+1,i) = SA_front_out_norm(v+1,i) > 2.50;
        SA_rear_in_logic(v+1,i) = SA_rear_in_norm(v+1,i) > 2.50;
        SA_rear_out_logic(v+1,i) = SA_rear_out_norm(v+1,i) > 2.50;
     end 
    SA_front_out_logic(295,i) = 1;
    SA_rear_out_logic(295,i) = 1;
    SA_rear_in_logic(295,i) = 1;

    V_front_in_lim(i,1) = (find(SA_front_in_logic(50:301,i),1)+50)/10;
    V_front_out_lim(i,1) = (find(SA_front_out_logic(50:301,i),1)+50)/10;
    V_rear_in_lim(i,1) = (find(SA_rear_in_logic(50:301,i),1)+50)/10;
    V_rear_out_lim(i,1) = (find(SA_rear_out_logic(50:301,i),1)+50)/10;
    
    V_4_lim(i,:) = [V_front_in_lim(i,1)  V_front_out_lim(i,1) V_rear_in_lim(i,1)  V_rear_out_lim(i,1)];
    %V_4_lim(2,1) = 17;
    V_lim(i,1) = min(V_4_lim(i,:));      % Calcolo le velocità limite corrispondenti ai valori di SA limite
    aY_max(i,1) =(V_lim(i,1))^2/R(i,1)/g;
 end
    

 % Trovati i valori di SA limite e le relative velocità verifico di avere abbastanza Fy per rimanere in aderenza e soddisfare l'equilibrio    
for i=1:N_turn
    aY_Lim (1:V_lim(i,1)*10+1,i) = aY_R(1:V_lim(i,1)*10+1,i);
    for v=0:V_lim(i,1)*10-1

        aY = aY_Lim(v+1,i);

     % Carichi verticali sulle ruote
        W_front_in_ver(v+1,i) = m_TOT * g *(b/l)/2 + DFront(v+1)/2 - DW_front * aY; 
        W_front_out_ver(v+1,i) = m_TOT * g *(b/l)/2 + DFront(v+1)/2 + DW_front * aY; 
        W_rear_in_ver(v+1,i) = m_TOT * g *(a/l)/2 + DRear(v+1)/2 - DW_rear * aY; 
        W_rear_out_ver(v+1,i) = m_TOT * g *(a/l)/2 + DRear(v+1)/2 + DW_rear * aY; 
 
     % Valore Cornering Stiffness
        C_alpha_front_in_ver(v+1,i) = interp1  (Fz_lat,C_alpha,W_front_in_ver(v+1,i));                                 % Entro nella curva e tiro fuori il valore per il W front
        C_alpha_front_out_ver(v+1,i) = interp1  (Fz_lat,C_alpha,W_front_out_ver(v+1,i)); 
        C_alpha_rear_in_ver(v+1,i) = interp1  (Fz_lat,C_alpha,W_rear_in_ver(v+1,i));
        C_alpha_rear_out_ver(v+1,i) = interp1  (Fz_lat,C_alpha,W_rear_out_ver(v+1,i));
 
    % Slip Angle
        SA_front_in_ver(v+1,i) = SA_front_in(v+1,i);
        SA_front_out_ver(v+1,i) = SA_front_out(v+1,i);
        SA_rear_in_ver(v+1,i) = SA_rear_in(v+1,i);
        SA_rear_out_ver(v+1,i) = SA_rear_out(v+1,i);

     % Slip angle normalizzati
        SA_n_front_in_ver(v+1,i) = C_alpha_front_in_ver(v+1,i) * tan(SA_front_in_ver(v+1,i))/(u_y * W_front_in_ver(v+1,i));
        SA_n_front_out_ver(v+1,i) = C_alpha_front_out_ver(v+1,i) * tan(SA_front_out_ver(v+1,i))/(u_y * W_front_out_ver(v+1,i));
        SA_n_rear_in_ver(v+1,i) = C_alpha_rear_in_ver(v+1,i) * tan(SA_rear_in_ver(v+1,i))/(u_y * W_rear_in_ver(v+1,i));
        SA_n_rear_out_ver(v+1,i) = C_alpha_rear_out_ver(v+1,i) * tan(SA_rear_out_ver(v+1,i))/(u_y * W_rear_out_ver(v+1,i));
 
    % Calcolo Fy normalizzata con Magic Formula
        Fy_n_front_in_magic(v+1,i) = D_lat*sin(C_lat*atan(B_lat*(1-E_lat)*SA_n_front_in_ver(v+1,i)+E_lat*atan(B_lat*SA_n_front_in_ver(v+1,i))));
        Fy_n_front_out_magic(v+1,i) = D_lat*sin(C_lat*atan(B_lat*(1-E_lat)*SA_n_front_out_ver(v+1,i)+E_lat*atan(B_lat*SA_n_front_out_ver(v+1,i))));
        Fy_n_rear_in_magic(v+1,i) = D_lat*sin(C_lat*atan(B_lat*(1-E_lat)*SA_n_rear_in_ver(v+1,i)+E_lat*atan(B_lat*SA_n_rear_in_ver(v+1,i))));
        Fy_n_rear_out_magic(v+1,i) = D_lat*sin(C_lat*atan(B_lat*(1-E_lat)*SA_n_rear_out_ver(v+1,i)+E_lat*atan(B_lat*SA_n_rear_out_ver(v+1,i))));

     % Forze laterali disponibili
        Fy_front_in_av_ver(v+1,i) = Fy_n_front_in_magic(v+1,i) * u_y * W_front_in_ver(v+1,i); 
        Fy_front_out_av_ver(v+1,i) = Fy_n_front_out_magic(v+1,i) * u_y * W_front_out_ver(v+1,i); 
        Fy_rear_in_av_ver(v+1,i) = Fy_n_rear_in_magic(v+1,i) * u_y * W_rear_in_ver(v+1,i); 
        Fy_rear_out_av_ver(v+1,i) = Fy_n_rear_out_magic(v+1,i) * u_y * W_rear_out_ver(v+1,i); 
 
        Fy_tot_av_ver(v+1,i) = Fy_front_in_av_ver(v+1,i) + Fy_front_out_av_ver(v+1,i) + Fy_rear_in_av_ver(v+1,i) + Fy_rear_out_av_ver(v+1,i);
 
     % Delta Forze laterali         
        Delta_Fy_ver(v+1,i) = Fy_tot_av_ver(v+1,i)-Fy_tot_dem(v+1,i);
    end
end

%   GRAFICO CHE COMPARA LA FORZA DISPONIBILE CON QUELLA RICHIESTA AL VARIARE
%   DELLA VELOCITA'
%               v = 1:V_lim(1,1)*10;
%               figure; plot(v/10,Fy_tot_av_ver(1:V_lim(1,1)*10,1),'b'); hold on; 
%                       plot(v/10,Fy_tot_dem(1:V_lim(1,1)*10,1),'r'); hold on;                     
%                       plot(v/10,Delta_Fy_ver(1:V_lim(1,1)*10,1),'g--'); hold on;
%                       title ('Fy .vs. Speed @ R = 8.75')
%                       xlabel('Speed (m/s)');
%                       ylabel('Fy (N)');
%                       legend ('Fy available','Fy demanded','Delta','location','northwest')
%                       
%               v = 1:V_lim(2,1)*10;
%               figure; plot(v/10,Fy_tot_av_ver(1:V_lim(2,1)*10,2),'b'); hold on; 
%                       plot(v/10,Fy_tot_dem(1:V_lim(2,1)*10,2),'r'); hold on;
%                       plot(v/10,Delta_Fy_ver(1:V_lim(2,1)*10,2),'g--'); hold on;
%                       title ('Fy .vs. Speed @ R = 20')
%                       xlabel('Speed (m/s)');
%                       ylabel('Fy (N)');  
%                       legend ('Fy available','Fy demanded','Delta','location','northwest')

% GRAFICO VARIAZIONE aY_Lim IN FUNZIONE DELLA VELOCITA'
% v = 1:V_lim(1,1)*10;
% figure; plot (v/10,aY_Lim(1:V_lim(1,1)*10,1));hold on;
%          title ('Lateral Acceleration .vs. Speed');
%          xlabel('v (m/s)');
%          ylabel('aY lim (g)');
% v = 1:V_lim(2,1)*10;
%          plot (v/10,aY_Lim(1:V_lim(2,1)*10,2));hold on;
%          title ('Lateral Acceleration .vs. Speed');
%          xlabel('v (m/s)');
%          ylabel('aY lim (g)');
%          legend ('R = 8.75','R = 20','location','northwest')


% Verifica contatto pneumatico asfalto
% for i=1:N_turn
%     W_front_in_ver(V_lim(i,1)*10,i) = -1;
%     W_front_in_ver_logic(:,i) = W_front_in_ver(:,i) < 0;
%     V_lim(i,1) = (find(W_front_in_ver_logic(:,i),1)-5)/10;
% end

%% FORZE LIMITE PNEUMATICO

% for v = 0:300
%     Fx_lim_brake(v+1,1) = Fx_front_av (fix(aX_Lim_brake(v+1)*1000),v+1);
% end
% 
% %Fy_lim = zeros(V_lim(2,1)*10,2);
% Fy_lim = zeros (301,2);
% for i=1:2
%     for v =0:V_lim(i,1)*10
% 
%         Fy_lim(v+1,i) = Fy_front_out_av_ver(v+1,i);
%     end
% end

%% VALUTO SITUAZIONI COMBIANTE (METODO ANTONIO)

% step = 10;
% N_Rturn = 1;
% for v=0:300
%     aX_Lim_brake(v+1);
%     aX_Comb(v+1,:) = linspace(0,aX_Lim_brake(v+1),step);
%     Fx_comb_av(:,v+1) = [Fx_tot_av(1,v+1) ; Fx_tot_av(fix(aX_Comb(v+1,2:step)*1000),v+1)];
%     Fx_comb_dem(:,v+1) = [Fx_tot_dem(1,v+1) ; Fx_tot_dem(fix(aX_Comb(v+1,2:step)*1000),v+1)]; 
%     Delta_Fx_comb(:,v+1) = Fx_comb_av(:,v+1) + Fx_comb_dem(:,v+1);
%     Delta_Fx_comb(step,:) = 0;
%     
%     for i= 1:length(Delta_Fx_comb(:,1))
%         [val,vY_comb] = min(abs(Fy_tot_av_ver(:,N_Rturn)-Delta_Fx_comb(i,v+1)));
%         aY_Comb(v+1,i) = aY_Lim(vY_comb+1,N_Rturn);
%         aY_Comb(:,step) = 0;
%     end
%     
% end
% 
% Delta_Fx_comb_T = transpose(Delta_Fx_comb);
% aX_Comb_T = transpose(aX_Comb);
% aY_Comb_T = transpose(aY_Comb);
% 
% % v = linspace(0,300,step);
% % plot3(aY_Comb,aX_Comb,v);
% 
% 
%     % Definisco matrice velocità
% for n=1:301
%     for m=1:step
%         v(n,m)= n;
%     end
% end
% 
% figure;surf(aY_Comb,-aX_Comb,v/10); hold on
%        surf(-aY_Comb,-aX_Comb,v/10);
%       title ('GGV Diagram');
%       xlabel('aY (g)');
%       ylabel('aX (g)');
%       zlabel('v (m/s)');
%       
% 
% step = 10;
% for v=0:300
%     aX_Lim_acc(v+1);
%     aX_Comb_acc(v+1,:) = linspace(0,aX_Lim_acc(v+1),step);
%     Fx_comb_av(:,v+1) = [Fx_tot_av(1,v+1) ; Fx_tot_av(fix(aX_Comb_acc(v+1,2:step)*1000),v+1)];
%     Fx_dem(v+1) = Fx_max(v+1); 
%     Delta_Fx_comb(:,v+1) = Fx_comb_av(:,v+1) + Fx_dem(:,v+1);
%     Delta_Fx_comb(step,:) = 0;
% 
%     for i= 1:length(Delta_Fx_comb(:,1))
% 
%         [val,vY_comb_acc] = min(abs(Fy_tot_av_ver(:,N_Rturn)-Delta_Fx_comb(i,v+1)));
%         aY_Comb_acc(v+1,i) = aY_Lim(vY_comb_acc,N_Rturn);
%         aY_Comb_acc(:,step) = 0;
%     end
%  end
% 
%     % Definisco matrice velocità
% for n=1:301
%     for m=1:step
%         v(n,m)= n;
%     end
% end
% 
% surf(aY_Comb_acc,aX_Comb_acc,v/10);
% surf(-aY_Comb_acc,aX_Comb_acc,v/10);


%% PLOT CON SUDDIVISIONI
%     % Definisco matrice velocità
%     speed_step = 50;
%     V = zeros(speed_step,step);
%     aX_Comb_step = zeros(speed_step,step);
%     aY_Comb_step = zeros(speed_step,step);
%     aX_Comb_acc_step = zeros(speed_step,step);
%     aY_Comb_acc_step = zeros(speed_step,step);
% for n=0:speed_step-1
%     for m=1:10
%         V(n+1,m)= 5*n;
%     end
% end
% 
% for n=0:speed_step-1
%         aX_Comb_step(n+1,:) = [aX_Comb(1+n*fix(300/speed_step),:)];
%         aY_Comb_step(n+1,:) = [aY_Comb(1+n*fix(300/speed_step),:)];
%         aY_Comb_acc_step(n+1,:) = [aY_Comb_acc(1+n*fix(300/speed_step),:)];
%         aX_Comb_acc_step(n+1,:) = [aX_Comb_acc(1+n*fix(300/speed_step),:)];
% end
% 
% aX_Comb_step(speed_step,:) = [aX_Comb(301,:)];
% aY_Comb_step(speed_step,:) = [aY_Comb(301,:)];
% aX_Comb_acc_step(speed_step,:) = [aX_Comb_acc(301,:)];
% aY_Comb_acc_step(speed_step,:) = [aY_Comb_acc(301,:)];
% 
% figure;surf(aY_Comb_step,-aX_Comb_step,V); hold on
%       surf(-aY_Comb_step,-aX_Comb_step,V);
%       surf(aY_Comb_acc_step,aX_Comb_acc_step,V);
%       surf(-aY_Comb_acc_step,aX_Comb_acc_step,V);
%       title ('GGV Diagram');
%       xlabel('aY (g)');
%       ylabel('aX (g)');
%       zlabel('v (m/s)');
      



%% VALUTO SITUAZIONI COMBINATE (CURVA-FRENATA)

% for v=0:25
% 
%     P = fix(aX_Lim_brake(26)/0.1);
%     Q = fix(aY_Lim(26)/0.095);
% 
%     n(v+1) = aX_Lim_brake(v+1)/P;
%     m(v+1) = aY_Lim(v+1)/Q;
%     
%     for p=1:P
% 
%         aX_Comb(p,v+1) = fix(p * n(v+1)*1000);
%     end
% 
%         for q=1:Q
% 
%             aY_comb(q,v+1) = fix(q * m(v+1)*1000);
%         end
%             for i=1:P 
%                 for j=1:Q
%             Ftot_dem(i,j) = (Fx_front_dem(aX_Comb(i,v+1),v+1).^2 + Fy_front_out(aY_comb(j,v+1),v+1).^2).^(1/2);           
%             Delta_Ftot_dem(i,j) = Fy_lim(v+1) - Ftot_dem(i,j);                                                           
%             Delta_Logic_Ftot_dem (i,j) = Delta_Ftot_dem(i,j) < 0;
%             
% 
%                 end
%             end
% 
%                         for j=1:Q
%                             aX_Lim_Comb(j,v+1) = find(Delta_Logic_Ftot_dem (j,:),1)*n(v+1);
%                         end
%            
%                                             
%           
%         end
%            
%     % Definisco matrice velocità
% for n=1:15
%     for m=1:26
%         v(n,m)= m;
%     end
% end
%     % Definisco matrice accelerazioni laterali combinate
%  aY_Comb = aY_comb/1000;
% 
% 
% % GRAFICO FINALE GGV
% figure;surf(aY_Comb,-aX_Lim_Comb,v);
%             title ('GGV Diagram');
%             xlabel('aY (g)');
%             ylabel('aX (g)');
%             zlabel('v (m/s)');
% 
% 
% 
% i=1;
%     plot3(-aX_Lim_Comb(:,i),aY_comb(:,1)/1000,v(:,1),'b'), hold on;
%             xlabel('aX (g)');
%             ylabel('aY (g)');
%             zlabel('v (m/s)');
% 
% 
% %% VALUTO SITUAZIONI COMBINATE (CURVA-ACCELERAZIONE)            
% 
% for v=0:25
% 
%     P = fix(aX_Lim_acc(26)/0.1);
%     Q = fix(aY_Lim(26)/0.095);
% 
%     n(v+1) = aX_Lim_acc(v+1)/P;
%     m(v+1) = aY_Lim(v+1)/Q;
%     
%     for p=1:P
% 
%         aX_Comb(p,v+1) = fix(p * n(v+1)*1000);
%     end
% 
%         for q=1:Q
% 
%             aY_comb(q,v+1) = fix(q * m(v+1)*1000);
%         end
%             for i=1:P 
%                 for j=1:Q
%             Ftot_dem(i,j) = (Fx_front_dem(aX_Comb(i,v+1),v+1).^2 + Fy_front_out(aY_comb(j,v+1),v+1).^2).^(1/2);           
%             Delta_Ftot_dem(i,j) = Fy_lim(v+1) - Ftot_dem(i,j);                                                           
%             Delta_Logic_Ftot_dem (i,j) = Delta_Ftot_dem(i,j) < 0;
%             
% 
%                 end
%             end
% 
%                         for j=1:Q
%                             aX_Lim_Comb(j,v+1) = find(Delta_Logic_Ftot_dem (j,:),1)*n(v+1);
%                         end
%            
%                                             
%           
% end






%% VALUTO SITUAZIONI COMBINATE (METODO 2)

% anaizzo la sizuazione ad aX_MAX - 0.2g:
% Ricavo carichi verticali per aX(i) = aX_MAX - 0.2g e poi Fx

% Fx_Fi = entro nel grafico Fx vs SR per il valore di W calcolato
% Fx_Fe = entro nel grafico Fx vs SR per il valore di W calcolato
% Fx_Ri = entro nel grafico Fx vs SR per il valore di W calcolato
% Fx_Re = entro nel grafico Fx vs SR per il valore di W calcolato

% La Fy disponibile la ricavo dal cerchio di aderenza degli pneumatici
% F_tot = (Fx2+Fy2)1/2 --> ricavo Fy in quanto conosco Fx e F_tot 
% Fx l'ho appena calcolata
% F_tot calcolata all'inizio

% Ricavo quindi Fy possibile per ogni ruota e di conseguenza la aY associta,
% ho individuato i punto sul grafico
% Trovati tutti gli i-esimi punti fino ad aX= 0.2g (per aX=0 ho aY_MAX già
% calcolato), posso interpolare e trovo il limite di aderenza per quella
% velocità

% I PASSAGGI DI QUESTA SEZIONE VANNO ITINERATI PER TUTTE LE VELICITA'


