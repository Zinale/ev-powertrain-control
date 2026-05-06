clc; clear; close all;

% 1. Caricamento Modello
modelName = "peacockElettricaSimulink";
load_system(modelName);

% 2. Importazione File .tir
% Assicurati che il percorso sia corretto
% Definisci il percorso del file .tir
tirePath = "tire_data/FSAE_Tire_Combined.tir";

% Importa correttamente il modello utilizzando il metodo .import
tm = tireModel.import(tirePath);

% 3. Esportazione per Simulink
% Invece di usare il comando 'set', estraiamo la struttura parametri
% Molti blocchi del Vehicle Dynamics Blockset leggono una struct
tireParameters = tm.getStruct(); 

% Se usi i blocchi "Extended Tire Features", a volte serve questo:
assignin('base', 'tireParameters', tireParameters);

disp("Parametri esportati con successo nel Workspace.");

% 4. Configurazione Blocco Simulink
% Sostituisci 'Combined Slip Wheel 2DOF' con il nome esatto del tuo blocco
blockPath = modelName + "/Combined Slip Wheel 2DOF";
set_param(blockPath, 'TireModel', 'Magic Formula');
% Se il blocco accetta una variabile esterna per i dati:
% set_param(blockPath, 'ParameterVariable', 'tireParameters');

% 5. Plotting (Mantenendo i tuoi calcoli manuali)
plot(tm); 
title('Validazione Modello Tir');