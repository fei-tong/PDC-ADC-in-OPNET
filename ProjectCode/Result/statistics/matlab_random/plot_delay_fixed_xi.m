% Plot Packet delivery latency, fixed \xi = 14, arrival rate=0.1,0.2,0.3
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_1=[2.864066651
3.177789385
3.646370402
3.811748932
4.203545673
4.247964343
4.582500801
4.691246771
5.109877522
];
asm_1=[2.73704508
3.006157146
3.393131762
3.711142584
3.835001423
4.202388985
4.346566417
4.654733936
4.847195443
];
basic_2=[4.471143981
6.146629674
7.248801458
7.71073921
8.02730243
8.442314828
8.914136109
9.682813839
9.290096485];
asm_2=[4.053849158
5.209342714
5.646541435
6.466746769
6.898723658
6.754528271
7.084409548
7.786216977
7.980593218
    ];
basic_3=[86.72831351
158.8845736
281.3979687
369.9797308
384.9651319
393.4587598
461.2631822
422.6704391
424.5665555
    ];
asm_3=[5.396854387
10.97584663
16.18956862
18.17447902
18.74254823
17.47186541
18.69322432
18.56323959
19.18170882
    ];

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
legend('PRI-basic, 0.1','PRI-basic, 0.15','PRI-basic, 0.2', ...
    'PRI-ASM, 0.1','PRI-ASM, 0.15','PRI-ASM, 0.2',1);
% grid on;