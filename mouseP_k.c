#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
//#include <graphics.h>

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
}Shape;

typedef struct 
{
  Shape s[Max_Shape];
}menu;

typedef struct
{
  Point p[2];
}Line;

typedef struct tBitmap
{
    word width;
    word height;
    byte *data;
} BITMAP;

menu M;
Line AL[99];
Shape AS[99];
Shape colorP[9];
int Ccolor=0;

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
    if(i<3 && S1.p[i+1].x != nullP.x && S1.p[i+1].y != nullP.y )
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

void paintline()
{
  int flag = 0;
  Point tempP1, tempP2;

  while (!kbhit () && flag ==0)
  {
    in.x.ax = 3;
    int86 (0X33,&in,&out);
    if (out.x.bx == 1)
    {
      
      tempP1.x = out.x.cx/2;
      tempP1.y = out.x.dx;
      flag = 1;
    }
    delay (100);
  } 

  flag =0;
  while (!kbhit () && flag ==0)
  {
    in.x.ax = 3;
    int86 (0X33,&in,&out);
    if (out.x.bx == 1)
    {
      tempP2.x = out.x.cx/2;
      tempP2.y = out.x.dx;
      flag = 1;
    }
    
    delay (100); // Otherwise due to quick computer response 100s of words will get print
  }

  drawline(tempP1,tempP2,Ccolor);

  AS[num_AL].p[0]= tempP1;
  AS[num_AL].p[1]= tempP2;

  num_AL++;
}

void paintRect()
{
  int flag = 0;
  Point tempP1, tempP2;
  Shape tempS;

  while (!kbhit () && flag ==0)
  {
    in.x.ax = 3;
    int86 (0X33,&in,&out);
    if (out.x.bx == 1)
    {
      
      tempP1.x = out.x.cx/2;
      tempP1.y = out.x.dx;
      flag = 1;
    }
    delay (100);
  } 

  flag =0;
  while (!kbhit () && flag ==0)
  {
    in.x.ax = 3;
    int86 (0X33,&in,&out);
    if (out.x.bx == 1)
    {
      tempP2.x = out.x.cx/2;
      tempP2.y = out.x.dx;
      flag = 1;
    }
    
    delay (100); // Otherwise due to quick computer response 100s of words will get print
  }

  tempS = createShape(tempP1,tempP2);

  drawShape(tempS,Ccolor);
  AS[num_AS]= tempS;
  num_AS++;
}


void detect ()
{

  int flag =0;
  int flag1 =0;
  int i,j;

  while (!kbhit () )
  {
    Point tempP;
    i = 0;
    j = 0;
    in.x.ax = 3;
    int86 (0X33,&in,&out);
    if (out.x.bx == 1)
    {
      
      tempP.x = out.x.cx/2;
      tempP.y = out.x.dx;
      delay (100);
      if(tempP.y<SCREEN_WIDTH/Max_Shape)
      {
        while (flag == 0 && i<Max_Shape)
        {
          if(isInShape(M.s[i],tempP)==1)
            flag = 1;
          else
            i++;
        }
        if(flag==1)
        {
          switch (i)
          {
            case 0 :  paintline();
                      break;
            case 1 :  paintRect();
                      break;
            default : break;
          }

          flag = 0;
        }
      }
      else
      {
        while (flag1 == 0 && j<16)
        {
          if(isInShape(colorP[j],tempP)==1)
            flag1 = 1;
          else
            j++;
        }

        if(flag1==1)
        {
          Ccolor=j;
          flag1=0;
        }
      }
    }
    delay (100); // Otherwise due to quick computer response 100s of words will get print
  }
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

void showmouse_text ()
{
  in.x.ax = 1;
  int86 (0X33,&in,&out);
}

void load_bmp(char *file, BITMAP *b)
{
    FILE *fp;
    long index;
    word num_colors;
    int x;

    /* open the file */
    if ((fp = fopen(file, "rb")) == NULL)
    {
        printf("Error opening file %s.\n", file);
        exit(1);
    }

    /* check to see if it is a valid bitmap file */
    if (fgetc(fp) != 'B' || fgetc(fp) != 'M')
    {
        fclose(fp);
        printf("%s is not a bitmap file.\n", file);
        exit(1);
    }

    /* read in the width and height of the image, and the
    number of colors used; ignore the rest */
    fseek(fp, 16, SEEK_CUR);
    fread(&b->width, sizeof (word), 1, fp);
    fseek(fp, 2, SEEK_CUR);
    fread(&b->height, sizeof (word), 1, fp);
    fseek(fp, 22, SEEK_CUR);
    fread(&num_colors, sizeof (word), 1, fp);
    fseek(fp, 6, SEEK_CUR);

    /* assume we are working with an 8-bit file */
    if (num_colors == 0) num_colors = 256;


    /* try to allocate memory */
    if ((b->data = (byte *) malloc((word) (b->width * b->height))) == NULL)
    {
        fclose(fp);
        printf("Error allocating memory for file %s.\n", file);
        exit(1);
    }

    /* Ignore the palette information. Using standard pallete. */
    fseek(fp, num_colors * 4, SEEK_CUR);

    /* read the bitmap */
    for (index = (b->height - 1) * b->width; index >= 0; index -= b->width)
        for (x = 0; x < b->width; x++)
            b->data[(word) index + x] = (byte) fgetc(fp);

    fclose(fp);
}

void plot_bitmap(BITMAP *bmp, int x, int y)
{
    int j;
    word screen_offset = y*SCREEN_WIDTH+x ;
    word bitmap_offset = 0;

    for (j = 0; j < bmp->height; j++)
    {
        //memcpy(VGA + screen_offset, &bmp->data[bitmap_offset], bmp->width);
        bitmap_offset += bmp->width;
        screen_offset += SCREEN_WIDTH;
    }
}


int main ()
{

  int i,j;
  Point tempP1,tempP2;
  BITMAP bmp;
  
  set_mode(VGA_256_COLOR_MODE); 

  for (i = 0; i < SCREEN_HEIGHT; ++i)
  {
    tempP1.x = 0;
    tempP1.y = i;
    tempP2.x = SCREEN_WIDTH;
    tempP2.y = i;
    drawline(tempP1,tempP2,15); 
  }

  for (i = 0; i < Max_Shape;++i)
  {
    M.s[i]=createShape(createPoint(i*SCREEN_WIDTH/Max_Shape,0),createPoint((i+1)*SCREEN_WIDTH/Max_Shape,SCREEN_WIDTH/Max_Shape));
    drawShape(M.s[i],0);
  /*
  for (i = 0; i < Max_Shape; ++i)
	{
		switch (i)
		{
		case 1:
			load_bmp("line.bmp", &bmp);
			break;
		case 2:
			load_bmp("rectangle.bmp", &bmp);
			break;
		case 3:
			load_bmp("paint.bmp", &bmp);
			break;
		case 4:
			load_bmp("resize.bmp", &bmp);
			break;
		case 5:
			load_bmp("rotate.bmp", &bmp);
			break;
		default:
			break;
		}

		if ((i % 8) == 0 && i != 0)
		{
			y1 = 7;
			x1 += 24;
		}
		else if (i != 0)
		{
			y1 += 24;
		}
		plot_bitmap(&bmp, x1, y1);
		free(bmp.data);
	}
	*/
	}

  for (i = 0; i < 16; ++i)
  {
    colorP[i]=createShape(createPoint(i*SCREEN_WIDTH/16,1+SCREEN_WIDTH/Max_Shape),createPoint((i+1)*SCREEN_WIDTH/16,(SCREEN_WIDTH/Max_Shape)+1+(SCREEN_WIDTH/16)));
    fillRect(colorP[i],i);
    drawShape(colorP[i],0);
  }

  detect_mouse ();
  showmouse_text ();
  detect ();
  hide_mouse ();
  getch ();

  set_mode(TEXT_MODE);
  return 0;
}

