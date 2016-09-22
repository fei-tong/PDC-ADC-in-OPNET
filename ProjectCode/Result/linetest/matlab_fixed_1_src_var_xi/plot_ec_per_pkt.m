% Plot network energy consumption per packet, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[6.372955286	3.556244695	2.61956295	2.153053631	1.951546903	1.94931255	1.949175317	1.948992844	1.948913954	1.948945939];
asm_14=[6.397764781	3.597914471	2.665798663	2.209491647	1.941947823	1.774218106	1.650453423	1.566118592	1.488242033	1.430985227];
basic_17=[6.306506593	3.516735795	2.598447374	2.164450074	2.149081384	2.14853964	2.148463179	2.148441875	2.148433493	2.14833131];
asm_17=[6.437783502	3.548528281	2.629649467	2.193255021	1.939552683	1.77234618	1.648902434	1.553376147	1.486103758	1.416607471];
basic_20=[6.207374421	3.478268271	2.572530114	2.34901563	2.34856752	2.348416209	2.348349856	2.348373527	2.348466307	2.348372744];
asm_20=[6.088482572	3.483433751	2.639824333	2.202399236	1.953193774	1.796396036	1.672289997	1.575303125	1.49391539	1.432172934];

figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,basic_14./7200,'ro-.');
hold on;
plot(arrival_rate,basic_17./7200,'r>-.');
plot(arrival_rate,basic_20./7200,'rx-.');


plot(arrival_rate,asm_14./7200,'bo-');
plot(arrival_rate,asm_17./7200,'b>-');
plot(arrival_rate,asm_20./7200,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
% axis([0.05 0.5 20 250]);
xlabel('Traffic Load (packet/s)');
ylabel('Power Dissipation Ratio (watt/packet)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',1);
% grid on;