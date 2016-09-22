% Plot Packet delivery latency, fixed arrival rate=0.2, \xi=14,17,20
% \xi = 14, 17, 20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[4.471143981
6.146629674
7.248801458
7.71073921
8.02730243
8.442314828
8.914136109
9.682813839
9.290096485];
asm_14=[4.053849158
5.209342714
5.646541435
6.466746769
6.898723658
6.754528271
7.084409548
7.786216977
7.980593218];
basic_18=[10.90199493
19.56976329
24.44562984
29.36574722
28.49410103
29.72178901
31.37026299
32.59560738
31.08419212];
asm_18=[5.598002669
9.460120007
10.4139273
11.7205853
12.26113148
12.52275804
12.97395848
13.92793588
14.78214693];
basic_22=[95.14025658
180.7919537
320.3306439
414.5329785
421.836972
426.2733445
490.3270516
494.4632589
522.5422929];
asm_22=[8.062677257
15.9144623
21.77978059
24.22990285
25.70431761
24.89577043
26.7817799
27.66325835
25.31058926];

figure;
hold off;
chain_hops=1:1:9;
semilogy(chain_hops,basic_14,'ro-.');
hold on;
semilogy(chain_hops,basic_18,'r>-.');
semilogy(chain_hops,basic_22,'rx-.');


semilogy(chain_hops,asm_14,'bo-');
semilogy(chain_hops,asm_18,'b>-');
semilogy(chain_hops,asm_22,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([1 9 2 1000]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 18}','PRI-basic, {{\it{\xi}} = 22}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 18}','PRI-ASM, {{\it{\xi}} = 22}',2);
% grid on;