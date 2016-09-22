% Plot duty cycle: analysis vs. simulation
% \xi = 14, traffic load: 0.1:0.05:0.5
% PRI-basic, PRI-ASM (adaptive schedule maintaince)
analysis=[0.0348	0.0435	0.0521	0.0608	0.0695	0.0782	0.0869	0.0948	0.0984];
simulation=[0.034209177	0.042389621	0.050986435	0.059132803	0.067668193	0.075527273	0.084277868	0.090856199	0.094882099];

figure;
hold off;
arrival_rate=0.1:0.05:0.5;
plot(arrival_rate,analysis,'ko');
hold on;
plot(arrival_rate,simulation,'k>-.');

% set(gca,'XTick',0.05:0.05:0.5);
% set(gca,'XTickLabel',0.05:0.05:0.5);
%set(gca,'Xdir','reverse')
axis([0.1 0.5 0.03 0.1]);
xlabel('Traffic Load (packet/s)');
ylabel('Average Duty Cycle');
legend('model','simulation',4);
% grid on;