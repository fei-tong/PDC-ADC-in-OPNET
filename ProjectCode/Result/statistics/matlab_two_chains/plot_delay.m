% Plot Packet delivery latency, chain topology, 10 hops, \xi = 14, 17, 20,fixed arrival rate=0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
analysis_14=[2.6576	3.1584	4.2485	8.4565	89.9323	102.6769	104.5324	105.3794	105.9235	106.3376];
analysis_18=[3.118	4.1479	7.8902	112.0889	128.8515	130.9413	131.9278	132.5955	133.1293	133.5955];
analysis_22=[3.6503	5.7197	36.6564	153.3624	156.8683	158.2322	159.1229	159.8366	160.4667	161.0519];


basic=[3.256380945	12.14067313	205.6686039	216.4164693	219.9929853	220.3770453	221.7185139	221.5173683	221.7345235	222.8165753;
    4.277176348	212.8892705	266.369786	272.8437264	273.8127196	274.8394725	275.8740316	276.393417	277.0639919	277.6931087;
    6.368382756	300.168616	321.0260125	326.6018813	327.693482	329.0825411	330.779734	330.1841022	329.733839	330.9906767
    ];
asm=[2.68652416	3.048583243	3.408072253	3.791603086	4.379307494	5.236013685	7.013679248	12.66589133	31.44270857	45.00102398;
    3.107441439	3.656182272	4.104547255	4.561430189	5.284994818	6.260551078	8.576819246	15.33321524	33.0235472	43.90355858;
    3.579645895	4.264571188	4.818578983	5.463582062	6.192614963	7.559562101	10.23095007	17.85092675	32.31371953	43.05005113
    ];

figure;
hold off;
arrival_rate=0.1:0.1:1;
semilogy(arrival_rate,analysis_14,'g+');
hold on;
plot(arrival_rate,analysis_18,'g^');
plot(arrival_rate,analysis_22,'gs');

plot(arrival_rate,basic(1,:),'ro-.');
plot(arrival_rate,basic(2,:),'r>-.');
plot(arrival_rate,basic(3,:),'rx-.');


plot(arrival_rate,asm(1,:),'bo-');
plot(arrival_rate,asm(2,:),'b>-');
plot(arrival_rate,asm(3,:),'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([0.1 1 2 500]);
xlabel('Traffic Load (packet/s)');
ylabel('Packet Delivery Latency (second)');
legend('model, basic, {{\it{\xi}} = 14}','model, basic, {{\it{\xi}} = 18}','model, basic, {{\it{\xi}} = 22}', ...
    'sim, basic, {{\it{\xi}} = 14}','sim, basic, {{\it{\xi}} = 18}','sim, basic, {{\it{\xi}} = 22}', ...
    'sim, ASM, {{\it{\xi}} = 14}','sim, ASM, {{\it{\xi}} = 18}','sim, ASM, {{\it{\xi}} = 22}',2);
% grid on;