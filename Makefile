TARGET=main

CXX_FLAGS=-Wall -O3 -std=c++11

GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut
EIGEN_FLAGS=-I.

$(TARGET): main.o phys.o render.o glsl.o Box.o
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(GL_LIBS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ $(GL_FLAGS) $(EIGEN_FLAGS)

main.o: phys.hpp render.hpp
phys.o: phys.hpp
render.o: phys.hpp render.hpp glsl.h Box.h
glsl.o: glsl.h
Box.o: Box.h

.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f *.o
