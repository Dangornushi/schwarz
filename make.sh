echo building...
g++ -o schwarz -std=c++1z main.cpp -lcurses
./schwarz test.cpp
rm -f .DS_Store
