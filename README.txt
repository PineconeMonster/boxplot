Compiling requires some variant of glut installed. For debian, try installing the package "freeglut3-dev"

For GCC, use the following command:
	gcc -std=c89 main.c -lm -lGL -lGLU -lglut -o boxplot

Basically, you need to link opengl, glu, glut, and the c math library.

Usage: boxplot <list of integers>
