% Aikajana
t = linspace(0, 200, 201)+.1;

% Jo sairastuneiden (I) osuus (p) väestöstä
ipAlku = 0.0001;

% Jo toipuneiden (R) immuunien osuus (p) väestöstä
% rpAlku = 1-1/2.4
rpAlku = 0.0001;

% Lähtötilanne [S,E,I,R]
x0 = [1-ipAlku-rpAlku, 0, ipAlku, rpAlku];

x = lsode("seir_eqn", x0, t);
plot(t,x(:,1),"-g","linewidth",2,t,x(:,2),"-y","linewidth",2,t,x(:,3),"-r","linewidth",2,t,x(:,4),"-b","linewidth",2,[0,100],[0.58,0.58]);
xlim([0 100]);
ylim([0 1.0]);
set(gca,'linewidth',2);
set(gca,'fontsize',14); % Sets font of numbers on axes
xlabel("Aika (päiviä)","fontweight","bold","fontsize",16)
ylabel("Osuus väestöstä","fontweight","bold","fontsize",16)
h = legend("Alttiit (S)","Altistuneet (E)","Sairastuneet (I)","Toipuneet (R)", "Laumasuojakynnys");
set (h, "fontsize", 16);
legend(h, "location", "northeastoutside");
legend(h,"show")
