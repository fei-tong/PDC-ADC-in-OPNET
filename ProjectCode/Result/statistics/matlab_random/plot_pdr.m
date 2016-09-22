% Plot PDR: packet delivery ratio
% \xi = 14, 17, 20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[0.996803251	0.994326105	0.993206229	0.986878566	0.942927591	0.824486323	0.752539487	0.689609757	0.637600879;
0.995737248	0.992658415	0.987532139	0.919644887	0.796921366	0.728770415	0.666158694	0.607916705	0.560943874;
0.994211409	0.99053198	0.956890391	0.810806576	0.743680824	0.669644275	0.604670394	0.558223175	0.519503463
    ];
asm=[0.996501063	0.995619313	0.993359671	0.988486434	0.985314565	0.98193369	0.956533667	0.898966986	0.836319636;
0.995074412	0.992419644	0.9908352	0.985150589	0.980844755	0.92094265	0.848539476	0.757497541	0.71187161;
0.9943611	0.991421094	0.985195165	0.978755082	0.917991688	0.819246892	0.731287992	0.671916041	0.614055443
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

xlabel('Traffic Load (packet/s)');
ylabel('Packet Delivery Ratio');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 18}','PRI-basic, {{\it{\xi}} = 22}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 18}','PRI-ASM, {{\it{\xi}} = 22}',3);
% grid on;