% Plot Packet delivery latency, fixed arrival rate=0.2, \xi=14,17,20
% \xi = 14, arrival rate=0.1,0.2,0.3
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_1=[1.441813617	1.574864347	1.707836778	1.840806569	1.973834653	2.106769413	2.239682566	2.372844425	2.505825497	2.638808467];
asm_1=[1.200435184	1.510693897	1.695841347	1.845947978	1.984578358	2.119595153	2.25359427	2.38694057	2.520201479	2.653127839];
basic_2=[1.982018264	2.115112872	2.248082786	2.381097796	2.514142679	2.647061009	2.779974243	2.913075327	3.046091964	3.179062948];
asm_2=[1.238417471	1.711433634	1.997606214	2.211254439	2.390151126	2.549089849	2.696265852	2.837502575	2.9759642	3.108935528];
basic_3=[3.102769126	3.235743886	3.368727288	3.501717207	3.634744033	3.767680214	3.900707225	4.033724157	4.166771059	4.29974591];
asm_3=[1.27827809	1.905017603	2.343035289	2.684518243	2.969266801	3.21665643	3.438510376	3.641066636	3.830253325	3.963287422];

figure;
hold off;
chain_hops=1:1:10;
plot(chain_hops,basic_1,'ro-.');
hold on;
plot(chain_hops,basic_2,'r>-.');
plot(chain_hops,basic_3,'rx-.');


plot(chain_hops,asm_1,'bo-');
plot(chain_hops,asm_2,'b>-');
plot(chain_hops,asm_3,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
% axis([1 9 2 500]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend('PRI-basic, 0.1','PRI-basic, 0.2','PRI-basic, 0.3', ...
    'PRI-ASM, 0.1','PRI-ASM, 0.2','PRI-ASM, 0.3',4);
% grid on;