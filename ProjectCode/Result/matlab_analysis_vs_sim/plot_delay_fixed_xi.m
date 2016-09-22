% Plot Packet delivery latency, 
% model vs. simulation
% \xi = 14, arrival rate=0.1,0.2,0.3,0.4,0.5
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
model_1=[1.460626016
1.593626016
1.726626016
1.859626016
1.992626016
2.125626016
2.258626016
2.391626016
2.524626016
2.657626016];
model_2=[1.9614
2.0944
2.2274
2.3604
2.4934
2.6264
2.7594
2.8924
3.0254
3.1584];
model_3=[3.0515
3.1845
3.3175
3.4505
3.5835
3.7165
3.8495
3.9825
4.1155
4.2485];
model_4=[7.1646
7.2976
7.4306
7.5636
7.6966
7.8296
7.9626
8.0956
8.2286
8.3616];
model_5=[28.6815
28.8145
28.9475
29.0805
29.2135
29.3465
29.4795
29.6125
29.7455
29.8785];
sim_1=[1.441813618
1.574864347
1.707836778
1.840806569
1.973834653
2.106769413
2.239682566
2.372844425
2.505825497
2.638808467];
sim_2=[1.982018264
2.115112873
2.248082786
2.381097796
2.514142679
2.647061009
2.779974243
2.913075327
3.046091964
3.179062948];
sim_3=[3.102769126
3.235743886
3.368727288
3.501717207
3.634744033
3.767680214
3.900707225
4.033724157
4.166771059
4.29974591];
sim_4=[6.941069928
7.074150459
7.207138966
7.340111118
7.473096361
7.606045347
7.739112928
7.872094763
8.005133733
8.138144966];
sim_5=[28.6862539
28.81924048
28.95221617
29.08528154
29.2182457
29.351224
29.48424559
29.61721781
29.75018836
29.88326939];

figure;
hold off;
chain_hops=1:1:10;
p=ones(1,10);
p(1)=semilogy(chain_hops,model_1,'bo');
hold on;
p(2)=semilogy(chain_hops,model_2,'b>');
p(3)=semilogy(chain_hops,model_3,'bx');
p(4)=semilogy(chain_hops,model_4,'bs');
p(5)=semilogy(chain_hops,model_5,'bd');

p(6)=semilogy(chain_hops,sim_1,'ko-');
p(7)=semilogy(chain_hops,sim_2,'k>-');
p(8)=semilogy(chain_hops,sim_3,'kx-');
p(9)=semilogy(chain_hops,sim_4,'ks-');
p(10)=semilogy(chain_hops,sim_5,'kd-');
% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([1 9 0 200]);
xlabel('Path Length (hop)');
ylabel('Packet Delivery Latency (second)');

legend(p(1:5),'model, 0.1','model, 0.2','model, 0.3','model, 0.4','model, 0.5');
ah=axes('position',get(gca,'position'),'visible','off');
legend(ah,p(6:10),'simulation, 0.1','simulation, 0.2','simulation, 0.3','simulation, 0.4','simulation, 0.5','location','west');
% legend('model, 0.1','model, 0.2','model, 0.3','model, 0.4','model, 0.5', ...
%     'simulation, 0.1','simulation, 0.2','simulation, 0.3','simulation, 0.4','simulation, 0.5',1);
% grid on;