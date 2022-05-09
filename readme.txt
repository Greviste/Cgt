Cgt : C++ Game Tools

Pour générer le projet :
    mkdir build
    cd build
    cmake .. -DCMAKE_CXX_COMPILER=<compilateur supportant c++20>
    make
(Si vous utilisez g++-10, call_cmake_ubuntu.sh effectue les 3 premières commandes)
    
Puis pour exécuter :
    ./Cgt


Commandes :
    ZQSD    Déplacement
    Clic G  Lancer une sphère



Ce projet récupère des dépendances extérieures à l'aide de FetchContent. Vous pouvez activer FETCHCONTENT_UPDATES_DISCONNECTED dans CMakeCache.txt pour éviter que CMake ne cherche à vérifier en ligne si les dépendances sont à jour.
