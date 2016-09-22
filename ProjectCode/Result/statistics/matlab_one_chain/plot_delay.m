% Plot Packet delivery latency, chain topology, 10 hops, \xi = 14, 17, 20,fixed arrival rate=0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
analysis_14=[2.6576	3.1584	4.2485	8.4565	89.9323	102.6769	104.5324	105.3794	105.9235	106.3376];
analysis_18=[3.118	4.1479	7.8902	112.0889	128.8515	130.9413	131.9278	132.5955	133.1293	133.5955];
analysis_22=[3.6503	5.7197	36.6564	153.3624	156.8683	158.2322	159.1229	159.8366	160.4667	161.0519];
basic_14=[2.638808466	3.179062947	4.299745909	8.185073345	82.28309504	99.73532038	102.9278741	104.0369179	104.669373	105.0932456];
basic_18=[3.106366213	4.191748725	7.840280213	102.1458818	125.2596259	128.7752037	129.9042449	130.5084788	131.0284679	131.3239764];
basic_22=[3.600313575	5.813010995	29.05226202	147.076172	153.0322109	155.2086864	156.1011109	156.749693	157.0716539	157.3315849];
asm_14=[2.390059534	2.436761875	2.463931487	2.496791792	2.529667044	2.562247482	2.588942376	2.638956919	2.685313307	2.74659871];
asm_18=[2.651705086	2.717274521	2.745649673	2.764558945	2.788051496	2.810898502	2.842360471	2.880288892	2.909690085	2.979058655];
asm_22=[2.961829073	2.99052225	3.008984784	3.01609608	3.034822914	3.043956019	3.061458356	3.100722047	3.134767001	3.187055092];

figure;
hold off;
arrival_rate=0.1:0.1:1;
semilogy(arrival_rate,analysis_14,'g+');
hold on;
plot(arrival_rate,analysis_18,'g^');
plot(arrival_rate,analysis_22,'gs');

plot(arrival_rate,basic_14,'ro-.');
plot(arrival_rate,basic_18,'r>-.');
plot(arrival_rate,basic_22,'rx-.');


plot(arrival_rate,asm_14,'bo-');
plot(arrival_rate,asm_18,'b>-');
plot(arrival_rate,asm_22,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([0.1 1 0 180]);
xlabel('Packet Generation Rate \lambda (packet/s)');
ylabel('Packet Delivery Latency (second)');
legend('model, basic, {{\it{\xi}} = 14}','model, basic, {{\it{\xi}} = 18}','model, basic, {{\it{\xi}} = 22}', ...
    'sim, basic, {{\it{\xi}} = 14}','sim, basic, {{\it{\xi}} = 18}','sim, basic, {{\it{\xi}} = 22}', ...
    'sim, ASM, {{\it{\xi}} = 14}','sim, ASM, {{\it{\xi}} = 18}','sim, ASM, {{\it{\xi}} = 22}',4);
% grid on;