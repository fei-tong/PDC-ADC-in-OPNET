% Plot network energy consumption per packet, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[0.000875299	0.000495595	0.000457739	0.000457257	0.000458723	0.000456163	0.000456683	0.000455425	0.000454643	0.000455855;
    0.000869993	0.000536355	0.000536374	0.000536909	0.000534481	0.000533328	0.000533884	0.000533419	0.000533715	0.000533868;
    0.000858262	0.000614106	0.000613729	0.000613477	0.000611223	0.000611544	0.000612558	0.000610385	0.000609445	0.000610647
    ];

asm=[0.000889374	0.000503842	0.000370538	0.000307811	0.000269324	0.000243926	0.000226376	0.000213337	0.000206147	0.000205181;
    0.000883348	0.000489842	0.000365312	0.000303986	0.000266366	0.00024224	0.00022531	0.000212093	0.000205104	0.000203537;
    0.000854501	0.000486454	0.000362789	0.000300583	0.00026517	0.000241387	0.000223943	0.000211319	0.000205042	0.000202724
    ];


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