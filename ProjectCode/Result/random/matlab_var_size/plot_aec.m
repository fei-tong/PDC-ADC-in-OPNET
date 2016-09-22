% Plot AEC: average energy consumption per node per second, fixed: xi=14, var traffic load=0.4,0.45,0.5. var size: 250,300,350,400,450
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_4=[0.059672518	0.059585395	0.059538528	0.059500717	0.059452526];
basic_45=[0.059709535	0.059614349	0.059557938	0.059518731	0.059466794];
basic_5=[0.059749843	0.059637851	0.059577093	0.059533769	0.059476525];
asm_4=[0.059790771	0.059636851	0.059591896	0.059540468	0.059504495];
asm_45=[0.059907974	0.059706163	0.059634786	0.059584897	0.059555728];
asm_5=[0.059993804	0.059769643	0.059679215	0.059623878	0.05958605];

figure;
hold off;
network_size=250:50:450;
plot(network_size,basic_4,'ro-.');
hold on;
plot(network_size,basic_45,'r>-.');
plot(network_size,basic_5,'rx-.');


plot(network_size,asm_4,'bo-');
plot(network_size,asm_45,'b>-');
plot(network_size,asm_5,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
%axis([0.05 0.5 0.012 0.026]);
xlabel('Network Size');
ylabel('Average Power Consumption (watt)');
legend('PRI-basic, 0.4','PRI-basic, 0.45','PRI-basic, 0.5', ...
    'PRI-ASM, 0.4','PRI-ASM, 0.45','PRI-ASM, 0.5',1);
% grid on;