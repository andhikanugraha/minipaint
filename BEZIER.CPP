#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>
#include <conio.h>
#include <limits.h>

#define VIDEO_INT		0x10	/* the BIOS video interrupt. */
#define SET_MODE		0X00	/* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE	0x13	/* use to set 256-color mode. */
#define TEXT_MODE		0x03	/* use to set 80x25 text mode. */

#define SCREEN_WIDTH		320	/* width in pixels of mode 0x13 */
#define SCREEN_HEIGHT		200	/* height in pixels of mode 0x13 */
#define NUM_COLORS		256	/* number of colors in mode 0x13 */

#define sgn(x)	((x<0) ?-1:((x>0)?1:0))	/* macro to return the sign of a number */
#define Vector2 Point /* Alias for the Point class */

#define Max_Point 10

#define byte unsigned char
#define word unsigned short
#define bool unsigned char
#define true 1
#define false 0

byte far *VGA=(byte far*)0xA0000000L;		/* this points to video memory. */
word *my_clock=(word *)0x0000046C;	/* this points to the 18.2hz system clock. */

typedef struct 
{
  int x, y;
}
Point;

int getcolor(int x, int y) {
	return VGA[y*SCREEN_WIDTH+x];
}

int compare_function(const void *a,const void *b) {
	Point *pa = (Point *) a;
	Point *pb = (Point *) b;
	return pa->x - pb->x;
}

/* Set to Video Mode */
void set_mode(byte mode)
{
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

/* Plot a pixel by directly writing to video memory */
void plot_pixel(int x, int y, byte color)
{
	/* y*320 = y*256 + y*64 = y*2^8 + y*2^6 */
	if (x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT)
		VGA[(y<<8)+(y<<6)+x]=color;
}
void plot_pixel(Point p, byte color) {
	plot_pixel(p.x, p.y, color);
}

/* Bresenham's Line Algorithm */
void line_fast(Point p1, Point p2, byte color)
{
	int i, sdx, sdy, dxabs, dyabs, x, y, px, py;
	//color=rand()%NUM_COLORS;
	dxabs = abs(p2.x-p1.x); /* the abs of horizontal distance of the line */
	dyabs = abs(p2.y-p1.y); /* the abs of vertical distance of the line */
	sdx = sgn(p2.x-p1.x);
	sdy = sgn(p2.y-p1.y);
	x = dyabs>>1;
	y = dxabs>>1;
	px = p1.x;
	py = p1.y;
	
	if (( VGA[(py<<8) + (py<<6) + px] == 0) || ( VGA[(py<<8) + (py<<6) + px] == 15) || ( VGA[(py<<8) + (py<<6) + px] < color)) {
		VGA[(py<<8) + (py<<6) + px] = color;
	}
	
	/* the line is more horizontal than vertical */
	if (dxabs >= dyabs) {
		for(i=0; i<dxabs; i++) {
			y+=dyabs;
			if (y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			if ((getcolor(px,py) == 0) || (getcolor(px,py) == 15) || (getcolor(px,py) < color)) {
				plot_pixel(px,py,color);
			}
		}
	}
	/* the line is more vertical than horizontal */
	else {
		for(i=0; i<dyabs; i++) {
			x += dxabs;
			if(x >= dyabs) {
				x -= dyabs;
				px += sdx;
			}
			py += sdy;
			if ((getcolor(px,py) == 0) || (getcolor(px,py) == 15)  || (getcolor(px,py) < color)) {	
				plot_pixel(px,py,color);
			}
		}
	}
}

void clear() {
	// TODO something more appropriate

	set_mode(TEXT_MODE);
	set_mode(VGA_256_COLOR_MODE);
}

/* -- BEZIER CURVE FUNCTIONS -- */
// Linear
float bezier(float t, float p0f, float p1f) {
	return ((1.0f - t) * p0f) + (t * p1f);
}
// Quadratic
float bezier(float t, float p0f, float p1f, float p2f) {
	float tt = 1.0f - t;
	return (tt * ((tt * p0f) + (t * p1f))) + (t * ((tt * p1f) + (t * p2f)));
}
// Cubic
float bezier(float t, float p0f, float p1f, float p2f, float p3f) {
	return ((1.0f - t) * bezier(t, p0f, p1f, p2f)) + (t * bezier(t, p1f, p2f, p3f));
}
// Recursive - BUGGY
float bezier(float t, float* d, int n) {
	float pt;

	if (n == 0) {
		// Basis
		pt = d[0];
	}
	else {
		// Rekurens
		float earlier[Max_Point];
		float latter[Max_Point];

		int i;
		for (i=0; i<n-1; ++i) {
			earlier[i] = d[i];
			latter[i] = d[i+1];
		}

		float left = ((1.0f - t) * bezier(t, earlier, n-1));
		float right = t * bezier(t, latter, n-1);

		pt = left + right;
	}

	return pt;
}

// Curve functions with points
Point bezier(float t, Point p0, Point p1) {
	Point pt;
	pt.x = (int) bezier(t, p0.x, p1.x);
	pt.y = (int) bezier(t, p0.y, p1.y);

	return pt;
}
Point bezier(float t, Point p0, Point p1, Point p2) {
	Point pt;
	pt.x = (int) bezier(t, p0.x, p1.x, p2.x);
	pt.y = (int) bezier(t, p0.y, p1.y, p2.y);

	return pt;
}
Point bezier(float t, Point p0, Point p1, Point p2, Point p3) {
	Point pt;
	pt.x = (int) bezier(t, p0.x, p1.x, p2.x, p3.x);
	pt.y = (int) bezier(t, p0.y, p1.y, p2.y, p3.y);

	return pt;
}

// Recursive for > 4 points, but not working
Point bezier(float t, Point* points, int n) {
	Point pt, p0, p1, p2, p3;
	switch (n) {
		case 1:
			pt = points[0];
			break;
		case 2:
			p0 = points[0];
			p1 = points[1];
			pt = bezier(t, p0, p1);
			break;
		case 3:
			p0 = points[0];
			p1 = points[1];
			p2 = points[2];
			pt = bezier(t, p0, p1, p2);
			break;
		case 4:
			p0 = points[0];
			p1 = points[1];
			p2 = points[2];
			p3 = points[3];
			pt = bezier(t, p0, p1, p2, p3);
			break;
		default: {
			float xs[Max_Point];
			float ys[Max_Point];

			int i;
			for (i=0; i<n; ++i) {
				xs[i] = (float) points[i].x;
				ys[i] = (float) points[i].y;
			}

			pt.x = (int) bezier(t, xs, n);
			// printf("X(%f)=%f ", t, bezier(t, xs, n));
			pt.y = (int) bezier(t, ys, n);
			// printf("Y(%f)=%f ", t, bezier(t, ys, n));
			// char temp;
			// scanf("%c", &temp);
		}			
	}

	return pt;
}

void plot_bezier(Point* points, int n, byte color, unsigned int granularity) {
	float granularity_f = (float) granularity;

	unsigned int i;
	float f, t;
	Point pt;
	for (i=0; i<granularity; ++i) {
		f = (float) i;
		t = f / granularity_f;
		pt = bezier(t, points, n);

		plot_pixel(pt, color);
	}
}
void plot_bezier(Point* points, int n, byte color) {
	unsigned int granularity = INT_MAX;
	plot_bezier(points, n, color, granularity);
}

/* MAIN */
int main() {
	int granularity;
	int color;
	char temp;

	int i,j;
	int tx,ty;

	Point points[Max_Point];
	char n = 0;

	clrscr();

	// TODO User input here

	printf("Bezier curve demo\n\n");

	// Request type of Bezier curve/number of points
	printf("Number of Bezier points: ");
	scanf("%d", &n);
	while (n < 2 || n > 4) {
		printf("Invalid value. Please enter an integer between 2 and 4.\n");
		printf("Number of Bezier points: ");
		scanf("%d", &n);
	}

	// Request each point
	for (i=0; i<n; ++i) {
		printf("Coordinates for point %d: ", i);
		scanf("%d %d", &tx, &ty);

		Point tp;
		tp.x = tx;
		tp.y = ty;
		points[i] = tp;
	}

	printf("Color: ");
	scanf("%d", &color);

	set_mode(VGA_256_COLOR_MODE);

	// Trace a gray line between the points
	for (i=1; i<n; ++i) {
		line_fast(points[i-1], points[i], 8);
	}

	// Plot each point white
	for (i=0; i<n; ++i) {
		plot_pixel(points[i], 15);
	}

	// Draw the bezier curve
	plot_bezier(points, n, color);

	// Plot each point white (again)
	for (i=0; i<n; ++i) {
		plot_pixel(points[i], 15);
	}

	scanf("%c",&temp);
	scanf("%c",&temp);
	set_mode(TEXT_MODE);
	return 0;
}