CCFLAGS = --std=c++11 -pthread -Wall
MAIN = main.cpp
PREVIEW = preview_main.cpp
CPPS = color.cpp bmp.cpp image.cpp ray.cpp vector3.cpp scene.cpp object.cpp camera.cpp kdtree.cpp
LIBS = lib/tinyxml2.cpp

all: main

main:
	g++ $(MAIN) $(CPPS) $(LIBS) $(CCFLAGS) -O4 -o main.exe

gdb:
	g++ $(MAIN) $(CPPS) $(LIBS) $(CCFLAGS) -g -o main.exe

bmp2image:
	g++ bmp2image.cpp color.cpp bmp.cpp image.cpp $(CCFLAGS) -O3 -o bmp2image.exe

clean:
	del *.exe *~
