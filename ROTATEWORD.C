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
 int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py,xtemp1,ytemp1;
 //color=rand()%NUM_COLORS;
 double phi = acos(-1);
 x1 = ((x1-centerx)*cos(degree*phi/180)-(y1-centery)*sin(degree*phi/180))+centerx;
 y1 = ((y1-centery)*cos(degree*phi/180)+(x1-centerx)*sin(degree*phi/180))+centery;
 x2 = ((x2-centerx)*cos(degree*phi/180)-(y2-centery)*sin(degree*phi/180))+centerx;
 y2 = ((y2-centery)*cos(degree*phi/180)+(x2-centerx)*sin(degree*phi/180))+centery;
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
	int x1,y1,x2,y2,color;
	float t1,t2;
	int i;

	set_mode(VGA_256_COLOR_MODE);
	/*x1=rand()%SCREEN_WIDTH;
	y1=rand()%SCREEN_HEIGHT;
	x2=rand()%SCREEN_WIDTH;
	y2=rand()%SCREEN_HEIGHT;
	color=rand()%NUM_COLORS;
	line_fast(x1,y1,x2,y2,color);*/
	//word : I
	line_rotate(160,40,160,20,1,-30,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);

	//word : N
	line_rotate(155,40,155,20,1,-20,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	line_rotate(165,40,155,20,1,-20,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	line_rotate(165,40,165,20,1,-20,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);

	//word : S
	line_rotate(165,20,155,20,1,-10,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	line_rotate(155,20,155,30,1,-10,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	line_rotate(155,30,165,30,1,-10,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	line_rotate(165,30,165,40,1,-10,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
	line_rotate(165,40,155,40,1,-10,SCREEN_WIDTH/2,SCREEN_HEIGHT/2);

	getch();
	return ;
}