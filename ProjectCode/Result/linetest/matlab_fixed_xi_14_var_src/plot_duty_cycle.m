% Plot duty cycle, chain topology, from 1hop - 10 hops
% frxed \xi = 14, src=(1,2,3), traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_src_1=[0.034209177	0.050986435	0.067668193	0.084205635	0.095287179	0.095423731	0.095446718	0.095451401	0.095448629	0.095452381];
basic_src_2=[0.048619371	0.079892812	0.092579063	0.094225321	0.095444807	0.095440722	0.095450085	0.095459467	0.095453029	0.095443164];
basic_src_3=[0.062014426	0.089434443	0.092485963	0.094275064	0.095437744	0.095454243	0.095445822	0.095452806	0.095441583	0.095450326];
asm_src_1=[0.034264365	0.051319664	0.069346023	0.088495995	0.109211958	0.130057661	0.152122369	0.172061488	0.193909462	0.213653903];
asm_src_2=[0.049317918	0.085175731	0.1240541	0.163799896	0.202335782	0.240010103	0.274434278	0.311201657	0.342972101	0.357931966];
asm_src_3=[0.064205498	0.116179467	0.173253737	0.226275499	0.277589552	0.3253328	0.342552274	0.348519144	0.354718723	0.358109321];

figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,basic_src_1,'ro-.');
hold on;
plot(arrival_rate,basic_src_2,'r>-.');
plot(arrival_rate,basic_src_3,'rx-.');


plot(arrival_rate,asm_src_1,'bo-');
plot(arrival_rate,asm_src_2,'b>-');
plot(arrival_rate,asm_src_3,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')

%axis([0.05 0.5 0.012 0.026]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Duty Cycle');
legend('PRI-basic, 1 src','PRI-basic, 2 srcs','PRI-basic, 3 srcs', ...
    'PRI-ASM, 1 src','PRI-ASM, 2 srcs','PRI-ASM, 3 srcs',2);
% grid on;