CXX_FLAGS=-Wall -O3 -std=c++11

GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut
EIGEN_FLAGS=-Ieigen/

main: main.o phys.o
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(GL_LIBS)

main.o: main.cpp phys.hpp
	$(CXX) $(CXX_FLAGS) -c main.cpp $(GL_FLAGS) $(EIGEN_FLAGS)

phys.o: phys.hpp phys.cpp
	$(CXX) $(CXX_FLAGS) -c phys.cpp $(EIGEN_FLAGS)

clean:
	rm -f main
	rm -f *.o
