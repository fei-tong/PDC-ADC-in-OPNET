% Plot network energy consumption per packet, fixed: xi=14, var traffic load=0.4,0.45,0.5. var size: 250,300,350,400,450
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_4=[70.41774881	67.45241289	79.71085305	90.78244542	107.6039712];
basic_45=[70.72305123	67.37528325	81.03061446	91.17684174	111.4830646];
basic_5=[70.5314483	67.44815653	82.00277936	91.6987041	113.1266049];
asm_4=[38.38516761	45.91092507	53.62422415	60.91965561	68.83930906];
asm_45=[34.83729613	42.08711774	50.95667982	56.27355608	64.3957733];
asm_5=[33.23686122	39.68461318	50.94573544	53.49704308	65.55156819];

figure;
hold off;
network_size=250:50:450;
plot(network_size,basic_4./7200,'ro-.');
hold on;
plot(network_size,basic_45./7200,'r>-.');
plot(network_size,basic_5./7200,'rx-.');


plot(network_size,asm_4./7200,'bo-');
plot(network_size,asm_45./7200,'b>-');
plot(network_size,asm_5./7200,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
%axis([0.05 0.5 0.012 0.026]);
xlabel('Network Size');
ylabel('Power Dissipation Ratio (watt/packet)');
legend('PRI-basic, 0.4','PRI-basic, 0.45','PRI-basic, 0.5', ...
    'PRI-ASM, 0.4','PRI-ASM, 0.45','PRI-ASM, 0.5',2);
% grid on;