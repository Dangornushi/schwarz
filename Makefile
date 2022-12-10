PRG    = ./build/schwarz
OBJDIR = ./build/obj/
OBJ    = main.o move.o display.o visualMode.o util.o finder.o colorscheme.o

CFLAGS = -g -O2 -std=c++1z
LFLAGS = -lcurses
CC     =  g++ $(CFLAGS)

$(PRG) : $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS)

.cpp.o:
	$(CC) -c $<

clean:
	rm -rf $(PRG) $(OBJDIR)/*.o

./build/colorscheme.o: colorscheme.cpp schwarz.hpp
./build/display.o: display.cpp schwarz.hpp move.hpp util.hpp finder.hpp
./build/finder.o: finder.cpp schwarz.hpp
./build/main.o: main.cpp schwarz.hpp move.hpp util.hpp finder.hpp visualMode.hpp
./build/move.o: move.cpp schwarz.hpp
./build/util.o: util.cpp schwarz.hpp
./build/visualMode.o: visualMode.cpp schwarz.hpp

