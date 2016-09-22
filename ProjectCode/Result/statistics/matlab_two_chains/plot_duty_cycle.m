% Plot duty cycle, chain topology, 10 hops, \xi = 14, 17, 20,fixed arrival rate=0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
analysis_14=[0.0348	0.0521	0.0695	0.0869	0.0988	0.0984	0.0981	0.0979	0.0977	0.0976];
analysis_18=[0.0313	0.0487	0.066	0.0791	0.0786	0.0784	0.0782	0.0781	0.078	0.078];
analysis_22=[0.029	0.0464	0.0637	0.0656	0.0653	0.0651	0.0651	0.065	0.065	0.0649];
basic=[0.034275278	0.05071664	0.054186602	0.053943965	0.053564272	0.053600459	0.053377993	0.053345116	0.053298866	0.053071027;
    0.030496258	0.043437523	0.043142589	0.04286378	0.042819923	0.042737603	0.042568449	0.042487276	0.0423902	0.042293048;
    0.028188309	0.036110843	0.035866211	0.035686143	0.035615077	0.035478837	0.035329348	0.035345607	0.03533315	0.035233979
    ];
asm=[0.034432945	0.052027044	0.071328472	0.090360306	0.110299925	0.130434233	0.150220582	0.169931172	0.183252588	0.185076326;
    0.030800655	0.049538185	0.06880811	0.088198931	0.108463442	0.128114555	0.14745075	0.167580997	0.180617274	0.183613096;
    0.02898332	0.047463365	0.066992326	0.087359203	0.106789222	0.126421059	0.146513454	0.165855204	0.17757374	0.182115535
    ];

figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,analysis_14,'g+');
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

%axis([0.05 0.5 0.012 0.026]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Duty Cycle');
legend('model, basic, {{\it{\xi}} = 14}','model, basic, {{\it{\xi}} = 18}','model, basic, {{\it{\xi}} = 22}', ...
    'sim, basic, {{\it{\xi}} = 14}','sim, basic, {{\it{\xi}} = 18}','sim, basic, {{\it{\xi}} = 22}', ...
    'sim, ASM, {{\it{\xi}} = 14}','sim, ASM, {{\it{\xi}} = 18}','sim, ASM, {{\it{\xi}} = 22}',2);
% grid on;