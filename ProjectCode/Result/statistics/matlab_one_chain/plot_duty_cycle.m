% Plot duty cycle, chain topology, 10 hops, \xi = 14, 17, 20,fixed arrival rate=0.1:0.1:1
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
analysis_14=[0.0348	0.0521	0.0695	0.0869	0.0988	0.0984	0.0981	0.0979	0.0977	0.0976];
analysis_18=[0.0313	0.0487	0.066	0.0791	0.0786	0.0784	0.0782	0.0781	0.078	0.078];
analysis_22=[0.029	0.0464	0.0637	0.0656	0.0653	0.0651	0.0651	0.065	0.065	0.0649];
basic_14=[0.034209177	0.050986435	0.067668193	0.084205635	0.095225307	0.095152398	0.094968801	0.094798968	0.094652461	0.094519467];
basic_18=[0.030818338	0.047160124	0.063841973	0.076140495	0.07608098	0.075909025	0.075758301	0.075621468	0.075509639	0.075405496];
basic_22=[0.028300296	0.045048086	0.060774901	0.063414094	0.063246666	0.063085746	0.062967484	0.062859235	0.062775073	0.06271285];
asm_14=[0.034632357	0.052619685	0.069681314	0.089006667	0.107691588	0.127020337	0.146252748	0.164705802	0.183466874	0.203331707];
asm_18=[0.031127041	0.048878809	0.067031491	0.086667434	0.104906144	0.125070145	0.145264621	0.164224411	0.183633753	0.202269007];
asm_22=[0.028963275	0.046988983	0.065151957	0.084177009	0.103935924	0.123561244	0.142281391	0.163405648	0.181939861	0.202147992];

figure;
hold off;
arrival_rate=0.1:0.1:1;
plot(arrival_rate,analysis_14,'g+');
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

%axis([0.05 0.5 0.012 0.026]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Duty Cycle');
legend('model, basic, {{\it{\xi}} = 14}','model, basic, {{\it{\xi}} = 18}','model, basic, {{\it{\xi}} = 22}', ...
    'sim, basic, {{\it{\xi}} = 14}','sim, basic, {{\it{\xi}} = 18}','sim, basic, {{\it{\xi}} = 22}', ...
    'sim, ASM, {{\it{\xi}} = 14}','sim, ASM, {{\it{\xi}} = 18}','sim, ASM, {{\it{\xi}} = 22}',2);
% grid on;