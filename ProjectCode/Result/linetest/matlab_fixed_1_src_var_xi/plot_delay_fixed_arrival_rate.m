% Plot Packet delivery latency, chain topology, from 1hop - 10 hops
% \xi = 14, 17, 20,fixed arrival rate=0.2, \xi=14,17,20
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
basic_14=[1.982018264	2.115112872	2.248082786	2.381097796	2.514142679	2.647061009	2.779974243	2.913075327	3.046091964	3.179062948];
asm_14=[1.238417471	1.711433634	1.997606214	2.211254439	2.390151126	2.549089849	2.696265852	2.837502575	2.9759642	3.108935528];
basic_17=[2.627844385	2.760930855	2.893941223	3.026848255	3.159913982	3.292876573	3.425753973	3.558858345	3.691941602	3.824850279];
asm_17=[1.437010966	2.04692426	2.434481588	2.711941601	2.932919486	3.12200147	3.293099479	3.453299356	3.606168019	3.739250675];
basic_20=[3.707663992	3.840729935	3.973743809	4.106666386	4.239748259	4.372647023	4.505616878	4.638683344	4.771726739	4.904681332];
asm_20=[1.668694736	2.444883995	2.964786264	3.352640724	3.665193318	3.931886062	4.167749644	4.380479194	4.577271837	4.710332815];

figure;
hold off;
chain_hops=1:1:10;
plot(chain_hops,basic_14,'ro-.');
hold on;
plot(chain_hops,basic_17,'r>-.');
plot(chain_hops,basic_20,'rx-.');


plot(chain_hops,asm_14,'bo-');
plot(chain_hops,asm_17,'b>-');
plot(chain_hops,asm_20,'bx-');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
% axis([1 9 2 2000]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');
legend('PRI-basic, {{\it{\xi}} = 14}','PRI-basic, {{\it{\xi}} = 17}','PRI-basic, {{\it{\xi}} = 20}', ...
    'PRI-ASM, {{\it{\xi}} = 14}','PRI-ASM, {{\it{\xi}} = 17}','PRI-ASM, {{\it{\xi}} = 20}',4);
% grid on;