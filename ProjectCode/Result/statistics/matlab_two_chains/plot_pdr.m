% Plot PDR: packet delivery ratio, from 1hop - 10 hops, \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic=[1	1	0.756142466	0.566441292	0.451378521	0.378720291	0.3245129	0.285055796	0.252059417	0.22585389;
1	0.915677498	0.609031987	0.454419641	0.366126362	0.306097466	0.260946022	0.228439345	0.202921872	0.183354266;
1	0.765102919	0.51210318	0.385428219	0.304331729	0.257623838	0.219023853	0.193362859	0.172054017	0.153945681
    ];

asm=[1	1	1	1	1	1	1	0.999513972	0.968447691	0.8839804;
1	1	1	1	1	1	1	0.998550512	0.957714004	0.884118014;
1	1	1	1	1	1	0.999773373	0.99616125	0.955281572	0.884929322
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