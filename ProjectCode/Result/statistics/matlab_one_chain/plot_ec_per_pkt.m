% Plot network energy consumption per packet, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[0.000885133	0.000493923	0.000363828	0.000299035	0.000270975	0.000270417	0.000270154	0.000269922	0.000269741	0.000269584;
0.000862079	0.000488153	0.000359278	0.000308227	0.000307344	0.000307023	0.00030679	0.000306587	0.000306422	0.000306268;
0.000858102	0.000479316	0.000358865	0.000344422	0.000343961	0.000343661	0.000343451	0.000343259	0.00034311	0.000343];

asm=[0.000884371	0.00049491	0.000373515	0.000305755	0.000267903	0.000242353	0.000224341	0.000211507	0.000201399	0.00019285;
0.000869765	0.000491636	0.000366482	0.000301402	0.000265899	0.000240207	0.00022224	0.000209721	0.000199788	0.000192106;
0.000854622	0.000483851	0.00036322	0.000301019	0.00026356	0.000239271	0.00022274	0.000209014	0.000199612	0.000191437];


figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,basic(1,:),'ro-.');
hold on;
plot(arrival_rate,basic(2,:),'r>-.');
plot(arrival_rate,basic(3,:),'rx-.');


plot(arrival_rate,asm(1,:),'bo-');
plot(arrival_rate,asm(2,:),'b>-');
plot(arrival_rate,asm(3,:),'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
% axis([0.05 0.5 20 250]);
xlabel('Traffic Load (packet/s)');
ylabel('Power Consumption per Packet (watt)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 18}','PRI-basic, {{\it{\xi}} = 22}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 18}','PRI-ASM, {{\it{\xi}} = 22}',1);
% grid on;