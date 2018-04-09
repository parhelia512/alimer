cmake -E make_directory Build
cmake -E chdir Build cmake -G "Visual Studio 15 2017 Win64" %* ..