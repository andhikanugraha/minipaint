#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

#define VIDEO_INT		0x10	/* the BIOS video interrupt. */
#define SET_MODE		0X00	/* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE	0x13	/* use to set 256-color mode. */
#define TEXT_MODE		0x03	/* use to set 80x25 text mode. */

#define SCREEN_WIDTH		320	/* width in pixels of mode 0x13 */
#define SCREEN_HEIGHT		200	/* height in pixels of mode 0x13 */
#define NUM_COLORS		256	/* number of colors in mode 0x13 */

#define sgn(x)	((x<0) ?-1:((x>0)?1:0))	/* macro to return the sign of a number */

typedef unsigned char byte;
typedef unsigned short word;

byte far *VGA=(byte far*)0xA0000000L;		/* this points to video memory. */
word *my_clock=(word *)0x0000046C;	/* this points to the 18.2hz system clock. */

class Point {
	public : 
	int x;
	int y;
	Point() {
		x = 0;
		y = 0;
	}
	Point (int x_, int y_) {
		x = x_;
		y = y_;
	}
	void print() {
		printf("%d,%d  ", x,y);
	}
};

int compare_function(const void *a,const void *b) {
	Point *pa = (Point *) a;
	Point *pb = (Point *) b;
	return pa->x - pb->x;
}

Point getIntersection(Point p1, Point p2, Point p3, Point p4) {
	long int x12 = p1.x - p2.x;
	long int x34 = p3.x - p4.x;
	long int y12 = p1.y - p2.y;
	long int y34 = p3.y - p4.y;

	long int c = (x12 * y34) - (y12 * x34);

	if (c == 0)
	{
		return Point(-1,-1); //no intersection
	}
	else
	{
		// Intersection
		long int a = (p1.x * p2.y) - (p1.y * p2.x);
		long int b = (p3.x * p4.y) - (p3.y * p4.x);

		long int x = ((a * x34) - (b * x12)) / c;
		long int y = ((a * y34) - (b * y12)) / c;

		//printf("x:%ld , y:%ld \n", x,y);

		if ((y > p1.y) && (y > p2.y)) {
			return Point(-1,-1);
		}
		if ((y < p1.y) && (y < p2.y)) {
			return Point(-1,-1);
		}
		//printf("x:%ld, y:%ld \n", x,y);
		//printf("Point!");
		return Point(x,y);
	}
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
	VGA[(y<<8)+(y<<6)+x]=color;
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

	VGA[(py<<8) + (py<<6) + px] = color;

	/* the line is more horizontal than vertical */
	if (dxabs >= dyabs) {
		for(i=0; i<dxabs; i++) {
			y+=dyabs;
			if (y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			plot_pixel(px,py,color);
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
			plot_pixel(px,py,color);
		}
	}
}

/* MAIN */
int main() {
	int color;
	char temp;

	int xmin = 20;
	int xmax = 120;
	int ymin = 20;
	int ymax = 120;
	int i,j;

	Point points[50];
	int counter = 0;

	Point polygon_points[16] = {Point(20,20) , Point(60,20) ,Point(60,40) ,Point(80,40) ,Point(80,20) ,Point(120,20) ,Point(120,60) ,Point(100,60) ,Point(100,30) ,Point(98,30) ,Point(98,120) , Point(42,120) , Point(42,30) , Point(40,30) ,Point(40,60) ,Point(20,60)};

	set_mode(VGA_256_COLOR_MODE);
	color=rand()%NUM_COLORS;

	/* Draw Polygon based on polygon_points */
	for (i=0; i<16; i++) {
		if (i == 15) {
			line_fast(polygon_points[i],polygon_points[0], color);
		} else {
			line_fast(polygon_points[i],polygon_points[i+1], color);
		}
	}


	for (j = ymin; j<=ymax; j++) {
		counter = 0;
		for (i=0; i<16; i++) {
			if (i == 15) {
				Point u = getIntersection(polygon_points[i],polygon_points[0], Point(xmin, j), Point(xmax, j));
				if ((u.x != -1) && (u.y != -1)) {
					points[counter] = u;
					counter++;
				}
			} else {
				Point u = getIntersection(polygon_points[i],polygon_points[i+1], Point(xmin, j), Point(xmax, j));
				if ((u.x != -1) && (u.y != -1)) {
					points[counter] = u;
					counter++;
				}
			}
		}
		qsort(points,counter, sizeof(Point), compare_function);
		int o = 0;
		while (o < counter-1) {
			color = 2;
			line_fast(points[o], points[o+1],color);
			o = o+2;
		}
	}
	scanf("%c",&temp);
	return 0;
}