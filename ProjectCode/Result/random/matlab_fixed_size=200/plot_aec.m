% Plot AEC: average energy consumption per node per second
% \xi = 14, 17, 20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[0.059346035	0.059412224	0.059475541	0.05954331	0.059623396	0.059714373	0.05976838	0.059840837	0.059886682	0.059925231];
asm_14=[0.059344657	0.059411567	0.059482344	0.059554112	0.05962858	0.059730056	0.059838747	0.059948733	0.060056192	0.060145912];
basic_17=[0.057880041	0.057942672	0.058013115	0.058091382	0.058173245	0.058235725	0.058298767	0.058335901	0.058374211	0.058412987];
asm_17=[0.057879552	0.05794847	0.058015492	0.058088156	0.058179607	0.058292509	0.058408555	0.058501975	0.058595515	0.058690294];
basic_20=[0.056813262	0.056883261	0.056947657	0.057032999	0.05709616	0.057163437	0.05720672	0.057239138	0.057286049	0.057319677];
asm_20=[0.056813651	0.05687916	0.056954235	0.057034433	0.05715322	0.05728183	0.057382415	0.057480786	0.057589587	0.057673829];

figure;
hold off;
arrival_rate=0.05:0.05:0.5;
plot(arrival_rate,basic_14,'ro-.');
hold on;
plot(arrival_rate,basic_17,'r>-.');
plot(arrival_rate,basic_20,'rx-.');


plot(arrival_rate,asm_14,'bo-');
plot(arrival_rate,asm_17,'b>-');
plot(arrival_rate,asm_20,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([0.05 0.5 0.054 0.061]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Power Consumption (watt)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',4);
% grid on;