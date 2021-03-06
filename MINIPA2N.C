#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <math.h>
//#include <graphics.h>
#include <mem.h>
#include <limits.h>

#define VIDEO_INT           0x10
#define SET_MODE            0x00
#define VGA_256_COLOR_MODE  0x13
#define TEXT_MODE           0x03

#define SCREEN_WIDTH        320 
#define SCREEN_HEIGHT       200 
#define NUM_COLORS          256 
#define LINE_COLOR          7
#define FILL_COLOR          1
#define SIZE(x)  (sizeof(x) / sizeof(x[0]))
#define Max_Point           4
#define Max_Shape           10
#define Max_draw            99


typedef unsigned char  byte;
typedef unsigned short word;

byte far *VGA=(byte far*)0xA0000000L;        /* this points to video memory. */

typedef struct 
{
  int x, y;
}Point;

typedef struct 
{
  Point p[Max_Point];
  int color;
}Shape;

typedef struct 
{
  Shape s[Max_Shape];
}menu;

typedef struct
{
  Point p[2];
  int color;
}Line;

typedef struct tagBITMAP              /* the structure for a bitmap. */
{
  word width;
  word height;
  byte *data;
} BITMAP;


menu M;
Line AL[99];
Shape AS[99];
Shape colorP[16];
int Ccolor = 0;
int CFunc = -1;

int num_AS = 0;
int num_AL = 0;
union REGS in, out;

void set_mode(byte mode)
{
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

void putpixel(int x, int y, byte color)
{
  VGA[y*SCREEN_WIDTH+x]=color;
}

int getcolor(int x, int y)
{
  return VGA[y*SCREEN_WIDTH+x];
}

int signum(int x)
{
  if (x > 0)
  {
    return 1;
  }
  else if (x < 0)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

void fskip(FILE *fp, int num_bytes)
{
   int i;
   for (i=0; i<num_bytes; i++)
      fgetc(fp);
}

Point createPoint(int x, int y)
{
  Point tempP;
  tempP.x = x;
  tempP.y = y;
  return tempP;
}

Shape createShape(Point p1, Point p2)
{
  Shape tempS;
  tempS.p[0]=p1;
  tempS.p[1].x=p1.x;
  tempS.p[1].y=p2.y;
  tempS.p[2]=p2;
  tempS.p[3].x=p2.x;
  tempS.p[3].y=p1.y;
  tempS.color=0;
  return tempS;
}

void drawline(Point p1, Point p2, int color)
{
  int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;

  dx=p2.x-p1.x;      /* the horizontal distance of the line */
  dy=p2.y-p1.y;      /* the vertical distance of the line */
  dxabs=abs(dx);
  dyabs=abs(dy);
  sdx=signum(dx);
  sdy=signum(dy);
  x=dyabs>>1;
  y=dxabs>>1;
  px=p1.x;
  py=p1.y;

  putpixel(px,py,color);

  if (dxabs>=dyabs) /* the line is more horizontal than vertical */
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

      putpixel(px,py,color);
    }
  }
  else /* the line is more vertical than horizontal */
  {
    for(i=0;i<dyabs;i++)
    {
      x+=dxabs;
      if (x>=dyabs)
      {
        x-=dyabs;
        px+=sdx;
      }
      py+=sdy;
      
      putpixel(px,py,color);    
    }
  }
}

void drawShape(Shape S1, int color){
  int i;
  Point nullP;
  for (i=0; i < Max_Point; i++)
  {
    if(i<3 )
      drawline(S1.p[i],S1.p[i+1],color);
    else
      drawline(S1.p[i],S1.p[0],color);
  }
}

int isInShape(Shape S1,Point P1)
{
  if (P1.x > S1.p[0].x && P1.x<S1.p[2].x && P1.y > S1.p[0].y && P1.y<S1.p[2].y)
    return 1;
  else
    return 0;
}


void fillRect(Shape tempS, int color)
{
  Point tempP1,tempP2;
  int i;
  for (i = tempS.p[0].y; i < tempS.p[2].y; ++i)
  {
    tempP1.x = tempS.p[0].x;
    tempP1.y = i;
    tempP2.x = tempS.p[2].x;
    tempP2.y = i;
    drawline(tempP1,tempP2,color); 
  }
}

void paintline(Point tempP1,Point tempP2)
{
  drawline(tempP1,tempP2,Ccolor);

  AS[num_AL].p[0]= tempP1;
  AS[num_AL].p[1]= tempP2;
  AS[num_AL].color = Ccolor;
  num_AL++;
}

void paintRect(Point tempP1,Point tempP2)
{
	Shape tempS = createShape(tempP1,tempP2);
	tempS.color = Ccolor;
	drawShape(tempS,Ccolor);
	AS[num_AS]= tempS;
	num_AS++;
}

void flood_fill (int x, int y, byte color, byte targetcolor, byte isFirst, int max)
{
  if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
    if (isFirst) {
      targetcolor = VGA[(y*SCREEN_WIDTH)+x-1];
      VGA[(y*SCREEN_WIDTH)+x] = targetcolor;
    }
    if (color != VGA[(y*SCREEN_WIDTH)+x] && max!=4000) {
      if (VGA[(y*SCREEN_WIDTH)+x] == targetcolor) {
        max = max+1;
        putpixel(x,y,color);
        flood_fill(x,y+1,color,targetcolor,0,max+1);
        flood_fill(x,y-1,color,targetcolor,0,max+1);
        flood_fill(x+1,y,color,targetcolor,0,max+1);
        flood_fill(x-1,y,color,targetcolor,0,max+1);
      }
    }
  }
}

void paintFill(Point tempP1) {
  flood_fill (tempP1.x, tempP1.y, Ccolor, 0, 1, 0);
}

void hide_mouse ()
{
  in.x.ax = 2;
  int86 (0X33,&in,&out);
}

void detect_mouse ()
{
  in.x.ax = 0;
  int86 (0X33,&in,&out);   //invoke interrupt
  if (out.x.ax == 0)
    printf ("\nMouse Failed To Initialize");
}

void showmouse ()
{
  in.x.ax = 1;
  int86 (0X33,&in,&out);
}

void zoomin()
{
  int starty, finishy, i, j, x, y;
  //((SCREEN_WIDTH/Max_Shape)+1+(SCREEN_WIDTH/16))

  starty = ((SCREEN_WIDTH/Max_Shape)+2+(SCREEN_WIDTH/16));
  finishy = starty + 74;
  x = 0;
  y = 199;

  for (j = finishy; j > starty + 1; j--)
  {
    for (i = 0; i < 160; i++)
    {
      putpixel(x, y, getcolor(i, j));
      putpixel(x+1, y, getcolor(i, j));
      putpixel(x, y-1, getcolor(i, j));
      putpixel(x+1, y-1, getcolor(i, j));
      x += 2;
    }
    x = 0;
    y -= 2;
  }
}

float bezier1(float t, float p0f, float p1f) {
  return ((1.0f - t) * p0f) + (t * p1f);
}
// Quadratic
float bezier2(float t, float p0f, float p1f, float p2f) {
  float tt = 1.0f - t;
  return (tt * ((tt * p0f) + (t * p1f))) + (t * ((tt * p1f) + (t * p2f)));
}
// Cubic
float bezier3(float t, float p0f, float p1f, float p2f, float p3f) {
  return ((1.0f - t) * bezier2(t, p0f, p1f, p2f)) + (t * bezier2(t, p1f, p2f, p3f));
}
// Curve functions with points
Point bezier1p(float t, Point p0, Point p1) {
  Point pt;
  pt.x = (int) bezier1(t, (float) p0.x, (float) p1.x);
  pt.y = (int) bezier1(t, (float) p0.y, (float) p1.y);

  return pt;
}
Point bezier2p(float t, Point p0, Point p1, Point p2) {
  Point pt;
  pt.x = (int) bezier2(t, (float) p0.x, (float) p1.x, (float) p2.x);
  pt.y = (int) bezier2(t, (float) p0.y, (float) p1.y, (float) p2.y);

  return pt;
}
Point bezier3p(float t, Point p0, Point p1, Point p2, Point p3) {
  Point pt;
  pt.x = (int) bezier3(t, (float) p0.x, (float) p1.x, (float) p2.x, (float) p3.x);
  pt.y = (int) bezier3(t, (float) p0.y, (float) p1.y, (float) p2.y, (float) p3.y);

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
      pt = bezier1p(t, p0, p1);
      break;
    case 3:
      p0 = points[0];
      p1 = points[1];
      p2 = points[2];
      pt = bezier2p(t, p0, p1, p2);
      break;
    case 4:
      p0 = points[0];
      p1 = points[1];
      p2 = points[2];
      p3 = points[3];
      pt = bezier3p(t, p0, p1, p2, p3);
      break;
  }

  return pt;
}
void plot_pixel(Point p, byte color) {
  putpixel(p.x, p.y, color);
}
void plot_bezier(Point* points, int n, byte color) {
  unsigned int granularity = INT_MAX;
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
struct {
  Point points[10];
  int colors[10];
  int count;
} beziermem;
void bezier_tool(Point tempP, int n) {
  // Quadratic bezier
  if (beziermem.count < n) {
    // Capture pixels
    beziermem.points[beziermem.count] = tempP;
    beziermem.colors[beziermem.count] = getcolor(tempP.x, tempP.y);
    plot_pixel(tempP, Ccolor);
    ++beziermem.count;
  }
  if (beziermem.count >= n) {
    // Undraw the control points
    int i;
    for (i=0; i<n; ++i) {
      plot_pixel(beziermem.points[i], beziermem.colors[i]);
    }
    plot_bezier(beziermem.points, beziermem.count, Ccolor);

    // Reset bezier memory state
    beziermem.count = 0;

    // Return to no function
    drawShape(M.s[CFunc],0);
    CFunc = -1;
  }
}

void detect ()
{
  int flag =0;
  int i;
  Point firstP = createPoint(-1,-1);
  Point secondP = createPoint(-1,-1);
  Point emptyP = createPoint(-1,-1);
  int centerx;
  int centery;
  Shape tempS,emptyS;
  Point tempP1,tempP2,tempP3,tempP4,emptyTP;
  double phi = acos(-1);
  while (!kbhit () )
  {
    Point tempP;
    flag = 0;
    i = 0;
    in.x.ax = 3;
    int86 (0X33,&in,&out);
    if (out.x.bx == 1)
    {
      tempP.x = out.x.cx/2;
      tempP.y = out.x.dx;
      hide_mouse();
      if((tempP.y>0) && (tempP.y<SCREEN_WIDTH/Max_Shape))
      {
        while (flag == 0 && i<Max_Shape)
        {
          if(isInShape(M.s[i],tempP)==1)
            flag = 1;
          else
          {
            i++;
          }
        }
        if(flag==1)
        {
          if(CFunc > -1)
          {
          	drawShape(M.s[CFunc],0);
          }
          if(CFunc!=i)
          {
          	firstP = emptyP;
			secondP = emptyP;
          }
          CFunc = i;
          drawShape(M.s[i],14);
          
          flag = 0;
        }
      }
      else if((tempP.y>SCREEN_WIDTH/Max_Shape) && (tempP.y<((SCREEN_WIDTH/Max_Shape)+1+(SCREEN_WIDTH/16)))) 
      {
        while (flag == 0 && i<16)
        {
          if(isInShape(colorP[i],tempP)==1)
            {
            	flag = 1;
            }
          else
          {
            i++;
          }
        }

        if(flag==1)
        {
          drawShape(colorP[Ccolor],0);
          Ccolor=i;
          drawShape(colorP[i],14);
          flag=0;
        }
      }
      else if(tempP.y>((SCREEN_WIDTH/Max_Shape)+1+(SCREEN_WIDTH/16)))
      {
      	if(CFunc>-1)
      	{
      		if(CFunc==0 || CFunc==1)
      		{
      			if(firstP.x != -1)
      			{
      				secondP = tempP;
      				if(CFunc == 0){
      					paintline(firstP,secondP);
      				}
      				else if (CFunc==1)
      				{
      					paintRect(firstP,secondP);
      				}
      				drawShape(M.s[CFunc],0);
      				CFunc = -1;
      				firstP = emptyP;
      				secondP = emptyP;
      			}
      			else
      			{
      				firstP = tempP;
      			}
      		}
      		else if(CFunc==2)
      		{
      			paintFill(tempP);
  				drawShape(M.s[CFunc],0);
      			CFunc = -1;
				firstP = emptyP;
				secondP = emptyP;
      		}
      		else if(CFunc==3)
      		{
  				
			  	while (flag == 0 && i<num_AS)
			 	  {
				    if(isInShape(AS[i],tempP)==1)
				    {
				      flag = 1;
				    }
				    else
			     	{
				       i++;
			      	}
		  		}
			  
			  	if(flag==1)
			   {
			      drawShape(AS[i], 15);
            centerx = min(AS[i].p[0].x,AS[i].p[2].x)+(max(AS[i].p[2].x,AS[i].p[0].x) - min(AS[i].p[0].x,AS[i].p[2].x));
            centery = min(AS[i].p[0].y,AS[i].p[2].y)+(max(AS[i].p[2].y,AS[i].p[0].y) - min(AS[i].p[0].y,AS[i].p[2].y));

			      tempS = AS[i];
            tempS.p[0].x= centerx+(AS[i].p[0].x - centerx)*1.5;
            tempS.p[0].y= centery+(AS[i].p[0].y - centery)*1.5;
            tempS.p[1].x= centerx+(AS[i].p[1].x - centerx)*1.5;
            tempS.p[1].y= centery+(AS[i].p[1].y - centery)*1.5;
            tempS.p[2].x= centerx+(AS[i].p[2].x - centerx)*1.5;
            tempS.p[2].y= centery+(AS[i].p[2].y - centery)*1.5;
            tempS.p[3].x= centerx+(AS[i].p[3].x - centerx)*1.5;
            tempS.p[3].y= centery+(AS[i].p[3].y - centery)*1.5;
			      tempS.color = AS[i].color;
			      drawShape(tempS, tempS.color);
			      AS[i]= tempS;    
			      tempS = emptyS; 
			      tempP1 = emptyTP;
			      tempP2 = emptyTP;
  				  drawShape(M.s[CFunc],0);
			      CFunc = -1;
  				  firstP = emptyP;
  				  secondP = emptyP;  
			    }
	      		
     		}
      		else if(CFunc==4)
      		{
      			while (flag == 0 && i<num_AS)
			 	{
				    if(isInShape(AS[i],tempP)==1)
				    {
				      flag = 1;
				    }
				    else
			     	{
				       i++;
			      	}
		  		}
			  
			  	if(flag==1)
			   	{
			      drawShape(AS[i], 15);
			      centerx = AS[i].p[0].x + (AS[i].p[2].x - AS[i].p[0].x);
			      centery = AS[i].p[0].y + (AS[i].p[2].y - AS[i].p[0].y);
				  tempP1.x  = (int)floor(((double)(AS[i].p[0].x-centerx)*(double)cos(45*phi/180)-(double)(AS[i].p[0].y-centery)*(double)sin(45*phi/180))+(double)centerx+ 0.5);
				  tempP1.y = (int)floor(((double)(AS[i].p[0].y-centery)*(double)cos(45*phi/180)+(double)(AS[i].p[0].x-centerx)*(double)sin(45*phi/180))+(double)centery+ 0.5);
				  tempP2.x = (int)floor(((double)(AS[i].p[1].x-centerx)*(double)cos(45*phi/180)-(double)(AS[i].p[1].y-centery)*(double)sin(45*phi/180))+(double)centerx+ 0.5);
				  tempP2.y = (int)floor(((double)(AS[i].p[1].y-centery)*(double)cos(45*phi/180)+(double)(AS[i].p[1].x-centerx)*(double)sin(45*phi/180))+(double)centery+ 0.5);
  				  tempP3.x  = (int)floor(((double)(AS[i].p[2].x-centerx)*(double)cos(45*phi/180)-(double)(AS[i].p[2].y-centery)*(double)sin(45*phi/180))+(double)centerx+ 0.5);
				  tempP3.y = (int)floor(((double)(AS[i].p[2].y-centery)*(double)cos(45*phi/180)+(double)(AS[i].p[2].x-centerx)*(double)sin(45*phi/180))+(double)centery+ 0.5);
				  tempP4.x = (int)floor(((double)(AS[i].p[3].x-centerx)*(double)cos(45*phi/180)-(double)(AS[i].p[3].y-centery)*(double)sin(45*phi/180))+(double)centerx+ 0.5);
				  tempP4.y = (int)floor(((double)(AS[i].p[3].y-centery)*(double)cos(45*phi/180)+(double)(AS[i].p[3].x-centerx)*(double)sin(45*phi/180))+(double)centery+ 0.5);

				  tempS.p[0] = tempP1;
				  tempS.p[1] = tempP2;
				  tempS.p[2] = tempP3;
				  tempS.p[3] = tempP4;
			      tempS.color = AS[i].color;
			      drawShape(tempS, 0);
			      AS[i]= tempS;    
			      tempS = emptyS; 
			      tempP1 = emptyTP;
			      tempP2 = emptyTP;
  				  drawShape(M.s[CFunc],0);  
			      CFunc = -1;
  				  firstP = emptyP;
  				  secondP = emptyP;
		    	}
	      		
      		}
          else if(CFunc==5)
          {
            zoomin();
            drawShape(M.s[CFunc],0);  
            CFunc = -1;
          }
          else if (CFunc==6)
          {
            bezier_tool(tempP, 3);
          }
          else if (CFunc==7)
          {
            // Cubic bezier
            bezier_tool(tempP, 4);
          }
    	}
      }
      showmouse();
    }
    delay (120); // Otherwise due to quick computer response 100s of words will get print
  }
}

void load_bmp(char *file,BITMAP *b)
{
  FILE *fp;
  long index;
  word num_colors;
  int x;

  /* open the file */
  if ((fp = fopen(file,"rb")) == NULL)
  {
    printf("Error opening file %s.\n",file);
    exit(1);
  }

  /* check to see if it is a valid bitmap file */
  if (fgetc(fp)!='B' || fgetc(fp)!='M')
  {
    fclose(fp);
    printf("%s is not a bitmap file.\n",file);
    exit(1);
  }

  /* read in the width and height of the image, and the
     number of colors used; ignore the rest */
  fskip(fp,16);
  fread(&b->width, sizeof(word), 1, fp);
  fskip(fp,2);
  fread(&b->height,sizeof(word), 1, fp);
  fskip(fp,22);
  fread(&num_colors,sizeof(word), 1, fp);
  fskip(fp,6);

  /* assume we are working with an 8-bit file */
  if (num_colors==0) num_colors=256;


  /* try to allocate memory */
  if ((b->data = (byte *) malloc((word)(b->width*b->height))) == NULL)
  {
    fclose(fp);
    printf("Error allocating memory for file %s.\n",file);
    exit(1);
  }

  /* Ignore the palette information for now.
     See palette.c for code to read the palette info. */
  fskip(fp,num_colors*4);

  /* read the bitmap */
  for(index=(b->height-1)*b->width;index>=0;index-=b->width)
    for(x=0;x<b->width;x++)
      b->data[(word)index+x]=(byte)fgetc(fp);

  fclose(fp);
}

void draw_bitmap(BITMAP *bmp,int x,int y)
{
  int j;
  word screen_offset = (y<<8)+(y<<6)+x;
  word bitmap_offset = 0;

  for(j=0;j<bmp->height;j++)
  {
    memcpy(&VGA[screen_offset],&bmp->data[bitmap_offset],bmp->width);

    bitmap_offset+=bmp->width;
    screen_offset+=SCREEN_WIDTH;
  }
}

int main ()
{
  BITMAP bmp;
  int i,j,x,y;
  Point tempP1,tempP2;
  set_mode(VGA_256_COLOR_MODE); 

  for (i = 0; i < SCREEN_HEIGHT; ++i)
  {
    tempP1.x = 0;
    tempP1.y = i;
    tempP2.x = SCREEN_WIDTH;
    tempP2.y = i;
    drawline(tempP1,tempP2,15); 
  }
  
  //draw_bitmap(&bmp,100,100);
  for(i=0;i<=7;i++)
  {
    switch (i)
    {
    case 0:
      load_bmp("line.bmp", &bmp);
      break;
    case 1:
      load_bmp("rect.bmp", &bmp);
      break;
    case 2:
      load_bmp("paint.bmp", &bmp);
      break;
    case 3:
      load_bmp("resize.bmp", &bmp);
      break;
    case 4:
      load_bmp("rotate.bmp", &bmp);
      break;
    case 5:
      load_bmp("zoom.bmp", &bmp);
      break;
    case 6:
      load_bmp("bezier2.bmp", &bmp);
      break;
    case 7:
      load_bmp("bezier3.bmp", &bmp);
      break;
    default:
      break;
    }
	for(y=0;y<32;y++)
		for(x=0;x<32;x++)
			VGA [(32*i)+x+y*SCREEN_WIDTH]=bmp.data[x+y*32];
  }

  for (i = 0; i < Max_Shape;++i)
  {
    M.s[i]=createShape(createPoint(i*SCREEN_WIDTH/Max_Shape,0),createPoint((i+1)*SCREEN_WIDTH/Max_Shape,SCREEN_WIDTH/Max_Shape));
    drawShape(M.s[i],0);
  }

  for (i = 0; i < 16; ++i)
  {
    colorP[i]=createShape(createPoint(i*SCREEN_WIDTH/16,1+SCREEN_WIDTH/Max_Shape),createPoint((i+1)*SCREEN_WIDTH/16,(SCREEN_WIDTH/Max_Shape)+1+(SCREEN_WIDTH/16)));
    fillRect(colorP[i],i);
    drawShape(colorP[i],0);
  }
  drawShape(colorP[Ccolor],14);

  detect_mouse ();
  showmouse ();
  detect ();
  hide_mouse ();
  getch ();
  set_mode(TEXT_MODE);
  free(bmp.data);
  return 0;
}