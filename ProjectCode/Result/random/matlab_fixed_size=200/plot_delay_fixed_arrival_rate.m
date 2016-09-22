% Plot Packet delivery latency, fixed arrival rate=0.2, \xi=14,17,20
% \xi = 14, 17, 20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[4.655595928
6.097257237
7.470889664
8.242168439
8.854827079
9.030423626
9.015862111
9.554668512
9.718901765];
asm_14=[3.906646991
5.370965493
6.156967932
6.594628801
7.107679685
7.42609043
7.809706576
8.349521301
8.438454314];
basic_17=[10.56274182
14.89773889
21.59694371
23.0405945
23.83880821
25.08604992
26.28579147
27.15565876
27.26347743];
asm_17=[4.961396218
7.748326917
9.20816026
10.61181341
11.12286322
11.40977312
11.82428583
12.36747373
11.85141659];
basic_20=[45.32827688
68.93333881
92.01823024
113.8110584
117.8031
130.1101614
135.923192
148.2626891
161.1029632];
asm_20=[7.509056521
11.338167
14.7156541
16.20125659
16.65510005
17.11363989
17.48956702
18.05349191
17.42663569];

figure;
hold off;
chain_hops=1:1:9;
semilogy(chain_hops,basic_14,'ro-.');
hold on;
semilogy(chain_hops,basic_17,'r>-.');
semilogy(chain_hops,basic_20,'rx-.');


semilogy(chain_hops,asm_14,'bo-');
semilogy(chain_hops,asm_17,'b>-');
semilogy(chain_hops,asm_20,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([1 9 2 2000]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',2);
% grid on;