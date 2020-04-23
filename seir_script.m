t = linspace(0, 200, 201)+.1;

% Lähtotilanne vahvistetut * osuus todellisista
iAlku = 500*10;

% Jo sairastuneiden osuus väestöstä
ipAlku = iAlku*10/5500000;

% Lähtötilanne [S,E,I,R]
x0 = [1-ipAlku*3, ipAlku*2, ipAlku, 0];

x = lsode("seir_eqn", x0, t);
plot(t,x(:,1),"-y",t,x(:,2),"-g",t,x(:,3),"-r",t,x(:,4),"-b");
xlim([0 100]);
xlabel("Aika (päiviä)","fontweight","bold")
ylabel("Lukumäärä","fontweight","bold")
h = legend("S","E","I","R");
legend(h,"show")
