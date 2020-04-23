function xdot = seir_eqn(x,t)
    % Parametrit
    tl   = 3;     % Latenssivaiheen kesto päivissä
    ti   = 3;     % Tartuttavuuden keskimääräinen kesto
    R0   = 2.4;   % Tartuttavuusluku
    beta = R0/ti; % Tartuttavuus per aikayksikkö

    % Puretaan muuttujat syötevektorista
    S = x(1); % Alttiit
    E = x(2); % Altistuneet (itämisvaiheessa olevat)
    I = x(3); % Sairastuneet (tartuttavat)
    R = x(4); % Toipuneet

    % Määritellään differentiaaliyhtälöt
    dS = -beta*S*I;         % Altteista poistuvat tartunnan saaneet
    dE = beta*S*I - E/tl;   % Alttiit saavat tartuntoja - sairastuvat
    dI = E/tl - I/ti;       % Tartunnan saaneet sairastuvat - toipuneet
    dR = I/ti;              % Sairaita toipuu

    % Palautetaan gradientit
    xdot = [dS,dE,dI,dR];
endfunction
  