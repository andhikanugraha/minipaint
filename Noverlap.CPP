#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>
#include <conio.h>

#define VIDEO_INT		0x10	/* the BIOS video interrupt. */
#define SET_MODE		0X00	/* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE	0x13	/* use to set 256-color mode. */
#define TEXT_MODE		0x03	/* use to set 80x25 text mode. */

#define SCREEN_WIDTH		320	/* width in pixels of mode 0x13 */
#define SCREEN_HEIGHT		200	/* height in pixels of mode 0x13 */
#define NUM_COLORS		256	/* number of colors in mode 0x13 */

#define sgn(x)	((x<0) ?-1:((x>0)?1:0))	/* macro to return the sign of a number */
#define Vector2 Point /* Alias for the Point class */

#define Max_Point 4
#define Max_Polygons 10

#define byte unsigned char
#define word unsigned short
#define bool unsigned char
#define true 1
#define false 0

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

// shorthand for new Point
Point pt(int x, int y) {
	return Point(x,y);
}

class Polygon {
public:
	Point points[Max_Point];
	int n;
	byte color;

	Polygon() {
		n = 0;
	}

	void addPoint(Point p) {
		points[n] = p;
		++n;
	}

	void addPoint(int x, int y) {
		Point p(x, y);
		addPoint(p);
	}

	void drawStroke(byte color) {
		int length = n;
		int i;

		// first line, between the first and last points
		line_fast(points[0], points[length-1], color);

		// the other lines
		for (i = 1; i < length; i++) {
			line_fast(points[i-1], points[i], color);
		}
	}

	bool bounds(Point p) {
		return (p.x > minX()
				&& p.x < maxX()
				&& p.y > minY()
				&& p.y < maxY());
	}

	void fillColor(byte color) {
		int xmin = 0;
		int xmax = 200;
		int ymin = 0;
		int ymax = 200;
		int j;

		Point temppoints[50];
		int counter = 0;
		int i;
		for (i=0; i<n; i++) {
			if (i == n-1) {
				Point u = getIntersection(points[i],points[0], Point(xmin, j), Point(xmax, j));
				if ((u.x != -1) && (u.y != -1)) {
					temppoints[counter] = u;
					counter++;
				}
			} else {
				Point u = getIntersection(points[i],points[i+1], Point(xmin, j), Point(xmax, j));
				if ((u.x != -1) && (u.y != -1)) {
					temppoints[counter] = u;
					counter++;
				}
			}
		}

		qsort(temppoints, counter, sizeof(Point), compare_function);
		int o = 0;
		while (o < counter-1) {
			line_fast(temppoints[o], temppoints[o+1],color);
			o = o+2;
		}
	}

	void transpose(int dx, int dy) {
		int i;
		for (i=0; i<n; ++i) {
			points[i].x += dx;
			points[i].y += dy;
		}
	}

	int maxX() {
		int i;
		int x = 0;
		for (i=0; i<n; ++i) {
			if (points[i].x > x)
				x = points[i].x;
		}

		return x;
	}

	int minX() {
		int i;
		int x = SCREEN_WIDTH;
		for (i=0; i<n; ++i) {
			if (points[i].x < x)
				x = points[i].x;
		}

		return x;
	}

	int maxY() {
		int i;
		int y = 0;
		for (i=0; i<n; ++i) {
			if (points[i].y > y)
				y = points[i].y;
		}

		return y;
	}

	int minY() {
		int i;
		int y = SCREEN_HEIGHT;
		for (i=0; i<n; ++i) {
			if (points[i].y < y)
				y = points[i].y;
		}

		return y;
	}

	Point midpoint() {
		int midX = ((maxX() - minX()) / 2) + minX();
		int midY = ((maxY() - minY()) / 2) + minY();
		return Point(midX, midY);
	}

	bool overlapsWith(Polygon b) {
		int i;
		for (i=0; i<n; i++) {
			if (b.bounds(points[i]))
				return 1;
		}

		return 0;
	}

	void draw() {
		drawStroke(color);
		fillColor(color);
	}

	void predraw(byte _c) {
		drawStroke(_c);
	}

	void predraw() {
		drawStroke(6 + (color / 10));
	}

	void predraw(int dx, int dy) {
		transpose(dx, dy);
		predraw();
		transpose(-dx, -dy);
	}

	// TODO fillColor

	// TODO fillPattern
};

bool isOverlap(Polygon a, Polygon b) {
	int i, j;

	for (i=0; i<a.n; i++) {
		Point p = a.points[i];
		if (b.bounds(p)) {
			return 1;
		}
	}

	return 0;
}

int getcolor(int x, int y) {
	return VGA[y*SCREEN_WIDTH+x];
}

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
	if (x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT)
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

Polygon allPolygons[Max_Polygons];
int nPolygons = 0;

Polygon* addPolygon(Polygon po) {
	allPolygons[nPolygons] = po;
	++nPolygons;

	return &allPolygons[nPolygons-1];
}

void transposePolygon(Polygon* a, int dx, int dy, int padding);

byte shove(Polygon* a, Polygon* b, int padding) {
	// a = new, b = old (b will be moved)
	// return 1=right, 2=left, 3=up, 4=down

	Point ma = a->midpoint();
	Point mb = b->midpoint();

	int dx = mb.x - ma.x;
	int dy = mb.y - ma.y;

	unsigned int adx = abs(dx);
	unsigned int ady = abs(dy);

	int d;
	if (ady > adx) {
		if (dy<0) {
			d = -1;
			dy = abs(a->minY() - b->maxY()) * -1;
		}
		else {
			d = 1;
			dy = abs(a->maxY() - b->minY());
		}
		printf("Y%c%d ", d>0?'+':'-', abs(dy));
		// b->transpose(0, dy + (padding * d));
		transposePolygon(b, 0, dy + (padding * d), padding);
	}
	else {
		if (dx<0) { // old shape is to the left
			d = -1;
			dx = abs(a->minX() - b->maxX()) * -1;
		}
		else {
			d = 1;
			dx = abs(a->maxX() - b->minX());
		}
		printf("X%c%d ", d>0?'+':'-', abs(dx));
		// b->transpose(dx + (padding * d), 0);
		transposePolygon(b, dx + (padding * d), 0, padding);
	}
}
byte shove(Polygon* a, Polygon* b) {
	shove(a, b, 1);
}

Polygon** getOverlappingPolygons(Polygon* po, int* _n) {
	Polygon* overlappingPolygons[Max_Polygons];
	int n = 0;

	int i = 0;
	while (i < nPolygons) {
		if (allPolygons[i].overlapsWith(*po)) {
			overlappingPolygons[n] = &allPolygons[i];
			++n;
		}

		++i;
	}

	*_n = n;

	return overlappingPolygons;
}

void transposePolygon(Polygon* a, int dx, int dy, int padding) {
	int n;
	a->predraw(dx, dy);
	a->transpose(dx, dy);
	Polygon** op = getOverlappingPolygons(a, &n);

	if (n > 0) {
		int i = 0;
		while (i < n) {
			shove(a, op[i], padding);

			i++;
		}
	}
}

void addPolygonAndShove(Polygon po, int padding) {
	Polygon* overlappingPolygons[Max_Polygons];
	int n = 0;

	int i = 0;
	while (i < nPolygons) {
		if (allPolygons[i].overlapsWith(po)) {
			overlappingPolygons[n] = &allPolygons[i];
			++n;
		}

		++i;
	}

	po.predraw(10);

	printf("%d overlaps", n);

	if (n > 0) {
		Polygon* newPolygon = addPolygon(po);

		i = 0;
		while (i < n) {
			getch();
			printf("\n%d = ", i);
			Polygon* oldPolygon = overlappingPolygons[i];
			shove(newPolygon, oldPolygon, padding);

			++i;
		}
	}
	else {
		addPolygon(po);
	}
}
void addPolygonAndShove(Polygon po) {
	addPolygonAndShove(po, 5);
}

bool tryPolygon(Polygon po) {
	bool overlaps = false;

	int i = 0;
	while (!overlaps && i < nPolygons) {
		if (allPolygons[i].overlapsWith(po))
			overlaps = true;

		++i;
	}

	return overlaps;
}

void clear() {
	// TODO something more appropriate

	set_mode(TEXT_MODE);
	set_mode(VGA_256_COLOR_MODE);

	/*
	int i;
	for (i=0; i<SCREEN_HEIGHT; ++i) {
		line_fast(Point(0,i), Point(SCREEN_WIDTH,i), 0);
	}
	*/

	/*
	int i, j;
	for (i=0; i<SCREEN_WIDTH; ++i) {
		for (j=0; j<SCREEN_HEIGHT; ++j) {
			plot_pixel(i,j,0);
		}
	}
	*/
}

void redraw() {
	clear();
	for (int i=0; i<nPolygons; ++i) {
		allPolygons[i].draw();
	}
}

void transposeAll(int dx, int dy) {
	for (int i=0; i<nPolygons; ++i) {
		allPolygons[i].transpose(dx, dy);
	}
}

/* MAIN */
int main() {
	int color;
	char temp;
	
	int xmin = 50;
	int xmax = 126;
	int ymin = 25;
	int ymax = 100;
	int pdg;

	int i,j;

	Point points[50];
	int counter = 0;

	// Point polygon1[4] = {Point(50,50) , Point(100,50) ,Point(100,100) ,Point(50,100)};
	//Point polygon2[4] = {Point(75,25) , Point(125,25) ,Point(125,75) ,Point(75,75)};

	// try new way
	Polygon poly1;
	poly1.addPoint(50,50);
	poly1.addPoint(100,50);
	poly1.addPoint(100,100);
	poly1.addPoint(50,100);
	poly1.color = 11;

	Polygon poly2;
	poly2.addPoint(75,25);
	poly2.addPoint(125,25);
	poly2.addPoint(125,75);
	poly2.addPoint(75,75);
	poly2.color = 34;

	Polygon poly3;
	poly3.addPoint(25,26);
	poly3.addPoint(65,26);
	poly3.addPoint(65,76);
	poly3.addPoint(25,76);
	poly3.color = 20;

	Polygon poly4;
	poly4.addPoint(85,24);
	poly4.addPoint(135,24);
	poly4.addPoint(135,74);
	poly4.addPoint(85,74);
	poly4.color = 80;

	Polygon poly5;
	poly5.addPoint(85,27);
	poly5.addPoint(135,27);
	poly5.addPoint(135,77);
	poly5.color = 90;

	clrscr();
	printf("Non-overlapping shape insertion demo\n\nInsert padding size (default 4): ");
	scanf("%d",&temp);
	temp = abs(temp);
	if (temp == 0)
		temp = 4;
	pdg = temp + 1;

	set_mode(VGA_256_COLOR_MODE);
	addPolygonAndShove(poly2, pdg);
	scanf("%c",&temp);
	redraw();
	addPolygonAndShove(poly4, pdg);
	scanf("%c",&temp);
	redraw();
	scanf("%c",&temp);
	addPolygonAndShove(poly3, pdg);
	scanf("%c",&temp);
	redraw();
	scanf("%c",&temp);
	addPolygonAndShove(poly1, pdg);
	scanf("%c",&temp);
	redraw();
	scanf("%c",&temp);
	addPolygonAndShove(poly5, pdg);
	scanf("%c",&temp);
	redraw();

	scanf("%c",&temp);
	set_mode(TEXT_MODE);
	return 0;
}