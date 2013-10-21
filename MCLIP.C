#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
//#include <graphics.h>
//#include "POINT.H"
//#include "LINE.H"

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

typedef unsigned char  byte;
typedef unsigned short word;

byte far *VGA=(byte far*)0xA0000000L;        /* this points to video memory. */

typedef struct 
{
  int x, y;
}Point;

Point p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14;


union REGS in, out;

void detect_mouse ()
{
	in.x.ax = 0;
	int86 (0X33,&in,&out);   //invoke interrupt
	if (out.x.ax == 0)
		printf ("\nMouse Failed To Initialize");
	//else
		//printf ("\nMouse was Succesfully Initialized");
}

void showmouse_text ()
{
	in.x.ax = 1;
	int86 (0X33,&in,&out);
}

void hide_mouse ()
{
	in.x.ax = 2;
	int86 (0X33,&in,&out);
}



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

void drawlinerainbow(int x1, int y1, int x2, int y2)
{
  int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;

  dx=x2-x1;      /* the horizontal distance of the line */
  dy=y2-y1;      /* the vertical distance of the line */
  dxabs=abs(dx);
  dyabs=abs(dy);
  sdx=signum(dx);
  sdy=signum(dy);
  x=dyabs>>1;
  y=dxabs>>1;
  px=x1;
  py=y1;

  putpixel(px,py,4);

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

      if (i%20<5)
      	putpixel(px,py,4);
    	else if (i%20<10)
    		putpixel(px,py,14);
    	else if (i%20<15)
    		putpixel(px,py,2);
    	else
    		putpixel(px,py,1);
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
			
			if (i%20<5)
      	putpixel(px,py,4);
    	else if (i%20<10)
    		putpixel(px,py,14);
    	else if (i%20<15)
    		putpixel(px,py,2);
    	else
    		putpixel(px,py,1);    
    }
  }
}

// void generateequation(Line l)
// {
//   l.m = (float) l.b.x / l.a.x;
//   printf ("%f", l.m);
//   l.m = (l.b.y - l.a.y) / (l.b.x - l.a.x);
//   l.c = ((l.b.x * l.a.y) - (l.a.x * l.b.y)) / (l.b.x - l.a.x);
//   getch();
// }

// void sortpointx(Point* p, int n)
// {
//   int i;
//   int j;
//   Point temp;

//   for(i = 0; i < n; i++)
//   {
//     for(j = i; j < n; j++)
//     {
//       if (p[j].x > p[j+1].x)
//       {
//         temp = p[j];
//         p[j] = p[j+1];
//         p[j+1] = temp;
//       }
//     }
//   }
// }

void fill(int x, int y, int color)
{
  putpixel(x, y, color);
  if (getcolor(x-1, y) == 0)
  {
    fill(x-1, y, color);
  }
  if (getcolor(x+1, y) == 0)
  {
    fill(x+1, y, color);
  }
  if (getcolor(x, y-1) == 0)
  {
    fill(x, y-1, color);
  }
  if (getcolor(x, y+1) == 0)
  {
    fill(x, y+1, color);
  }
}

// void scanline(Line * lines, int n, int y, int color)
// {
//   int p;
//   int i;
//   int x;
//   Point * points = malloc(n);

//   p = 0;

//   for (i = 0; i < n; i++)
//   {
//     if (((lines[i].a.y - y) * (lines[i].b.y - y)) < 0)
//     {
//       generateequation(lines[i]);
//       getch();
//       x = (y - lines[i].c) / lines[i].m;
//       if (getcolor(x,y) == LINE_COLOR)
//       {}
//       else
//       {
//         x++;
//       }
//       points[p].x = x;
//       points[p].y = y;
//       p++;
//     }
//   }

//   sortpointx(points, p);

//   for (i = 0; i < p; i+=2)
//   {
//     drawline(points[i], points[i+1], color);
//   }

//   return;
// }

void scanline(int y, int n, int color)
{
  int x, i;
  byte mode;
  Point * temppoints;
  Point * points;
  byte top, bottom;

  i = 0;
  mode = 0;
  temppoints = malloc(n);
  top = 0;
  bottom = 0;
  for (x = 0; x < SCREEN_WIDTH; x++)
  {
    if (mode == 0)
    {

      if (getcolor(x,y) == LINE_COLOR)
      {
        // pengecekan garis lurus atau menekuk
        if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
        {
          top++;
        }
        if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
        {
          bottom++;
        }

        // pengecekan garis berlanjut pada sumbu x atau tidak
        if (getcolor(x+1,y) == LINE_COLOR)
        {
          // mencari titik ujung dari garis yang berlanjut pada sumbu x
          while (getcolor(x+1,y) == LINE_COLOR)
          {
            x++;
          }
          // mengecek lagi garis lurus atau menekuk
          // if (top == 0)
          // {
            if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
            {
              top++;
            }
          // }
          // else if (bottom == 0)
          // {
            if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
            {
              bottom++;
            }
          // }
        }

        // pengecekan apakah titik merupakan titik potong atau tidak
        if (top == 1 && bottom == 1)
        {
          temppoints[i].x = x;
          temppoints[i].y = y;
          i++;
          mode = 1;
          top = 0;
          bottom = 0;
        }
        else
        {
          top = 0;
          bottom = 0;
        }
      }
    }
    else if (mode == 1)
    {
      if (getcolor(x,y) ==  0)
      {
        putpixel(x,y,color);
      }
      else
      {
        temppoints[i].x = x;
        temppoints[i].y = y;
        i++;

        // pengecekan garis lurus atau menekuk
        if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
        {
          top++;
        }
        if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
        {
          bottom++;
        }

        // pengecekan garis berlanjut pada sumbu x atau tidak
        if (getcolor(x+1,y) == LINE_COLOR)
        {
          // mencari titik ujung dari garis yang berlanjut pada sumbu x
          while (getcolor(x+1,y) == LINE_COLOR)
          {
            x++;
          }

          temppoints[i].x = x;
          temppoints[i].y = y;
          i++;

          // mengecek lagi garis lurus atau menekuk
          // if (top == 0)
          // {
            if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
            {
              top++;
            }
          // }
          // else if (bottom == 0)
          // {
            if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
            {
              bottom++;
            }
          // }
        }

        // pengecekan apakah mode pewarnaan berlanjut atau tidak
        if (top == 1 && bottom == 1)
        {
          mode = 0;
          top = 0;
          bottom = 0;
        }
        else
        {
          top = 0;
          bottom = 0;
        }
      }
    }
  }

  points = malloc(i-1);

  for (i = 0; i < SIZE(points); i++)
  {
    points[i] = temppoints[i];
  }
}

void scanlinepattern (int y, int n, byte * pattern)
{
  int x, i;
  byte mode;
  Point * temppoints;
  Point * points;
  byte top, bottom;

  i = 0;
  mode = 0;
  temppoints = malloc(n);
  top = 0;
  bottom = 0;
  for (x = 0; x < SCREEN_WIDTH; x++)
  {
    if (mode == 0)
    {

      if (getcolor(x,y) == LINE_COLOR)
      {
        // pengecekan garis lurus atau menekuk
        if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
        {
          top++;
        }
        if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
        {
          bottom++;
        }

        // pengecekan garis berlanjut pada sumbu x atau tidak
        if (getcolor(x+1,y) == LINE_COLOR)
        {
          // mencari titik ujung dari garis yang berlanjut pada sumbu x
          while (getcolor(x+1,y) == LINE_COLOR)
          {
            x++;
          }
          // mengecek lagi garis lurus atau menekuk
          // if (top == 0)
          // {
            if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
            {
              top++;
            }
          // }
          // else if (bottom == 0)
          // {
            if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
            {
              bottom++;
            }
          // }
        }

        // pengecekan apakah titik merupakan titik potong atau tidak
        if (top == 1 && bottom == 1)
        {
          temppoints[i].x = x;
          temppoints[i].y = y;
          i++;
          mode = 1;
          top = 0;
          bottom = 0;
        }
        else
        {
          top = 0;
          bottom = 0;
        }
      }
    }
    else if (mode == 1)
    {
      if (getcolor(x,y) ==  0)
      {
        putpixel(x,y,pattern[x]);
      }
      else
      {
        temppoints[i].x = x;
        temppoints[i].y = y;
        i++;

        // pengecekan garis lurus atau menekuk
        if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
        {
          top++;
        }
        if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
        {
          bottom++;
        }

        // pengecekan garis berlanjut pada sumbu x atau tidak
        if (getcolor(x+1,y) == LINE_COLOR)
        {
          // mencari titik ujung dari garis yang berlanjut pada sumbu x
          while (getcolor(x+1,y) == LINE_COLOR)
          {
            x++;
          }

          temppoints[i].x = x;
          temppoints[i].y = y;
          i++;

          // mengecek lagi garis lurus atau menekuk
          // if (top == 0)
          // {
            if (getcolor(x-1,y-1) == LINE_COLOR || getcolor(x,y-1) == LINE_COLOR || getcolor(x+1,y-1) == LINE_COLOR)
            {
              top++;
            }
          // }
          // else if (bottom == 0)
          // {
            if (getcolor(x-1,y+1) == LINE_COLOR || getcolor(x,y+1) == LINE_COLOR || getcolor(x+1,y+1) == LINE_COLOR)
            {
              bottom++;
            }
          // }
        }

        // pengecekan apakah mode pewarnaan berlanjut atau tidak
        if (top == 1 && bottom == 1)
        {
          mode = 0;
          top = 0;
          bottom = 0;
        }
        else
        {
          top = 0;
          bottom = 0;
        }
      }
    }
  }

  points = malloc(i-1);

  for (i = 0; i < SIZE(points); i++)
  {
    points[i] = temppoints[i];
  }
}

byte* makesimplepattern(int x)
{
  int i;
  byte * temp;

  temp = malloc(SCREEN_WIDTH);

  if (x%8 == 0 || x%8 == 3)
  {
    for (i = 0; i < SCREEN_WIDTH; i++)
    {
      if (i%16 == 1 || i%16 == 2)
      {
        temp[i] = 12;
      }
      else
      {
        temp[i] = 1;
      }
    }
  }
  else if (x%8 == 1 || x%8 == 2)
  {
    for (i = 0; i < SCREEN_WIDTH; i++)
    {
      if (i%16 == 0 || i%16 == 1 || i%16 == 2 || i%16 == 3)
      {
        temp[i] = 12;
      }
      else
      {
        temp[i] = 1;
      }
    }
  }
  else if (x%8 == 4 || x%8 == 7)
  {
    for (i = 0; i < SCREEN_WIDTH; i++)
    {
      if (i%16 == 9 || i%16 == 10)
      {
        temp[i] = 12;
      }
      else
      {
        temp[i] = 1;
      }
    }
  }
  else if (x%8 == 5 || x%8 == 6)
  {
    for (i = 0; i < SCREEN_WIDTH; i++)
    {
      if (i%16 == 8 || i%16 == 9 || i%16 == 10 || i%16 == 11)
      {
        temp[i] = 12;
      }
      else
      {
        temp[i] = 1;
      }
    }
  }
  return temp;
}

void binaryclipping (Point b1, Point b2, Point p1, Point p2, int pcolor)
{
  int left, right, top, bottom, x1, x2, i, j, k;
  Point * points;
  int found;

  left = b1.x;
  top = b1.y;
  right = b2.x;
  bottom = b2.y;
  x1 = 0;
  x2 = 0;

  if (p1.x < left)        x1 += 8; 
  else if (p1.x > right)  x1 += 4;

  if (p1.y < top)         x1 += 2; 
  else if (p1.y > bottom) x1 += 1;
  
  if (p1.x >= left && p1.x <= right && p1.y >= top && p1.y <= bottom) x1=0;

  if (p2.x < left)        x2 += 8; 
  else if (p2.x > right)  x2 += 4;

  if (p2.y < top)         x2 += 2; 
  else if (p2.y > bottom) x2 += 1;

  if (p2.x >= left && p2.x <= right && p2.y >= top && p2.y <= bottom) x2=0;

  if ((x1 & x2) == 0)
  {

  	drawline(p1, p2, pcolor);	
    points = malloc(2);
    j = 0;
    	found = 0;

		i = left;
    	while (found == 0 && i <= right)
    	{
    		k = top;
    		while (found == 0 && k <= bottom)
    		{
    			if (getcolor(i, k) == pcolor)
				{
					points[j].x = i;
					points[j].y = k;
					found = 1;
					j++;
				}
    			k++;
    		}
    		i++;
    	}
    	
    	found = 0;
    	i = right;
    	while (found == 0 && i >= left)
    	{
    		k = bottom;
    		while (found == 0 && k >= top)
    		{
    			if (getcolor(i, k) == pcolor)
			      {
			        points[j].x = i;
			        points[j].y = k;
			        found = 1;
			        j++;
			      }
    			k--;
    		}
    		i--;
    	}
    

    if (j == 2)
    {
	  set_mode(VGA_256_COLOR_MODE);
      drawline(points[0], points[1], 2);
    }
  }
}

void detect ()
{
	int button;
	while (!kbhit () )
	{
		in.x.ax = 3;
		int86 (0X33,&in,&out);
		button=out.x.bx&7;
		switch(button)
		{
			case 1:
  				set_mode(VGA_256_COLOR_MODE);
				p12.y -= 1;
				p10.y-= 1;
				p9.y-= 1;
				p11.y-= 1;
				binaryclipping(p12, p10, p1, p2, 1);
				drawline(p9, p10, LINE_COLOR);
				drawline(p10, p11, LINE_COLOR);
				drawline(p11, p12, LINE_COLOR);
				drawline(p12, p9, LINE_COLOR);
			break;
			case 2:
  				set_mode(VGA_256_COLOR_MODE);
				p12.y+= 1;
				p10.y+= 1;
				p9.y+= 1;
				p11.y+= 1;
				
				binaryclipping(p12, p10, p1, p2, 1);
				drawline(p9, p10, LINE_COLOR);
				drawline(p10, p11, LINE_COLOR);
				drawline(p11, p12, LINE_COLOR);
				drawline(p12, p9, LINE_COLOR);
			break;
			case 4:
				printf("middle button pressed\n");
			break;
		}

	  
		delay (100); // Otherwise due to quick computer response 100s of words will get print
	}
}

void main()
{
  int x,y,color,i,j;
  float t1,t2;
  
  /*//Line * lines;
  byte ** simplepattern;

  // Tugas 1 : Menggambar garis warna-warni
  set_mode(VGA_256_COLOR_MODE);

  drawlinerainbow(10, 20, 100, 100);

  getch();

  // Tugas 2 : Scanline fill
  set_mode(VGA_256_COLOR_MODE);

  simplepattern = malloc(8);
  for (i = 0; i < 8 ; i++)
  {
    simplepattern[i] = malloc(SCREEN_WIDTH);
    simplepattern[i] = makesimplepattern(i);
  }*/

  p1.x = 30;
  p1.y = 10;
  p2.x = 50;
  p2.y = 10;
  p3.x = 50;
  p3.y = 15;
  p4.x = 70;
  p4.y = 15;
  p5.x = 70;
  p5.y = 10;
  p6.x = 90;
  p6.y = 10;
  p7.x = 110;
  p7.y = 30;
  p8.x = 95;
  p8.y = 45;
  p9.x = 90;
  p9.y = 40;
  p10.x = 90;
  p10.y = 100;
  p11.x = 30;
  p11.y = 100;
  p12.x = 30;
  p12.y = 40;
  p13.x = 25;
  p13.y = 45;
  p14.x = 10;
  p14.y = 30;

  /*drawline(p1, p2, LINE_COLOR);
  drawline(p2, p3, LINE_COLOR);
  drawline(p3, p4, LINE_COLOR);
  drawline(p4, p5, LINE_COLOR);
  drawline(p5, p6, LINE_COLOR);
  drawline(p6, p7, LINE_COLOR);
  drawline(p7, p8, LINE_COLOR);
  drawline(p8, p9, LINE_COLOR);
  drawline(p9, p10, LINE_COLOR);
  drawline(p10, p11, LINE_COLOR);
  drawline(p11, p12, LINE_COLOR);
  drawline(p12, p13, LINE_COLOR);
  drawline(p13, p14, LINE_COLOR);
  drawline(p14, p1, LINE_COLOR);

  for (i = 0; i < SCREEN_HEIGHT; i++)
  {
    scanlinepattern(i, 14, simplepattern[i%8]);

    // scanline(i, SIZE(lines), FILL_COLOR);
  }
  
  getch();*/

  // Tugas 3 : Binary Clipping
  set_mode(VGA_256_COLOR_MODE);

  p1.x = 50;
  p1.y = 20;
  p2.x = 80;
  p2.y = 120;

  binaryclipping(p12, p10, p1, p2, 1);

	drawline(p9, p10, LINE_COLOR);
	drawline(p10, p11, LINE_COLOR);
	drawline(p11, p12, LINE_COLOR);
	drawline(p12, p9, LINE_COLOR);

  detect_mouse ();
  detect ();
  getch ();

  set_mode(TEXT_MODE);

  return;
}