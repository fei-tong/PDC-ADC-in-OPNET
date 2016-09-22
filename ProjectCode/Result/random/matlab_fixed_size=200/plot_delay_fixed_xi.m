% Plot Packet delivery latency, fixed arrival rate=0.2, 
% \xi = 14, arrival rate=0.1,0.2,0.3
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_1=[2.677577623
3.224754187
3.524581951
3.750365773
3.986844394
4.312763182
4.571843977
4.800647479
5.245552982
];
asm_1=[2.674987958
3.226589885
3.373005803
3.726041851
4.027607203
4.135806302
4.381952927
4.646110954
5.204986929
];
basic_2=[4.655595928
6.097257237
7.470889664
8.242168439
8.854827079
9.030423626
9.015862111
9.554668512
9.718901765];
asm_2=[3.906646991
5.370965493
6.156967932
6.594628801
7.107679685
7.42609043
7.809706576
8.349521301
8.438454314];
basic_3=[89.68493627
172.2894263
298.7933638
394.0039568
414.9963598
417.5770532
472.7306066
475.7495161
467.9815732];
asm_3=[5.638864789
11.00094891
15.84599751
17.64630883
18.98790681
17.70055088
18.54698107
18.91201238
17.55247814];

figure;
hold off;
chain_hops=1:1:9;
semilogy(chain_hops,basic_1,'ro-.');
hold on;
semilogy(chain_hops,basic_2,'r>-.');
semilogy(chain_hops,basic_3,'rx-.');


semilogy(chain_hops,asm_1,'bo-');
semilogy(chain_hops,asm_2,'b>-');
semilogy(chain_hops,asm_3,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([1 9 2 500]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend('PRI-basic, 0.1','PRI-basic, 0.2','PRI-basic, 0.3', ...
    'PRI-ASM, 0.1','PRI-ASM, 0.2','PRI-ASM, 0.3',1);
% grid on;