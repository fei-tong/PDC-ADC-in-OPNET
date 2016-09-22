% Plot PDR: packet delivery ratio, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[1	1	1	1	0.948447649	0.799006967	0.678938666	0.592277389	0.527607369	0.474975785;
1	1	1	0.946084926	0.76241455	0.637904623	0.545449143	0.481149597	0.428186709	0.384695499;
1	1	1	0.795644268	0.643391032	0.529328324	0.459079082	0.400218243	0.354610602	0.320329661
    ];

asm=[1	1	1	1	1	1	1	1	1	1;
1	1	1	1	1	1	1	1	1	1;
1	1	1	1	1	1	1	1	1	1
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

xlabel('Traffic Load (packet/s)');
ylabel('Packet Delivery Ratio');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 18}','PRI-basic, {{\it{\xi}} = 22}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 18}','PRI-ASM, {{\it{\xi}} = 22}',3);
% grid on;