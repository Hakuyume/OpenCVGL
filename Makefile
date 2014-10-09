GL_FLAGS=`pkg-config gl glu --cflags`
GL_LIBS=`pkg-config gl glu --libs` -lglut
EIGEN_FLAGS='-Ieigen/'

main: main.cpp
	g++ $^ -o $@ $(GL_FLAGS) $(EIGEN_FLAGS) $(GL_LIBS)
