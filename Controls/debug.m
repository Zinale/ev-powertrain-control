
% Salva il modello
save_system('peacockElettricaSimulink')
% 
% loops = Simulink.BlockDiagram.getAlgebraicLoops('peacockElettricaSimulink');
% 
% % Converti gli handle in nomi leggibili
% for i = 1:numel(loops)
%     fprintf('\n=== Loop %d (IsArtificial: %d) ===\n', i, loops(i).IsArtificial);
%     handles = loops(i).BlockHandles;
%     for j = 1:numel(handles)
%         fprintf('  Blocco %d: %s\n', j, getfullname(handles(j)));
%     end
% end