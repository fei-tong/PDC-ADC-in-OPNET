% Plot PDR: packet delivery ratio, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[0.999855491	0.999588221	0.999640118	0.998381284	0.936637325	0.787597075	0.669122433	0.583615589	0.51992755	0.468067351];
basic_17=[0.999859353	0.999659062	0.999301851	0.987035457	0.792128366	0.661420315	0.567071446	0.494478696	0.437139326	0.397778752];
basic_20=[0.999583799	0.999309911	0.998290416	0.851634663	0.683820934	0.571758089	0.489995997	0.428235396	0.379970705	0.341527156];
asm_14=[0.999450126	0.999646566	0.99967317	0.999094826	0.999194138	0.998682457	0.999212973	0.998638978	0.998692581	0.998055236];
asm_17=[0.999571244	0.999362799	0.999908381	0.999203198	0.998896309	0.99827071	0.998462387	0.997878251	0.998297261	0.998040423];
asm_20=[0.999721835	0.999446654	0.999353594	0.998309741	0.998808609	0.99760493	0.997494972	0.998153374	0.997834326	0.997051126];

figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,basic_14,'ro-.');
hold on;
plot(arrival_rate,basic_17,'r>-.');
plot(arrival_rate,basic_20,'rx-.');


plot(arrival_rate,asm_14,'bo-');
plot(arrival_rate,asm_17,'b>-');
plot(arrival_rate,asm_20,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')

xlabel('Traffic Load (packet/s)');
ylabel('Packet Delivery Ratio');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',3);
% grid on;