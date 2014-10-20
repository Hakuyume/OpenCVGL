TARGET=main

CXX_FLAGS=-Wall -O3 -std=c++11

CV_FLAGS=`pkg-config opencv --cflags`
CV_LIBS=`pkg-config opencv --libs`
GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut
EIGEN_FLAGS=-I.

$(TARGET): main.o phys.o render.o mc.o comp.o
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(CV_LIBS) $(GL_LIBS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ $(CV_FLAGS) $(GL_FLAGS) $(EIGEN_FLAGS)

main.o: comp.hpp phys.hpp mc.hpp render.hpp
render.o: comp.hpp phys.hpp mc.hpp render.hpp
mc.o: comp.hpp phys.hpp mc.hpp
phys.o: comp.hpp phys.hpp
comp.p: comp.hpp

.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f *.o
