% Plot PDR: packet delivery ratio, chain topology, from 1hop - 10 hops
% frxed \xi = 14, src=(1,2,3), traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_src_1=[0.999855491	0.999588221	0.999640118	0.998381284	0.936637325	0.787597075	0.669122433	0.583615589	0.51992755	0.468067351];
basic_src_2=[0.999862138	0.998434539	0.775210541	0.587317867	0.469663074	0.391148119	0.334738878	0.293159988	0.260680806	0.234935958];
basic_src_3=[0.999532867	0.783825131	0.523455056	0.38988967	0.314320582	0.260865663	0.223981579	0.19540628	0.173319473	0.156384113];
asm_src_1=[0.999450126	0.999646566	0.99967317	0.999094826	0.999194138	0.998682457	0.999212973	0.998638978	0.998692581	0.998055236];
asm_src_2=[0.999857213	0.99969015	0.999187555	0.998505641	0.998135374	0.998370523	0.998218585	0.998065648	0.997607732	0.936930972];
asm_src_3=[0.999537851	0.999086489	0.998687419	0.998469636	0.998226212	0.997546736	0.890148683	0.782170819	0.693952849	0.623684671];

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

axis([0.1 1 0 1]);
xlabel('Traffic Load (packet/s)');
ylabel('Packet Delivery Ratio');
legend('PRI-basic, 1 src','PRI-basic, 2 srcs','PRI-basic, 3 srcs', ...
    'PRI-ASM, 1 src','PRI-ASM, 2 srcs','PRI-ASM, 3 srcs',3);
% grid on;