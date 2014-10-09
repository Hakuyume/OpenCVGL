GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut
EIGEN_FLAGS='-Ieigen/'

main: main.o phys.o
	g++ $^ -o $@ $(GL_LIBS)

main.o: main.cpp phys.hpp
	g++ -c main.cpp $(GL_FLAGS) $(EIGEN_FLAGS)

phys.o: phys.hpp phys.cpp
	g++ -c phys.cpp $(EIGEN_FLAGS)
