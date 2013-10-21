#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <math.h>

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

/*********************************************************************
* set_mode							     *
*	Sets the video mode.					     *
**********************************************************************/

void set_mode(byte mode)
{
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

/*********************************************************************
* plot_pixel							     *
*	Plot a pixel by directly writing to video memory, with no    *
*	multiplication.						     *
*********************************************************************/

void plot_pixel(int x, int y, byte color)
{
	/* y*320 = y*256 + y*64 = y*2^8 + y*2^6 */
	VGA[(y<<8)+(y<<6)+x]=color;
}

/*********************************************************************
* line_fast							     *
*	draws a line using Bresenham's line-drawing algorithm, which *
*	uses no multiplication or division.			     *
*********************************************************************/

void line_fast(int x1, int y1, int x2, int y2, byte color)
{
 int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;
 //color=rand()%NUM_COLORS;
 dx=x2-x1;	/* the horizontal distance of the line */
 dy=y2-y1;	/* the vertical distance of the line */
 dxabs=abs(dx);
 dyabs=abs(dy);
 sdx=sgn(dx);
 sdy=sgn(dy);
 x=dyabs>>1;
 y=dxabs>>1;
 px=x1;
 py=y1;

 VGA[(py<<8)+(py<<6)+px]=color;

 if (dxabs>=dyabs)	/* the line is more horizontal than vertical */
 {
   for(i=0;i<dxabs;i++)
   {
     y+=dyabs;
     if (y>=dxabs)
     {
	y-=dxabs;
	py+=sdy;
     }
     px+=sdx;
     /*if ((i+1)%5==0){
      color = rand()%NUM_COLORS;
     }*/
	plot_pixel(px,py,color);
	//delay(100);
   }
 }
 else	/* the line is more vertical than horizontal */
 {
   for(i=0;i<dyabs;i++)
   {
     x+=dxabs;
     if(x>=dyabs)
     {
       x-=dyabs;
       px+=sdx;
     }
     py+=sdy;
     /*
     if ((i+1)%5==0){
      color=rand()%NUM_COLORS;
     }*/
     plot_pixel(px,py,color);
     //delay(100);
   }
 }
}

void line_rotate(int x1, int y1, int x2, int y2, byte color, double degree, int centerx, int centery)
{
 int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;
 //color=rand()%NUM_COLORS;
 double phi = acos(-1);
 double tempX1 = x1;
 double tempY1 = y1;
 double tempX2 = x2;
 double tempY2 = y2;
 x1 = (int)floor(((double)(tempX1-centerx)*(double)cos(degree*phi/180)-(double)(tempY1-centery)*(double)sin(degree*phi/180))+(double)centerx+ 0.5);
 y1 = (int)floor(((double)(tempY1-centery)*(double)cos(degree*phi/180)+(double)(tempX1-centerx)*(double)sin(degree*phi/180))+(double)centery+ 0.5);
 x2 = (int)floor(((double)(tempX2-centerx)*(double)cos(degree*phi/180)-(double)(tempY2-centery)*(double)sin(degree*phi/180))+(double)centerx+ 0.5);
 y2 = (int)floor(((double)(tempY2-centery)*(double)cos(degree*phi/180)+(double)(tempX2-centerx)*(double)sin(degree*phi/180))+(double)centery+ 0.5);
 dx=x2-x1;	/* the horizontal distance of the line */
 dy=y2-y1;	/* the vertical distance of the line */
 dxabs=abs(dx);
 dyabs=abs(dy);
 sdx=sgn(dx);
 sdy=sgn(dy);
 x=dyabs>>1;
 y=dxabs>>1;
 px=x1;
 py=y1;

 VGA[(py<<8)+(py<<6)+px]=color;

 if (dxabs>=dyabs)	/* the line is more horizontal than vertical */
 {
   for(i=0;i<dxabs;i++)
   {
     y+=dyabs;
     if (y>=dxabs)
     {
	y-=dxabs;
	py+=sdy;
     }
     px+=sdx;
     /*if ((i+1)%5==0){
      color = rand()%NUM_COLORS;
     }*/
	plot_pixel(px,py,color);
	//delay(100);
   }
 }
 else	/* the line is more vertical than horizontal */
 {
   for(i=0;i<dyabs;i++)
   {
     x+=dxabs;
     if(x>=dyabs)
     {
       x-=dyabs;
       px+=sdx;
     }
     py+=sdy;
     /*
     if ((i+1)%5==0){
      color=rand()%NUM_COLORS;
     }*/
     plot_pixel(px,py,color);
     //delay(100);
   }
 }
}

/* MAIN */
void main()
{
	//int x1,y1,x2,y2,color;
	//float t1,t2;
	//int i;
  int xmin,xmax,ymin,ymax,centery,centerx, color1, color2, color3,degree,degreespace,degreeI;
  degree = 0;
  degreeI = 13;
  degreespace = 30;
  xmin = 155;
  xmax = 165;
  ymin = 50;
  ymax = 40;
  centerx = SCREEN_WIDTH/2 ;
  centery = SCREEN_HEIGHT/2;
  color1 = 1;
  color2 = 2;
  color3 = 3;
	set_mode(VGA_256_COLOR_MODE);
	/*x1=rand()%SCREEN_WIDTH;
	y1=rand()%SCREEN_HEIGHT;
	x2=rand()%SCREEN_WIDTH;
	y2=rand()%SCREEN_HEIGHT;
	color=rand()%NUM_COLORS;
	line_fast(x1,y1,x2,y2,color);*/
  plot_pixel(centerx,centery,color1);
	//word : I
	line_rotate(centerx,ymin,centerx,ymax,color1,degree,centerx,centery);
  degree = degree+ degreeI;

	//word : N
	line_rotate(xmin,ymin,xmin,ymax,color1,degree,centerx,centery);
	line_rotate(xmax,ymin,xmin,ymax,color1,degree,centerx,centery);
	line_rotate(xmax,ymin,xmax,ymax,color1,degree,centerx,centery);
  degree = degree+ degreeI;

	//word : S
	line_rotate(xmax,ymax,xmin,ymax,color1,degree,centerx,centery);
	line_rotate(xmin,ymax,xmin,ymax-((ymax-ymin)/2),color1,degree,centerx,centery);
	line_rotate(xmin,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color1,degree,centerx,centery);
	line_rotate(xmax,ymax-((ymax-ymin)/2),xmax,ymin,color1,degree,centerx,centery);
	line_rotate(xmax,ymin,xmin,ymin,color1,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : T
  line_rotate(centerx,ymin,centerx,ymax,color1,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color1,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : I
  line_rotate(centerx,ymin,centerx,ymax,color1,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : T
  line_rotate(centerx,ymin,centerx,ymax,color1,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color1,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : U
  line_rotate(xmin,ymin,xmin,ymax,color1,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymin,color1,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color1,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : T
  line_rotate(centerx,ymin,centerx,ymax,color1,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color1,degree,centerx,centery);
  degree = degree+ degreespace;

  //word : T
  line_rotate(centerx,ymin,centerx,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color2,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : E
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color2,degree,centerx,centery);
  line_rotate(xmin,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color2,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color2,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : K
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymax-((ymax-ymin)/2),xmax,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymax-((ymax-ymin)/2),xmax,ymin,color2,degree,centerx,centery);
  degree = degree+ degreeI;
  
  //word : N
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color2,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : O
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color2,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color2,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : L
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color2,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : O
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color2,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color2,degree,centerx,centery);
  degree = degree+ degreeI;
  
  //word : G
  line_rotate(xmin,ymin,xmin,ymax,color2,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color2,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax-((ymax-ymin)/2),color2,degree,centerx,centery);
  line_rotate(centerx,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color2,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color2,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : I
  line_rotate(centerx,ymin,centerx,ymax,color2,degree,centerx,centery);
  degree = degree+ degreespace;
  
  //word : B
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color3,degree,centerx,centery);
  line_rotate(xmin,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : A
  line_rotate(xmin,ymin,centerx,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,centerx,ymax,color3,degree,centerx,centery);
  line_rotate(xmin+((xmax-xmin)/4),ymax-((ymax-ymin)/2),xmax-((xmax-xmin)/4),ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : N
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : D
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  degree = degree+ degreeI;  

  //word : U
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymin,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : N
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : G
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(centerx,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreespace;

//word : B
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color3,degree,centerx,centery);
  line_rotate(xmin,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : A
  line_rotate(xmin,ymin,centerx,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,centerx,ymax,color3,degree,centerx,centery);
  line_rotate(xmin+((xmax-xmin)/4),ymax-((ymax-ymin)/2),xmax-((xmax-xmin)/4),ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : N
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : D
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  degree = degree+ degreeI;  

  //word : U
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymin,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : N
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreeI;

  //word : G
  line_rotate(xmin,ymin,xmin,ymax,color3,degree,centerx,centery);
  line_rotate(xmin,ymin,xmax,ymin,color3,degree,centerx,centery);
  line_rotate(xmax,ymin,xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(centerx,ymax-((ymax-ymin)/2),xmax,ymax-((ymax-ymin)/2),color3,degree,centerx,centery);
  line_rotate(xmin,ymax,xmax,ymax,color3,degree,centerx,centery);
  degree = degree+ degreespace;
  

	getch();
	return ;
}