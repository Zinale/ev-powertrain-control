clc
close all

CAR_P5_outboard_Nuovo_datiaggiornati
GGV_Diagram_Nuovo_datiaggiornati
   
       % Lunghezza rettilinei tracciato (m)
L_s = [35; 30; 40; 50; 60; 50; 30; 65; 40; 40; 40; 25];    
L_start = L_s(1);
L_finish = L_s(end);

        % Lunghezza e raggi curvatura curve tracciato (m)
L_t = [70; 20; 20; 40; 30; 30; 20; 20; 30; 40; 70; 30];
R_t = [80; 15; 15; 40; 10; 28; 12; 12; 15; 26; 70; 18];
L_slaloom = 10;
R_slaloom = 12;
L_start_slaloom = L_slaloom/2;
Cones = 6;
L_slaloom_tot = 2*(L_start_slaloom) + (Cones)*L_slaloom;

    % Lunghezza totale traccaito
L_track = sum(L_s) + sum(L_t) + L_slaloom_tot;

    % Distanza fra i punti di analisi (m)
p = 0.25;

    % Numero punti di analisi lungo il tracciato
n = fix(L_track/p);

    % Posizione auto nel tracciato
    cp = zeros(fix(L_track/p),1);
for x = 1:fix(L_track/p)
    cp(x,1) = p*x;
end

    % Plot traccaito
    Track = imread('TestAutoX.png');
    imshow(Track);

%% FORWARD SIMULATION
[Acc_start,dt_start,v_start,t_start] = StartStraight(g,aX_Lim_acc,L_start,p);

[V_start_slaloom,T_start_slaloom,dt_start_slaloom,v_start_slaloom,t_start_slaloom,V_slaloom,T_slaloom,dt_slaloom,v_slaloom,t_slaloom] = Slaloom(L_start_slaloom,L_slaloom,p,V_lim(1,1));

[Acc_2,dt_s2,v_s2,t_s2] = Straight(g,aX_Lim_acc,L_s(2),p,V_lim(1,1));

V_lim(2,1)=v_s2(end);
[V_t1,T_t1,dt_t1,v_t1,t_t1,Acc_t1] = Turn(g,aY_max,aX_Lim_acc,L_t(1),p,V_lim(2,1),v_s2(end),1);

V_lim(2,1) = v_t1(end); 
[Acc_3,dt_s3,v_s3,t_s3] = Straight(g,aX_Lim_acc,L_s(3),p,V_lim(2,1));

[V_t2,T_t2,dt_t2,v_t2,t_t2,Acc_t2] = Turn(g,aY_max,aX_Lim_acc,L_t(2),p,V_lim(3,1),v_s3(end),2);
[V_t3,T_t3,dt_t3,v_t3,t_t3,Acc_t3] = Turn(g,aY_max,aX_Lim_acc,L_t(3),p,V_lim(4,1),v_t2(end),3);

[Acc_4,dt_s4,v_s4,t_s4] = Straight(g,aX_Lim_acc,L_s(4),p,V_lim(4,1));

[V_t4,T_t4,dt_t4,v_t4,t_t4,Acc_t4] = Turn(g,aY_max,aX_Lim_acc,L_t(4),p,V_lim(5,1),v_s4(end),4);

[Acc_5,dt_s5,v_s5,t_s5] = Straight(g,aX_Lim_acc,L_s(5),p,V_lim(5,1));

[V_t5,T_t5,dt_t5,v_t5,t_t5,Acc_t5] = Turn(g,aY_max,aX_Lim_acc,L_t(5),p,V_lim(6,1),v_s5(end),5);

[Acc_6,dt_s6,v_s6,t_s6] = Straight(g,aX_Lim_acc,L_s(6),p,V_lim(6,1));

[V_t6,T_t6,dt_t6,v_t6,t_t6,Acc_t6] = Turn(g,aY_max,aX_Lim_acc,L_t(6),p,V_lim(7,1),v_s6(end),6);

[Acc_7,dt_s7,v_s7,t_s7] = Straight(g,aX_Lim_acc,L_s(7),p,V_lim(7,1));

[V_t7,T_t7,dt_t7,v_t7,t_t7,Acc_t7] = Turn(g,aY_max,aX_Lim_acc,L_t(7),p,V_lim(8,1),v_s7(end),7);
[V_t8,T_t8,dt_t8,v_t8,t_t8,Acc_t8] = Turn(g,aY_max,aX_Lim_acc,L_t(8),p,V_lim(9,1),v_t7(end),8);

[Acc_8,dt_s8,v_s8,t_s8] = Straight(g,aX_Lim_acc,L_s(8),p,V_lim(9,1));

[V_t9,T_t9,dt_t9,v_t9,t_t9,Acc_t9] = Turn(g,aY_max,aX_Lim_acc,L_t(9),p,V_lim(10,1),v_s8(end),9);

[Acc_9,dt_s9,v_s9,t_s9] = Straight(g,aX_Lim_acc,L_s(9),p,V_lim(10,1));

[V_t10,T_t10,dt_t10,v_t10,t_t10,Acc_t10] = Turn(g,aY_max,aX_Lim_acc,L_t(10),p,V_lim(11,1),v_s9(end),10);

[Acc_10,dt_s10,v_s10,t_s10] = Straight(g,aX_Lim_acc,L_s(10),p,V_lim(11,1));

V_lim(12,1)=v_s10(end);
[V_t11,T_t11,dt_t11,v_t11,t_t11,Acc_t11] = Turn(g,aY_max,aX_Lim_acc,L_t(11),p,V_lim(12,1),v_s10(end),11);

V_lim(12,1) = v_t11(end);
[Acc_11,dt_s11,v_s11,t_s11] = Straight(g,aX_Lim_acc,L_s(11),p,V_lim(12,1));

[V_t12,T_t12,dt_t12,v_t12,t_t12,Acc_t12] = Turn(g,aY_max,aX_Lim_acc,L_t(12),p,V_lim(13,1),v_s11(end),12);

[Acc_finish,dt_finish,v_finish,t_finish] = Straight(g,aX_Lim_acc,L_finish,p,V_lim(13,1));

V_fw = [v_start; v_start_slaloom; v_slaloom; v_slaloom; v_slaloom; v_slaloom; v_slaloom; v_slaloom; v_start_slaloom; 
             v_s2; v_t1; v_s3; v_t2; v_t3; v_s4; v_t4; v_s5; v_t5; v_s6; v_t6; v_s7; v_t7; v_t8; v_s8; v_t9; v_s9; v_t10; v_s10; v_t11; v_s11; v_t12; v_finish];

Distance_fw = 1:3992;
% figure;plot(Distance_fw*p,V_fw*3.6,'g--'); hold on

%% BACKWARDS SIMULATION

[Acc_finish_rw,dt_finish_rw,v_finish_rw,t_finish_rw] = Straight(g,aX_Lim_brake,L_finish,p,V_t12);

[V_t12_rw,T_t12_rw,dt_t12_rw,v_t12_rw,t_t12_rw,Acc_t12_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(12),p,V_lim(13,1),V_lim(13,1),12);

[Acc_11_rw,dt_s11_rw,v_s11_rw,t_s11_rw] = Straight(g,aX_Lim_brake,L_s(11),p,v_t12_rw(end));

[V_t11_rw,T_t11_rw,dt_t11_rw,v_t11_rw,t_t11_rw,Acc_t11_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(11),p,V_lim(12,1),V_lim(12,1),11);

[Acc_10_rw,dt_s10_rw,v_s10_rw,t_s10_rw] = Straight(g,aX_Lim_brake,L_s(10),p,v_t11_rw(end));

[V_t10_rw,T_t10_rw,dt_t10_rw,v_t10_rw,t_t10_rw,Acc_t10_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(10),p,V_lim(11,1),V_lim(11,1),10);

[Acc_9_rw,dt_s9_rw,v_s9_rw,t_s9_rw] = Straight(g,aX_Lim_brake,L_s(9),p,v_t10_rw(end));

[V_t9_rw,T_t9_rw,dt_t9_rw,v_t9_rw,t_t9_rw,Acc_t9_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(9),p,V_lim(10,1),V_lim(10,1),9);

[Acc_8_rw,dt_s8_rw,v_s8_rw,t_s8_rw] = Straight(g,aX_Lim_brake,L_s(8),p,v_t9_rw(end));

[V_t8_rw,T_t8_rw,dt_t8_rw,v_t8_rw,t_t8_rw,Acc_t8_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(8),p,V_lim(9,1),V_lim(9,1),8);
[V_t7_rw,T_t7_rw,dt_t7_rw,v_t7_rw,t_t7_rw,Acc_t7_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(7),p,V_lim(8,1),V_lim(8,1),7);

[Acc_7_rw,dt_s7_rw,v_s7_rw,t_s7_rw] = Straight(g,aX_Lim_brake,L_s(7),p,v_t7_rw(end));

[V_t6_rw,T_t6_rw,dt_t6_rw,v_t6_rw,t_t6_rw,Acc_t6_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(6),p,V_lim(7,1),V_lim(7,1),6);

[Acc_6_rw,dt_s6_rw,v_s6_rw,t_s6_rw] = Straight(g,aX_Lim_brake,L_s(6),p,v_t6_rw(end));

[V_t5_rw,T_t5_rw,dt_t5_rw,v_t5_rw,t_t5_rw,Acc_t5_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(5),p,V_lim(6,1),V_lim(6,1),5);

[Acc_5_rw,dt_s5_rw,v_s5_rw,t_s5_rw] = Straight(g,aX_Lim_brake,L_s(5),p,v_t5_rw(5,1));

[V_t4_rw,T_t4_rw,dt_t4_rw,v_t4_rw,t_t4_rw,Acc_t4_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(4),p,V_lim(5,1),V_lim(5,1),4);

[Acc_4_rw,dt_s4_rw,v_s4_rw,t_s4_rw] = Straight(g,aX_Lim_brake,L_s(4),p,v_t4_rw(4,1));

[V_t3_rw,T_t3_rw,dt_t3_rw,v_t3_rw,t_t3_rw,Acc_t3_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(3),p,V_lim(4,1),V_lim(4,1),3);
[V_t2_rw,T_t2_rw,dt_t2_rw,v_t2_rw,t_t2_rw,Acc_t2_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(2),p,V_lim(3,1),V_lim(3,1),2);

[Acc_3_rw,dt_s3_rw,v_s3_rw,t_s3_rw] = Straight(g,aX_Lim_brake,L_s(3),p,v_t2_rw(end));

[V_t1_rw,T_t1_rw,dt_t1_rw,v_t1_rw,t_t1_rw,Acc_t1_rw] = Turn(g,aY_max,aX_Lim_brake,L_t(1),p,V_lim(2,1),V_lim(2,1),1);

[Acc_2_rw,dt_s2_rw,v_s2_rw,t_s2_rw] = Straight(g,aX_Lim_brake,L_s(2),p,v_t1_rw(end));

[V_start_slaloom_rw,T_start_slaloom_rw,dt_start_slaloom_rw,v_start_slaloom_rw,t_start_slaloom_rw,V_slaloom_rw,T_slaloom_rw,dt_slaloom_rw,v_slaloom_rw,t_slaloom_rw] = Slaloom(L_start_slaloom,L_slaloom,p,V_lim(1,1));

[Acc_start_rw,dt_start_rw,v_start_rw,t_start_rw] = Straight(g,aX_Lim_brake,L_start,p,v_start_slaloom_rw(end));
% INVERTIRE VETTORI
V_rw = [flipud(v_start_rw); flipud(v_start_slaloom_rw); flipud(v_slaloom_rw); flipud(v_slaloom_rw); flipud(v_slaloom_rw); flipud(v_slaloom_rw); flipud(v_slaloom_rw);
        flipud(v_slaloom_rw); flipud(v_start_slaloom_rw); flipud(v_s2_rw); flipud(v_t1_rw); flipud(v_s3_rw); flipud(v_t2_rw); flipud(v_t3_rw); flipud(v_s4_rw); flipud(v_t4_rw); 
        flipud(v_s5_rw); flipud(v_t5_rw); flipud(v_s6_rw); flipud(v_t6_rw); flipud(v_s7_rw); flipud(v_t7_rw); flipud(v_t8_rw); flipud(v_s8_rw); flipud(v_t9_rw); flipud(v_s9_rw); 
        flipud(v_t10_rw); flipud(v_s10_rw); flipud(v_t11_rw); flipud(v_s11_rw); flipud(v_t12_rw); flipud(v_finish_rw)];


% plot(Distance_fw*p,V_rw*3.6,'r--'); hold on
% title 'Distanza percorsa .vs. Velocità'

%% RISULTATO FINALE

% VELOCITA'
for i=1:fix(L_track/p)
    V_tot(i,1)= min(V_fw(i,1),V_rw(i,1));
end

for i=fix(L_track/p)-L_finish/p-1:fix(L_track/p)
    V_tot(i,1)= V_fw(i,1);
end



Distance = 1:3980;
figure;
yyaxis left
plot(Distance*p,V_tot*3.6,'b',LineWidth=2); hold on
title 'Distance .vs. Speed'
    xlabel('Distance [m]')
    ylabel('Speed [km/h]')
    %legend ('V_F_o_r','V_R_e_a_r','V_R_e_a_l', 'location','southeast')
%         plot([35,35],[0,150],'k'); hold on  % StartSlm BP
%         plot([135,135],[0,150],'k'); hold on  % T1 
%         plot([245,245],[0,150],'k'); hold on  % T2 BP
%         plot([265,265],[0,150],'k'); hold on  % T3
%         plot([335,335],[0,150],'k'); hold on  % T4 BP
%         plot([435,435],[0,150],'k'); hold on  % T5 BP
%         plot([515,515],[0,150],'k'); hold on  % T6 BP
%         plot([575,575],[0,150],'k'); hold on  % T7 BP
%         plot([595,595],[0,150],'k'); hold on  % T8
%         plot([680,680],[0,150],'k'); hold on  % T9 BP
%         plot([750,750],[0,150],'k'); hold on  % T10 BP
%         plot([830,830],[0,150],'k'); hold on  % T11
%         plot([940,940],[0,150],'k'); hold on  % T12 BP
    axis([0 L_track 0 150])

%_________________________________________________________________________%

% PUNTI STACCATA

v_BP_StartSlm = max(V_tot(1:35/p,1));
BP_StartSlm = find(V_tot(1:35/p,1) == v_BP_StartSlm);

v_BP_2 = max(V_tot(206/p:245/p,1));
BP_2 = find(V_tot(206/p:245/p,1) == v_BP_2);
BP_2 = BP_2(end);

v_BP_4 = max(V_tot(285/p:335/p));
BP_4 = find(V_tot(285/p:335/p) == v_BP_4);

v_BP_5 = max(V_tot(375/p:435/p));
BP_5 = find(V_tot(375/p:435/p) == v_BP_5);
BP_5 = BP_5(end);

v_BP_6 = max(V_tot(465/p:515/p));
BP_6 = find(V_tot(465/p:515/p) == v_BP_6);

v_BP_7 = max(V_tot(545/p:575/p));
BP_7 = find(V_tot(545/p:575/p) == v_BP_7);

v_BP_9 = max(V_tot(615/p:680/p));
BP_9 = find(V_tot(615/p:680/p) == v_BP_9);

v_BP_10 = max(V_tot(710/p:750/p));
BP_10 = find(V_tot(710/p:750/p) == v_BP_10);

v_BP_12 = max(V_tot(905/p:940/p));
BP_12 = find(V_tot(905/p:940/p) == v_BP_12);
BP_12 = BP_12(end);

% PLOT BRAKE POINT
%           plot([BP_StartSlm*p,BP_StartSlm*p],[0,150],'k'); hold on  % StartSlm BP
%           plot([206+BP_2*p,206+BP_2*p],[0,150],'k'); hold on  % T2 BP
%           plot([285+BP_4*p,285+BP_4*p],[0,150],'k'); hold on  % T4 BP
%           plot([375+BP_5*p,375+BP_5*p],[0,150],'k'); hold on  % T5 BP
%           plot([465+BP_6*p,465+BP_6*p],[0,150],'k'); hold on  % T6 BP
%           plot([545+BP_7*p,545+BP_7*p],[0,150],'k'); hold on  % T7 BP
%           plot([615+BP_9*p,615+BP_9*p],[0,150],'k'); hold on  % T9 BP
%           plot([710+BP_10*p,710+BP_10*p],[0,150],'k'); hold on  % T10 BP
%           plot([905+BP_12*p,905+BP_12*p],[0,150],'k'); hold on  % T12 BP

%_________________________________________________________________________%

% CALCOLO TEMPO e ACCELERAZIONI

    % S_Start
for i=1 : BP_StartSlm
    t_TOT(i,1) = dt_start(i,1);
    a_long(i,1) = Acc_start(i,1);
    a_lat(i,1) = 0;
end
    %Brake_Slm
for i=BP_StartSlm+1 : L_start/p
    t_TOT(i,1) = dt_start_rw(L_start/p+1-i,1);
    a_long(i,1) = -Acc_start_rw(L_start/p+1-i,1);   
    a_lat(i,1) = 0;
end
    % Start_Slm
for i=L_start/p+1 : L_start/p+ L_start_slaloom/p
    t_TOT(i,1) = dt_start_slaloom(1,1);
    a_long(i,1) = 0;
    a_lat(i,1) = -aY_Lim(fix(V_start_slaloom*10),2);
end
    % Slaloom
for i=L_start/p+L_start_slaloom/p+1 : L_start/p+L_start_slaloom/p+1+ L_slaloom/p*6
    t_TOT(i,1) = dt_slaloom(1,1);
    a_long(i,1) = 0;
    a_lat(i,1) = -aY_Lim(fix(V_slaloom*10),2);
end
    % Start_Slm
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+  L_start_slaloom/p
    t_TOT(i,1) = dt_start_slaloom(1,1);
    a_long(i,1) = 0;
    a_lat(i,1) = dt_start_slaloom(1,1);
end
    % S_2
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+ L_s(2)/p 
    t_TOT(i,1) = dt_s2(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+ L_s(2)/p+1-i,1);
    a_long(i,1) = Acc_2(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p),1);
    a_lat(i,1) = 0;
end
    % T_1
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+ L_t(1)/p 
    t_TOT(i,1) = dt_t1(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+ L_t(1)/p+2-i,1);
    a_long(i,1) = 0;
    a_lat(i,1) = aY_Lim(fix(V_t1*10),2);
end
    % S_3
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+ BP_2  
    t_TOT(i,1) = dt_s3(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+BP_2+1-i,1);
    a_long(i,1) = Acc_3(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p-1),1);
    a_lat(i,1) = 0;
end
    % Brake_T2
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+BP_2+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+ L_s(3)/p 
    t_TOT(i,1) = dt_s3_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+1-i,1);
    a_long(i,1) = -Acc_3_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+1-i,1);
    a_lat(i,1) = 0;
end
    % T_2
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+ L_s(3)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+ L_s(3)/p+ L_t(2)/p
    t_TOT(i,1) = dt_t2(1,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = -aY_Lim(fix(V_t2*10),2);
end
    % T_3
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+ L_t(3)/p 
    t_TOT(i,1) = dt_t3(1,1);
    a_long(i,1) = 0;
    a_lat(i,1) = aY_Lim(fix(V_t3*10),2);
end
    % S_4
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ BP_4  
    t_TOT(i,1) = dt_s4(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+BP_4+1-i,1);
    a_long(i,1) = Acc_4(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p-1),1);    
    a_lat(i,1) = 0;
end
    % Brake_T4
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+BP_4+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p
    t_TOT(i,1) = dt_s4_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+1-i,1);
    a_long(i,1) = -Acc_4_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+1-i,1);     
    a_lat(i,1) = 0;
end
    % T_4
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+ L_t(4)/p 
    t_TOT(i,1) = dt_t4(1,1);
    a_long(i,1) = 0;
    a_lat(i,1) = aY_Lim(fix(V_t4*10),2);
end
    % S_5
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+L_t(4)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+ BP_5
    t_TOT(i,1) = dt_s5(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+L_t(4)/p+ BP_5+1-i,1);
    a_long(i,1) = Acc_5(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+L_t(4)/p-1),1); 
    a_lat(i,1) = 0;
end
    % Brake_T5
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+BP_5+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+ L_s(5)/p
    t_TOT(i,1) = dt_s5_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+L_t(4)/p+ L_s(5)/p+1-i,1);
    a_long(i,1) = -Acc_5_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+ L_s(4)/p+L_t(4)/p+ L_s(5)/p+1-i,1); 
    a_lat(i,1) = 0;
end
    % T_5
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+ L_t(5)/p
    t_TOT(i,1) = dt_t5(1,1);
    a_long(i,1) = 0;
    a_lat(i,1) = aY_Lim(fix(V_t5*10),2);
end    
    % S_6
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+ BP_6 
    t_TOT(i,1) = dt_s6(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+BP_6+1-i,1);
    a_long(i,1) = Acc_6(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p-1),1); 
    a_lat(i,1) = 0;
end
    % Brake_T6
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+BP_6+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+ L_s(6)/p
    t_TOT(i,1) = dt_s6_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+1-i,1);
    a_long(i,1) = -Acc_6_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+1-i,1); 
    a_lat(i,1) = 0;
end
    % T_6
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+ L_s(6)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+ L_t(6)/p
    t_TOT(i,1) = dt_t6(1,1);
    a_long(i,1) = 0;  
   a_lat(i,1) = -aY_Lim(fix(V_t6*10),2);
end 
    % S_7
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+ BP_7 
    t_TOT(i,1) = dt_s7(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+ BP_7+1-i,1);
    a_long(i,1) = Acc_7(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p-1),1);     
    a_lat(i,1) = 0;
end
    % Brake_T7
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+ BP_7+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+ L_s(7)/p
    t_TOT(i,1) = dt_s7_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+ L_s(7)/p+1-i,1);
    a_long(i,1) = -Acc_7_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+ L_s(7)/p+1-i,1);     
    a_lat(i,1) = 0;
end
    % T_7
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+ L_t(7)/p
    t_TOT(i,1) = dt_t7(1,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = -aY_Lim(fix(V_t7*10),2);
end 
    % T_8
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+ L_t(8)/p 
    t_TOT(i,1) = dt_t8(1,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = aY_Lim(fix(V_t8*10),2);
end 
    % S_8
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+ BP_9  
    t_TOT(i,1) = dt_s8(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+ BP_9+1-i,1);
    a_long(i,1) = Acc_8(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p-1),1);     
    a_lat(i,1) = 0;
end
    % Brake_T9
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+BP_9+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+ L_s(8)/p
    t_TOT(i,1) = dt_s8_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+1-i,1);
    a_long(i,1) = -Acc_8_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+1-i,1);     
    a_lat(i,1) = 0;
end
    % T_9
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+ L_t(9)/p 
    t_TOT(i,1) = dt_t9(1,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = -aY_Lim(fix(V_t9*10),2);
end 
    % S_9
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+ L_t(9)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ BP_10 
    t_TOT(i,1) = dt_s9(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ BP_10+1-i,1);
    a_long(i,1) = Acc_9(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+ L_t(9)/p-1),1);     
    a_lat(i,1) = 0;
end
    % Brake_T10
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+BP_10+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ L_s(9)/p
    t_TOT(i,1) = dt_s9_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ L_s(9)/p+1-i,1);
    a_long(i,1) = -Acc_9_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ L_s(9)/p+1-i,1);     
    a_lat(i,1) = 0;
end
    % T_10
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ L_s(9)/p+ L_t(10)/p
    t_TOT(i,1) = dt_t10(1,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = aY_Lim(fix(V_t10*10),2);
end 
    % S_10
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ L_s(9)/p+L_t(10)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+ L_s(10)/p 
    t_TOT(i,1) = dt_s10(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+1-i,1);
    a_long(i,1) = Acc_10(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+ L_s(9)/p+L_t(10)/p),1);     
    a_lat(i,1) = 0;
end
    % T_11
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+ L_t(11)/p
    t_TOT(i,1) = dt_t11(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+ L_t(11)/p+1-i,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = aY_Lim(fix(V_t11*10),2);
end 
    % S_11
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+ BP_12 
    t_TOT(i,1) = dt_s11(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+BP_12+1-i,1);
    a_long(i,1) = Acc_11(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p-1),1);     
    a_lat(i,1) = 0;
end
    % Brake_T12    
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+BP_12+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+ L_s(11)/p
    t_TOT(i,1) = dt_s11_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+ L_s(11)/p+1-i,1);
    a_long(i,1) = -Acc_11_rw(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+ L_s(11)/p+1-i,1);   
    a_lat(i,1)= 0;
end
    % T_12   
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+L_s(11)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+L_s(11)/p+ L_t(12)/p
    t_TOT(i,1) = dt_t12(1,1);
    a_long(i,1) = 0;    
    a_lat(i,1) = aY_Lim(fix(V_t12*10),2);
end 
    % S_finish
for i=L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+L_s(11)/p+L_t(12)/p+1 : L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+L_s(11)/p+L_t(12)/p+ L_finish/p  
    t_TOT(i,1) = dt_finish(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+L_s(11)/p+L_t(12)/p+ L_finish/p+1-i,1);
    a_long(i,1) = Acc_finish(i-(L_start/p+L_start_slaloom/p+1+L_slaloom/p*6+L_start_slaloom/p+L_s(2)/p+L_t(1)/p+L_s(3)/p+L_t(2)/p+L_t(3)/p+L_s(4)/p+L_t(4)/p+L_s(5)/p+L_t(5)/p+L_s(6)/p+L_t(6)/p+L_s(7)/p+L_t(7)/p+L_t(8)/p+L_s(8)/p+L_t(9)/p+L_s(9)/p+L_t(10)/p+L_s(10)/p+L_t(11)/p+L_s(11)/p+L_t(12)/p),1);     
    a_lat(i,1) = 0;
end

T = sum(t_TOT);
disp (T);

V_max = max(V_tot)*3.6;     %  km/h
disp (V_max);

Distance_acc = 1:L_track/p+1;

%figure;
yyaxis right
plot(Distance_acc*p,a_long/g,LineWidth=1)
title 'Distance .vs. Longitudinal Acceleration'
    xlabel('Distance [m]')
    ylabel('Longitudinale Acceleration [g]')
    axis([0 L_track -2.0 1.5])

figure; plot(Distance_acc*p,a_lat,LineWidth=1)
title 'Distance .vs. Laterla Acceleration'
    xlabel('Distance [m]')
    ylabel('Lateral Acceleration [g]')
     axis([0 L_track -1.5 1.5])














