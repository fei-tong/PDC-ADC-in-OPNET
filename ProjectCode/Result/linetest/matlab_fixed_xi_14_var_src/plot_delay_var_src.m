% Plot Packet delivery latency, chain topology, from 1hop - 10 hops, fixed traffic load:0.2, frxed \xi = 14, src=(1,2,3)
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_src_1=[1.982018264	2.115112872	2.248082786	2.381097796	2.514142679	2.647061009	2.779974243	2.913075327	3.046091964	3.179062948];
basic_src_2=[4.136242418	7.236980123	7.370004301	7.502987755	7.636017564	7.768975917	7.901995559	8.034953119	8.167920217	9.049250369];
basic_src_3=[224.846329	481.5054602	772.2291068	772.3621491	772.4950961	772.6281392	772.7611414	772.8941206	773.8717279	768.2173568];
asm_src_1=[1.238417471	1.711433634	1.997606214	2.211254439	2.390151126	2.549089849	2.696265852	2.837502575	2.9759642	3.108935528];
asm_src_2=[1.235161658	1.828339955	2.483808281	3.015840108	3.462714288	3.859711808	4.223170269	4.557182569	4.789258054	5.352923774];
asm_src_3=[1.235475392	1.911851015	2.747200349	3.589179566	4.375327366	5.120501727	5.833892032	6.351116858	7.108633115	7.676292797];

figure;
hold off;
chain_hops=1:1:10;
semilogy(chain_hops,basic_src_1,'ro-.');
hold on;
semilogy(chain_hops,basic_src_2,'r>-.');
semilogy(chain_hops,basic_src_3,'rx-.');


semilogy(chain_hops,asm_src_1,'bo-');
semilogy(chain_hops,asm_src_2,'b>-');
semilogy(chain_hops,asm_src_3,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')

% axis([0.1 1 0 1]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend('PRI-basic, 1 src','PRI-basic, 2 srcs','PRI-basic, 3 srcs', ...
    'PRI-ASM, 1 src','PRI-ASM, 2 srcs','PRI-ASM, 3 srcs',1);
% grid on;