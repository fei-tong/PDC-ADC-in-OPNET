% Plot AEC: average energy consumption per node per second, chain topology, from 1hop - 10 hops
% \xi = 14, 17, 20, traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[0.064414698	0.071854441	0.079252345	0.086586233	0.091500086	0.091560816	0.091570628	0.091572886	0.091571888	0.09157339];
asm_14=[0.064434301	0.071977964	0.079918602	0.088317955	0.097362364	0.106440282	0.116036096	0.124707023	0.134221464	0.142823906];
basic_17=[0.063260423	0.070664948	0.077967243	0.084686769	0.084996097	0.085007562	0.085010509	0.085012651	0.08501232	0.085008276];
asm_17=[0.063169745	0.070964152	0.079274165	0.087936705	0.097058077	0.106337795	0.115633315	0.124892301	0.132756105	0.142113678];
basic_20=[0.062490362	0.069863336	0.077164319	0.080222105	0.080229672	0.080234291	0.080235287	0.080236095	0.080239265	0.080236069];
asm_20=[0.062749613	0.070552084	0.078725262	0.088232787	0.097889989	0.107136347	0.116622552	0.125704649	0.134948702	0.143228109];

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
% axis([0.05 0.5 0.054 0.061]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Power Consumption (watt)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',2);
% grid on;