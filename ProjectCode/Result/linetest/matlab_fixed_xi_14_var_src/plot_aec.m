% Plot AEC: average energy consumption per node per second, chain topology, from 1hop - 10 hops
% frxed \xi = 14, src=(1,2,3), traffic load: 0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_src_1=[0.064414698	0.071854441	0.079252345	0.086586233	0.091500086	0.091560816	0.091570628	0.091572886	0.091571888	0.09157339];
basic_src_2=[0.07080399	0.08467162	0.090297932	0.091028746	0.091569744	0.091568592	0.09157241	0.091576189	0.091573636	0.091569675];
basic_src_3=[0.076742617	0.088901957	0.090256535	0.091050509	0.091567095	0.091574036	0.091570745	0.091573525	0.091569044	0.091572561];
asm_src_1=[0.064434301	0.071977964	0.079918602	0.088317955	0.097362364	0.106440282	0.116036096	0.124707023	0.134221464	0.142823906];
asm_src_2=[0.071089402	0.086850413	0.103807173	0.12110722	0.137894255	0.15434869	0.169410601	0.185534383	0.199484334	0.206055935];
asm_src_3=[0.077641668	0.100360277	0.125218557	0.148360583	0.170822418	0.19177354	0.199329368	0.201938036	0.204650505	0.206133597];

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
ylabel('Average Power Consumption (watt)');
legend('PRI-basic, 1 src','PRI-basic, 2 srcs','PRI-basic, 3 srcs', ...
    'PRI-ASM, 1 src','PRI-ASM, 2 srcs','PRI-ASM, 3 srcs',2);
% grid on;