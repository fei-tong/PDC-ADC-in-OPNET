% Plot duty cycle, fixed: xi=14, var traffic load=0.4,0.45,0.5. var size: 250,300,350,400,450
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_4=[0.024018672	0.023819707	0.023717098	0.023627468	0.023522382];
basic_45=[0.024099398	0.0238809	0.023755273	0.02366355	0.023549584];
basic_5=[0.024188121	0.023928622	0.023794207	0.02369243	0.023567023];
asm_4=[0.024290988	0.023936426	0.023840626	0.023718342	0.023642816];
asm_45=[0.024563355	0.02409304	0.023934312	0.023816032	0.0237577];
asm_5=[0.024761632	0.024236077	0.024031824	0.023900695	0.02382527];

figure;
hold off;
network_size=250:50:450;
plot(network_size,basic_4,'ro-.');
hold on;
plot(network_size,basic_45,'r>-.');
plot(network_size,basic_5,'rx-.');


plot(network_size,asm_4,'bo-');
plot(network_size,asm_45,'b>-');
plot(network_size,asm_5,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
%axis([0.05 0.5 0.012 0.026]);
xlabel('Network Size');
ylabel('Average Duty Cycle');
legend('PRI-basic, 0.4','PRI-basic, 0.45','PRI-basic, 0.5', ...
    'PRI-ASM, 0.4','PRI-ASM, 0.45','PRI-ASM, 0.5',1);
% grid on;