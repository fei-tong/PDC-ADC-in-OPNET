% Plot yield: Network Yield; \xi = 14, 17, 20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[0.996660487	0.994229492	0.992427204	0.982399135	0.869452422	0.74170825	0.649774977	0.574754145	0.516245294;
    0.995036548	0.992277966	0.983761601	0.824733326	0.69806056	0.59317185	0.526319205	0.459980248	0.412628044;
    0.992870574	0.989299327	0.867685178	0.690804071	0.57853164	0.495949909	0.430727591	0.381730754	0.346416149
    ];
asm=[0.996067766	0.995330932	0.99222676	0.987538973	0.98221122	0.976471727	0.935122582	0.869922978	0.802117914;
    0.994667891	0.99194228	0.989786429	0.981800541	0.969075167	0.891073025	0.803732858	0.709142039	0.657624896;
    0.993528774	0.990376948	0.981668127	0.964497599	0.881192304	0.766064816	0.672235215	0.60202219	0.537649747
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
ylabel('Network Yield');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 18}','PRI-basic, {{\it{\xi}} = 22}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 18}','PRI-ASM, {{\it{\xi}} = 22}',3);
% grid on;