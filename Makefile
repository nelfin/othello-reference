# COMP3130 MAKEFILE
# Stephen Gould <stephen.gould@anu.edu.au>

FILES_H = board.h comms.h leader.h
FILES_CPP = board.cpp comms.cpp

FILES_OBJ = $(FILES_CPP:.cpp=.o)

CCC = g++

all: client server server_headless

server_headless: ${FILES_OBJ} server_headless.o
	${CCC} -g -o server_headless server_headless.o ${FILES_OBJ}

gui: ${FILES_OBJ} gui.cpp
	${CCC} -g `wxGTK/build/wx-config --cxxflags` -Wno-write-strings -fno-strict-aliasing \
		-o gui gui.cpp ${FILES_OBJ} `wxGTK/build/wx-config --libs core,base,gl`

server: ${FILES_OBJ} server.o
	${CCC} -g -o server server.o ${FILES_OBJ}

client: ${FILES_OBJ} client.o
	${CCC} -g -o client client.o ${FILES_OBJ}

%.o : %.cpp ${FILES_H}
	${CCC} -c $< -o $@

clean:
	rm -f client server server_headless
	rm -f *.o
