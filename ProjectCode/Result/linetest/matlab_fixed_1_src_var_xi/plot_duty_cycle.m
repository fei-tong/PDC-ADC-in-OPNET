% Plot duty cycle, chain topology, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[0.034209177	0.050986435	0.067668193	0.084205635	0.095287179	0.095423731	0.095446718	0.095451401	0.095448629	0.095452381];
basic_17=[0.031337282	0.04803432	0.06450085	0.079654359	0.080351468	0.080377371	0.08038424	0.080389344	0.080388515	0.080378406];
basic_20=[0.029405174	0.046031177	0.062495057	0.06939033	0.069407483	0.069418289	0.069420522	0.069422529	0.069430465	0.069422485];
asm_14=[0.034264365	0.051319664	0.069346023	0.088495995	0.109211958	0.130057661	0.152122369	0.172061488	0.193909462	0.213653903];
asm_17=[0.031145066	0.048782902	0.067691516	0.087498082	0.108437697	0.129773546	0.151135586	0.172397131	0.190434195	0.211853424];
asm_20=[0.03000734	0.047696559	0.066350204	0.088183791	0.110421469	0.131732554	0.153565246	0.17442753	0.19562516	0.214580962];

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

%axis([0.05 0.5 0.012 0.026]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Duty Cycle');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',2);
% grid on;