GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut

main: main.cpp
	g++ $^ -o $@ $(GL_FLAGS) $(GL_LIBS)

