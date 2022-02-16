Cgt : C++ Game Tools

Pour générer le projet :
    mkdir build
    cd build
    cmake ..
    make
Puis pour exécuter :
    ./Cgt


Commandes :
    ZQSD    Déplacement
    C       Mode orbital On/Off (Utilise aussi ZQSD)
    UP/DOWN Modifier la vitesse de rotation du mode orbital


En cas de problème de build sur les ordinateurs de la FDS : relancer la DSIN, puis forcer le compilateur c++ à g++-10 dans build/CMakeCache.txt

Ce projet récupère des dépendances extérieures à l'aide de FetchContent. Vous pouvez activer FETCHCONTENT_UPDATES_DISCONNECTED dans CMakeCache.txt pour éviter que CMake ne cherche à vérifier en ligne si les dépendances sont à jour.
