#release
gcc -O3 -DNDEBUG -o obfrel -std=c++1z -lstdc++ ../internal.cpp
#debug
gcc -o obfdbg -std=c++1z -lstdc++ ../internal.cpp

