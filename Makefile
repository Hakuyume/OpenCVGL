TARGET=main

CXX_FLAGS=-Wall -O3 -std=c++11

GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut
EIGEN_FLAGS=-I.

$(TARGET): main.o phys.o render.o comp.o
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(GL_LIBS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ $(GL_FLAGS) $(EIGEN_FLAGS)

main.o: comp.hpp phys.hpp render.hpp
phys.o: comp.hpp phys.hpp
render.o: comp.hpp phys.hpp render.hpp
comp.o: comp.hpp

.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f *.o
