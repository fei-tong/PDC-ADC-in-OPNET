% Plot PDR: packet delivery ratio, fixed: xi=14, var traffic load=0.4,0.45,0.5. var size: 250,300,350,400,450
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_4=[0.529714286	0.664608328	0.64966998	0.652073563	0.622073093];
basic_45=[0.469329672	0.595595165	0.565382069	0.57743834	0.531619146];
basic_5=[0.42108721	0.527911711	0.504130536	0.51486749	0.478209138];
asm_4=[0.979955169	0.972846626	0.972183785	0.970605175	0.976715766];
asm_45=[0.965093671	0.944162574	0.914656274	0.940538091	0.918615517];
asm_5=[0.907920923	0.902579496	0.816425456	0.888241498	0.824763376];

figure;
hold off;
network_size=250:50:450;
plot(network_size,basic_4,'ro-.');
hold on;
plot(network_size,basic_45,'r>-.');
plot(network_size,basic_5,'rx-.');


plot(network_size,asm_4,'bo-');
plot(network_size,asm_45,'b>-');
plot(network_size,asm_5,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
%axis([0.05 0.5 0.012 0.026]);
xlabel('Network Size');
ylabel('Packet Delivery Ratio');
legend('PRI-basic, 0.4','PRI-basic, 0.45','PRI-basic, 0.5', ...
    'PRI-ASM, 0.4','PRI-ASM, 0.45','PRI-ASM, 0.5',1);
% grid on;