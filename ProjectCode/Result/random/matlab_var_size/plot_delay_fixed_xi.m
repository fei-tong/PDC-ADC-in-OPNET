% Plot Packet delivery latency, fixed traffic load = 0.4, network size = 250,300,350,400,450
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_250=[5.229678038	346.4203414	670.2311687	1329.50375	1456.804621	1539.326474	1695.314676	1740.004175];
basic_300=[52.14458568	318.5905017	589.3188183	880.3723365	1137.538532	1323.033804	1290.740101	1471.069099];
basic_350=[38.69296032	149.044523	289.4661819	526.194892	708.6174378	1326.72729	1416.648724	1366.777956];
basic_400=[46.7362676	182.0283226	336.176631	620.3921965	975.3231875	1279.354892	1516.281412	1318.66893];
basic_450=[10.80544465	152.3471112	382.7220466	739.824926	1137.343922	1527.687135	1455.704231	1271.12436];
asm_250=[4.928937643	29.22707068	38.63511831	45.27251427	48.67002602	52.21584676	51.15870341	56.75448516];
asm_300=[6.226699785	30.33681736	38.39156879	49.2879723	54.34667484	66.59997483	84.15241727	88.01710319];
asm_350=[5.745627891	28.81752317	43.90849754	55.87231217	66.82541342	79.09508865	72.50948804	93.08002815];
asm_400=[6.019101665	28.19017142	34.95320985	42.54899091	48.19216221	50.26107448	49.90298329	39.94643979];
asm_450=[5.602016341	31.34943791	46.2353987	52.61581494	61.77421293	67.82844033	66.26335131	75.18698363];

figure;
hold off;
chain_hops=1:1:8;
p=ones(1,10);
p(1)=semilogy(chain_hops,basic_250,'ro-.');
hold on;
p(2)=semilogy(chain_hops,basic_300,'r>-.');
p(3)=semilogy(chain_hops,basic_350,'rx-.');
p(4)=semilogy(chain_hops,basic_400,'r>-.');
p(5)=semilogy(chain_hops,basic_450,'rx-.');

p(6)=semilogy(chain_hops,asm_250,'bo-');
p(7)=semilogy(chain_hops,asm_300,'b>-');
p(8)=semilogy(chain_hops,asm_350,'bx-');
p(9)=semilogy(chain_hops,asm_400,'b>-');
p(10)=semilogy(chain_hops,asm_450,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
% axis([1 9 2 500]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend(p(1:5),'PRI-basic, size=250','PRI-basic, size=300','PRI-basic, size=350','PRI-basic, size=400','PRI-basic, size=450');
ah=axes('position',get(gca,'position'),'visible','off');
legend(ah,p(6:10),'PRI-ASM, size=250','PRI-ASM, size=300','PRI-ASM, size=350','PRI-ASM, size=400','PRI-ASM, size=450','west');
%legend('PRI-basic, size=250','PRI-basic, size=300','PRI-basic, size=350','PRI-basic, size=400','PRI-basic, size=450', ...
%    'PRI-ASM, size=250','PRI-ASM, size=300','PRI-ASM, size=350','PRI-ASM, size=400','PRI-ASM, size=450',1);
% grid on;