// ����������� ������������� �����, ���������� configure
#include <defines.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "options.hpp"

#define MaxString 255

#ifdef MAIN_

using namespace std;

#include "opt.hpp"

unsigned char NoCover = 0, exflag = 0;

#if HAVE_STRUCT_DECL_IN_TEMPL_FUNC
 unsigned char Convert = 0;
#endif

long aver = 1, smooth = 1, le = 0, si = -1, iter, block = -1;
int razm, next, ac, header = 0;
float maxglob, minglob, *m, Shift = 0.;
char *type, **av;
char *OutputPath;
// ���� ������ ����� ������, �� ���������� �� ��������������� ��� ���
// �� ��������� - ���
bool Seq = false;
bool Cycle = false; // ������ � ������ Seq ��������� ������������� ����� ����������
                    // �������� � ���������� �� ������
// � ������, ���� Seq = true, �� cur_file - ��� ����� ��������
// ������������� �����
long cur_file = 0;
long timint = 5000; // ��� �������� ������� (� ��) ��� ��������� �������
                    // ������� ���������, ��������� ���� ��� ���
long slide_time = -1; // �������� ������� (� ��) ��� �����-���
long Group = 1;       // ���������� ������ ��� �����������, ���� ������ ����� --seq
bool Post = false;    // ������� ����� � postscript-����, ���� true

/*-- ���� �������� ����� ��� ������ ����. ���������� --*/
char * XBaseResourcesFile;

/*-- ��� ����� �������� ��� ������� ����������� ��� See
     �� ���� ������������ ���� �������� XBaseResourcesFile
--*/
char * XResourceFile;

/*---- regard.cpp ----*/

void Plotting ( void );
void ReadResources (int&, char**&);

/*----- mem.cpp -----*/

unsigned long getAvailableMemory ( void );

#endif

#ifdef XW_

/*--- ���������� ��������� ����������/���������� �������� ����������� ---*/
#define zoom 100
#define MAX_COLOR_NUMBER 256

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/ShellP.h>

#include "ps.hpp"
#include "gifsave.h"

typedef struct { Pixel fg, bg;
                 Font  font;   } wD, *twData;

// ��������
XtAppContext appcont;
// �������
Display *DisId;
// �������� ������
Widget coreW;

postscript ps; // ps-file

extern char * XBaseResourcesFile, * XResourceFile;      /*-- ����� ������ �������� ��� ������� --*/

extern float maxglob, minglob, *m, Shift;
extern char **av, *type;
extern char * OutputPath;
extern long le, si, aver, smooth, iter, block;
extern int ac, razm, next;
extern unsigned char NoCover, exflag;
extern bool Seq, Cycle;
extern long cur_file;
extern bool Post;
extern int header;

#if HAVE_STRUCT_DECL_IN_TEMPL_FUNC
 extern unsigned char Convert;
#endif

float max[zoom], min[zoom];
float cur_x = 0., cur_y = 0.;

// ������������� ���/���� �������������� ��������� ������� Expose
// ��� ���������� Backing Store
// �� ��������� - �������������
// ���������� LCtrl + RCtrl
// ����������� - LCtrl ��� RCtrl
bool auto_expose = true;

short int *keep, **keepcur;

long size, rate, left, window, lo, okno, min_okno = 2, bll = -1, blr = -1;
long lp[zoom], wp[zoom], rp[zoom];
int file, count = 0, tab = 0, noread = 0;

extern long timint;
unsigned long *mtstatus;     // ������ ������ ��������� ������ (modification time)
extern long slide_time;
bool dir = true;            // ������������� �������������� ����������� ���������� ������
                            // � ������, ����� Cycle = false.
			    // ���� dir = true, �� ������
extern long Group;			    

unsigned int fow, foa, fod;

#define TYPE(i,s,t,lg,wr) { s t *m1; \
                            m1 = new s t[block]; \
                            if ( !m1 ) { printf("Not enough memory!\n"); exit(1); } \
                            Read(m1, m, lg, wr, file, av[i]); \
			    delete (m1); }
			    
/*----- util.cpp -----*/			    
void UpdateSeeXResources (char *, char *);

/*----- see.cpp -----*/

long average   ( float *, long );
long smoothing ( float *, long );
char * glue    (  char *,  int );
char * SelectFileName ( char * );

Dimension pw, ph;

/*---------- Borrowed from Kirill & Kazman |  Updated by me ------------------*/

#define INIT_WIDGETS_PROPS \
   wD wData; \
\
  Dimension w, h; \
  XtResource tRes[] = { \
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel), XtOffset(twData, fg), XtRString, (void*)"black"}, \
    {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel), XtOffset(twData, bg), XtRString, (void*)"white"}, \
    {XtNfont, XtCFont, XtRFont, sizeof(Font), XtOffset(twData, font), XtRString, (void*)"6x10"}}; \
\
  XtGetApplicationResources(targetW, &wData, tRes, XtNumber(tRes), NULL, 0); \
\
  Display *disID = XtDisplay(targetW); \
  Window winID = XtWindow(targetW); \
  GC     grC   = XtGetGC(targetW, 0, NULL); \
  XSetForeground(disID, grC, wData.fg); \
  XSetFont(disID, grC, wData.font); \
\
  XWindowAttributes att; \
  XSetWindowAttributes  attr; \
  XGetWindowAttributes(disID, winID, &att); \
  attr.backing_store = Always; \
  XChangeWindowAttributes(disID, winID, CWBackingStore, &attr); \
  w = att.width; \
  h = att.height;

#define UNPLUG_WIDGETS_GC XtReleaseGC(targetW, grC);

/*--------------------------------------------------------------------*/

/*------- �������� ������ image ��� ������ � ������������ ------------*/

class image {

public:
 enum { RED, GREEN, BLUE };
 static const int base = 3, bits_in_byte = 8;
 unsigned char rgb[base];
 
 XImage *im;
 XColor *col;
 Colormap clmp;
 int width, height, ncol, flag;
 unsigned long **thecolors; // array keeping the colors

 void GetPalette (Display *d) {
   for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++) {
     (col+ncol)->pixel = XGetPixel(im, j, i);
     for (int q=0;q<ncol;q++) if ( (col+q)->pixel == (col+ncol)->pixel ) { flag = 1; break; } 
     if ( (i == 0 && j == 0) || flag == 0) ncol++; else flag = 0;
				     }
     XQueryColors(d, clmp, col, ncol);
 };
 
 image () {
  thecolors = NULL;
  col = new XColor[MAX_COLOR_NUMBER];
  if (!col) { printf("Not enough memory!\n"); exit(1); }
 };

 image (Display *d, Window win, int x, int y, int w, int h, Colormap colmap) {
  thecolors = NULL;
  col = new XColor[MAX_COLOR_NUMBER];
  if (!col) { printf("Not enough memory!\n"); exit(1); }
  init (d, win, x, y, w, h, colmap);
 };

 void init (Display *d, Window win, int x, int y, int w, int h, Colormap colmap) {
   clmp = colmap;
   width = w;
   height = h;
   ncol = flag = 0;
   im = XGetImage (d, win, x, y, width, height, AllPlanes, ZPixmap);
   GetPalette (d);
 };

 void close () {
   if (thecolors != NULL) { for (unsigned char i=0; i<ncol; ++i) { delete thecolors[i]; } delete (thecolors); }
   thecolors = NULL;
   if (im != NULL) XDestroyImage (im);
   im = NULL;
 };

 virtual ~image () {
   if (thecolors != NULL) { for (unsigned char i=0; i<ncol; ++i) { delete thecolors[i]; } delete (thecolors); }
   delete (col);
   if (im != NULL) XDestroyImage (im);
 };

 void GetRGB (int i, int j) {
   for (int q=0;q<ncol;q++) {
    if ( (col+q)->pixel != XGetPixel(im, j, i) ) continue;
     else { rgb[RED]   = (col+q)->red   >> bits_in_byte;
            rgb[GREEN] = (col+q)->green >> bits_in_byte;
            rgb[BLUE]  = (col+q)->blue  >> bits_in_byte;
            break;
           }
                             }
 };

 void CreateColors () {
  thecolors = new unsigned long*[ncol];
  if (!thecolors) { perror("new thecolors"); exit (1); }
  for (unsigned char i=0; i <ncol; ++i) {
   thecolors[i] = new unsigned long[4];
   if (!thecolors[i]) { perror("new thecolors[i]"); exit (1); }
   memset (thecolors[i], 0, 4 * sizeof(unsigned long));
  }
  unsigned char thecolor_counter = 0;
  unsigned char k;
  for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) {
   GetRGB (i, j);
   /* setting the colors */
   for (k=0; k<thecolor_counter; ++k) {
    if (rgb[0] == thecolors[k][0] && rgb[1] == thecolors[k][1] && rgb[2] == thecolors[k][2]) break;
   }
   if (k == thecolor_counter) {
    thecolors[thecolor_counter][0] = rgb[0];
    thecolors[thecolor_counter][1] = rgb[1];
    thecolors[thecolor_counter][2] = rgb[2];
    thecolors[thecolor_counter][3] = XGetPixel(im, j, i);
    thecolor_counter++;
    if (thecolor_counter == ncol) break;
   } //if
  } //for
 };

 int get_pixel (int x, int y) {
  unsigned long pix = XGetPixel(im, x, y);
  for (int i=0; i<ncol; ++i) if (pix == thecolors[i][3]) return i;
  return 0;
 };

};

template<int N> struct frame_col {
  Pixel fg[N];
};
const int max_clnnum = 255;

image pict; // image file

#endif



#ifdef MEM_

using namespace std;

#ifndef _SC_AVPHYS_PAGES
 #define __NO_MEMORY_MACROS__
#else
 #ifndef _SC_PAGESIZE
  #define __NO_MEMORY_MACROS__ 
 #endif
#endif

#ifdef __NO_MEMORY_MACROS__

#if HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

 #ifdef __Linux__
  #define __YES_LINUX__
 #else
  #ifdef __linux__
   #define __YES_LINUX__
  #endif
 #endif

 #ifdef __YES_LINUX__

 #if HAVE_SYS_SYSINFO_H
 # include <sys/sysinfo.h>
 #endif

 #if HAVE_LINUX_VERSION_H
 # include <linux/version.h>
 #endif

  #ifndef KERNEL_VERSION
   #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
  #endif
  
 #endif //#ifdef __YES_LINUX__
  
#endif //#ifdef __NO_MEMORY_MACROS__

#endif  //#ifdef MEM_

