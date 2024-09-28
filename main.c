#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define view_scale 16

/* Plot info required for rendering */
int *list, ln, *outliers, outlier_count;
int min, max; float q1, q2, q3;
int plot_min, plot_max;
int win_x, win_y;

float absf(float a){
	return a >= 0 ? a : -a;
}

void swapInt(int *a, int *b){
	int c = *a;
	*a = *b;
	*b = c;
	return;
}

int powerOfTen(int power){
	int out = 1, i = power;
	for(;--i+1;)
		out = out * 10;
	return out;
}

int readInt(char*str){
	int ln=-1, out=0, c=1, p=-1, i, sign=1;

	/*Get length*/
	while(1){
		if(str[++ln]=='\0')
			break;
		if(ln > 255)
			exit(EXIT_FAILURE);
	}

	assert(ln != 0);

	/*Check sign*/
	if(str[0] == '-')
		sign=-1;

	/*Read integer*/
	for(i=ln;--i+1;)
		if(!(sign == -1 && i == 0))
			out += powerOfTen(++p) * (str[i] - '0');

	return out * sign;
}

int *readArgs(int *ln, int argc, char**argv){
	int i;
	*ln = argc - 1;
	int *out = malloc(sizeof(int) * *ln);

	for(i=*ln; --i+1;)
		out[i] = readInt(argv[i+1]);

	return out;
}

void sortList(int ln, int *list){

	assert(ln > 0);
	int i, ii;

	for(ii=0;ii<ln;++ii)
	for(i=0;i<ln-1;++i)
		if(list[i] > list[i+1])
			swapInt(&list[i], &list[i+1]);
	return;
}

void getSummary(int ln, int *list,
		int *min, int *max,
		float *q1, float *q2, float *q3){

	assert(ln > 0);
	int i,
	q1_place = floor(0.25 * (float)ln),
	q2_place = floor(0.5 * (float)ln),
	q2_width = ln % 2 == 0 ? 2 : 1,
	q3_place = q2_place + q1_place;

	*q1 = list[q1_place];
	if(q2_width == 2)
		*q2 = 0.5 * (list[q2_place-1] + list[q2_place]);
	else
		*q2 = list[q2_place];
	*q3 = list[q3_place];

	int minout = list[0], maxout = list[0];
	for(i=ln;--i;){
		if(list[i] < minout)
			minout = list[i];
		if(list[i] > maxout)
			maxout = list[i];
	}
	*min = minout;
	*max = maxout;

	return;
}

void display(){
	glClearColor(255, 255, 255, 255);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_x, win_y, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-min * view_scale, 0, 0);

	/*Displacement*/
	int d = 2 * view_scale,/*X border displacement*/
	    n = 1 * view_scale,/*Near*/
	    m = 2 * view_scale,/*Middle*/
	    f = 3 * view_scale;/*Far*/

	glColor3ub(0, 0, 0);

	glLineWidth(2);
	glBegin(GL_LINES);

	/*Whisker A*/
	glVertex2i(d + plot_min * view_scale, m);
	glVertex2i(d + (int)q1 * view_scale, m);

	/*Box A*/
	glVertex2i(d + (int)q1 * view_scale, n);
	glVertex2i(d + (int)q1 * view_scale, f);
	glVertex2i(d + (int)q1 * view_scale, n);
	glVertex2i(d + (int)q2 * view_scale, n);
	glVertex2i(d + (int)q1 * view_scale, f);
	glVertex2i(d + (int)q2 * view_scale, f);
	glVertex2i(d + (int)q2 * view_scale, n);
	glVertex2i(d + (int)q2 * view_scale, f);

	/*Box B*/
	glVertex2i(d + (int)q2 * view_scale, n);
	glVertex2i(d + (int)q2 * view_scale, f);
	glVertex2i(d + (int)q2 * view_scale, n);
	glVertex2i(d + (int)q3 * view_scale, n);
	glVertex2i(d + (int)q2 * view_scale, f);
	glVertex2i(d + (int)q3 * view_scale, f);
	glVertex2i(d + (int)q3 * view_scale, n);
	glVertex2i(d + (int)q3 * view_scale, f);

	/*Whisker B*/
	glVertex2i(d + (int)q3 * view_scale, m);
	glVertex2i(d + plot_max * view_scale, m);
	glEnd();

	/*Outliers*/
	int i;
	glPointSize(3);
	glBegin(GL_POINTS);
	for(i=outlier_count;--i+1;)
		glVertex2i(d + outliers[i] * view_scale, m);
	glEnd();

	glutSwapBuffers();
	return;
}

int main(int argc, char**argv){
	if(argc == 1){
		puts("usage: boxplot <integer> ...");
		return 0;
	}
	int i, ii;
	list = readArgs(&ln, argc, argv);

	sortList(ln, list);

	getSummary(ln, list, &min, &max, &q1, &q2, &q3);
	printf("summary:\n%i %f %f %f %i\n",min, q1, q2, q3, max);

	float IQR = absf(q1 - q3);
	float upper_bound = q3 + 1.5 * IQR;
	float lower_bound = q1 - 1.5 * IQR;
	printf("bounds:\n%f %f\n", lower_bound, upper_bound);

	plot_max = min, plot_min = max;

	/*Find bounds excluding outliers and
	   count total outliers for the list*/

	outlier_count=0;
	for(i=ln;--i+1;)
		if(list[i] <= upper_bound && list[i] >= lower_bound){
			if(list[i] > plot_max)
				plot_max = list[i];
			if(list[i] < plot_min)
				plot_min = list[i];
		}else
			++outlier_count;

	printf("plot min: %i max: %i\n", plot_min, plot_max);

	/*Create a list of outliers*/

	outliers = malloc(sizeof(int) * outlier_count);
	ii=-1;
	for(i=ln;--i+1;)
		if(list[i] > upper_bound || list[i] < lower_bound)
			outliers[++ii] = list[i];

	/*Sort and print the list to stdio*/
	if(outlier_count){
		sortList(outlier_count, outliers);
		for(i=0; i<outlier_count; ++i)
			printf("outlier no. %i: %i\n", i+1, outliers[i]);
	}

	int view_width = abs(max - min);

	win_x = view_width * view_scale + 4 * view_scale;
	win_y = 4 * view_scale;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

	glutInitWindowSize(win_x, win_y);

	glutCreateWindow("Box plot");
	glutDisplayFunc(display);

	glutMainLoop();

	free(list);
	free(outliers);
	return 0;
}
