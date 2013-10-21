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
 
/*
1001 | 1000 | 1010 
0001 | 0000 | 0010 
0101 | 0100 | 0110
*/
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000
const int TOP_LEFT = 9; //1001
const int TOP_RIGHT = 10; //1010
const int BOTTOM_LEFT = 5; //0101
const int BOTTOM_RIGHT = 6; //0110

int xmin = 100;
int xmax = 200;
int ymin = 50;
int ymax = 150;
 
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
 
// Compute the bit code for a Point(x, y)
// Border : xmin - xmax, ymin - ymax 
int getAreaCode(Point p)
{
        int code = INSIDE;
	if (p.y < ymin) {
		if (p.x < xmin) 	code = TOP_LEFT;
		else if (p.x <= xmax) 	code = TOP;
		else 			code = TOP_RIGHT;
	} else 
	if (p.y <= ymax) {
		if (p.x < xmin) 	code = LEFT;
		else if (p.x <= xmax) 	code = INSIDE;
		else 			code = RIGHT;
	}
	else {
		if (p.x < xmin) 	code = BOTTOM_LEFT;
		else if (p.x <= xmax) 	code = BOTTOM;
		else 			code = BOTTOM_RIGHT;
	}
        return code;
}
 
void BinaryClipping(Point a, Point b, int color)
{
        int codeA = getAreaCode(a);
        int codeB = getAreaCode(b);
        int accept = 0;
 
        while (accept == 0) {
		//printf("masuk while  - codeA : %d, codeB : %d\n", codeA, codeB);
		//printf("A : %d,%d  - B : %d,%d \n", a.x, a.y, b.x, b.y);
                if (!(codeA | codeB)) { // Bitwise OR is 0. Trivially accept and get out of loop
                        accept = 1;
                        break;
                } 
		else if (codeA & codeB) { // Bitwise AND is not 0. Trivially reject and get out of loop
                        break;
                } else {
			long int x, y;
                        // At least one endpoint is outside the clip rectangle; pick it.
                        int codeOut = codeA ? codeA : codeB;
 
                        // Find the intersection point;
                        // Use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
                        if (codeOut & TOP) {           // point is above the clip rectangle
                                x = a.x + ((b.x - a.x) * (ymax - a.y) / (b.y - a.y));
                                y = ymax;
				//printf("TOP! x:%d, y:%d\n",x,y);
                        } else if (codeOut & BOTTOM) { // point is below the clip rectangle
                                x = a.x + ((b.x - a.x) * (ymin - a.y) / (b.y - a.y));
                                y = ymin;
				//printf("BOTTOm! x:%d, y:%d\n",x,y);
                        } else if (codeOut & RIGHT) {  // point is to the right of clip rectangle
                                y = a.y + ((b.y - a.y) * (xmax - a.x) / (b.x - a.x));
                                x = xmax;
				//printf("RIGHT! x:%d, y:%d\n",x,y);
                        } else if (codeOut & LEFT) {   // point is to the left of clip rectangle
                                y = a.y + ((b.y - a.y) * (xmin - a.x) / (b.x - a.x));
                                x = xmin;
				//printf("LEFT! x:%d, y:%d\n",x,y);
                        }
 
                        // Now we move outside point to intersection point to clip and get ready for next pass.
                        if (codeOut == codeA) {
                                a.x = x;
                                a.y = y;
                                codeA = getAreaCode(a);
				//printf("code A! %d , x,y : %d,%d\n", codeA, a.x, a.y);
                        } else {
                                b.x = x;
                                b.y = y;
                                codeB = getAreaCode(b);
				//printf("code B! %d , x,y : %d,%d\n", codeB, b.x, b.y);
			}
                }
        }
        if (accept == 1) {
               //printf("FOUND! A : %d,%d   & B: %d,%d\n", a.x, a.y, b.x, b.y);
	       line_fast(a, b, color);
        }
}

int main() {
	int color = rand()%NUM_COLORS;
	int temp; 
	
	set_mode(VGA_256_COLOR_MODE);
	
	/* Draw Rectangle */
	line_fast(Point(xmin,ymin), Point(xmax,ymin), color);
	line_fast(Point(xmax,ymin), Point(xmax,ymax), color);
	line_fast(Point(xmax,ymax), Point(xmin,ymax), color);
	line_fast(Point(xmin,ymax), Point(xmin,ymin), color);
	
	/* Draw Clipping Line */
	color = 35;
	plot_pixel(30,70, color);
	plot_pixel(250,120, color);
	BinaryClipping(Point(30,70), Point (250,120), color);

	color = 3;
	plot_pixel(90,70,color);
	plot_pixel(220,70,color);
	BinaryClipping(Point(90,70), Point (220,70), color);

	color = 100;
	plot_pixel(120,20,color);
	plot_pixel(170,180,color);
	BinaryClipping(Point(120,20), Point (170,180), color);

	color = 76;
	plot_pixel(70,20,color);
	plot_pixel(220,10, color);
	BinaryClipping(Point(70,20), Point(220,10), color);

	scanf("%d",&temp);

	return 0;
}