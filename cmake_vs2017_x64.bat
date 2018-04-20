cmake -E make_directory Build
cmake -E chdir Build cmake -G "Visual Studio 15 2017 Win64" -DALIMER_ENABLE_ALL=ON -DCMAKE_INSTALL_PREFIX="Alimer-SDK" %* ..