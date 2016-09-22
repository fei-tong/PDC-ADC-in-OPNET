% Plot network energy consumption per packet
% \xi = 14, 17, 20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[0.01663689	0.01116339	0.008439779	0.006733317	0.00631437	0.006348341	0.006363669	0.006366117	0.006393311;
    0.016039745	0.010704243	0.008237164	0.00767684	0.007684242	0.007689084	0.007689344	0.007758875	0.00776837;
    0.015635387	0.010636542	0.00896994	0.008982426	0.00903552	0.009071758	0.009105051	0.009166195	0.00912727
    ];
asm=[0.016706353	0.011228519	0.008385282	0.006701777	0.005554028	0.004857434	0.004391293	0.004239499	0.004184893;
    0.015884792	0.010873158	0.008059396	0.006464183	0.005500637	0.00519708	0.00503382	0.005037291	0.004943984;
    0.015630694	0.010722585	0.007931246	0.006507041	0.005984787	0.00588148	0.005866524	0.005846166	0.005913956
    ];

figure;
hold off;
arrival_rate=0.1:0.05:0.5;
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