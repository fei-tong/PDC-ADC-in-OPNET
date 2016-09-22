% Plot network energy consumption per packet, chain topology, from 1hop - 10 hops
% frxed \xi = 14, src=(1,2,3), traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_src_1=[6.372955286	3.556244695	2.61956295	2.153053631	1.951546903	1.94931255	1.949175317	1.948992844	1.948913954	1.948945939];
basic_src_2=[3.571672769	2.134052141	1.921970955	1.937354337	1.948868336	1.948843816	1.948925079	1.949005494	1.948951157	1.948866867];
basic_src_3=[2.559993903	1.892313826	1.920919449	1.937817523	1.94881195	1.948959677	1.948889642	1.948948804	1.948853428	1.948928291];
asm_src_1=[6.397764781	3.597914471	2.665798663	2.209491647	1.941947823	1.774218106	1.650453423	1.566118592	1.488242033	1.430985227];
asm_src_2=[3.584571571	2.160937269	1.728926868	1.515134987	1.383042654	1.286265699	1.217340986	1.156322642	1.112660909	1.09889978];
asm_src_3=[2.572795732	1.69677519	1.396835306	1.240895549	1.138758249	1.068227493	1.062358114	1.076197535	1.090524266	1.098427209];

figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,basic_src_1./7200,'ro-.');
hold on;
plot(arrival_rate,basic_src_2./7200,'r>-.');
plot(arrival_rate,basic_src_3./7200,'rx-.');


plot(arrival_rate,asm_src_1./7200,'bo-');
plot(arrival_rate,asm_src_2./7200,'b>-');
plot(arrival_rate,asm_src_3./7200,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')

%axis([0.05 0.5 0.012 0.026]);
xlabel('Traffic Load (packet/s)');
ylabel('Power Dissipation Ratio (watt/packet)');
legend('PRI-basic, 1 src','PRI-basic, 2 srcs','PRI-basic, 3 srcs', ...
    'PRI-ASM, 1 src','PRI-ASM, 2 srcs','PRI-ASM, 3 srcs',1);
% grid on;