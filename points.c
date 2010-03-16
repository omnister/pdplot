#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "points.h"
#include "xwin.h"
#include "readfont.h"
#include "symbol.h"

#define MAXPLOTS 10

// initialized variables will get
// overridden by initplot();
void fontclip(int mode);

int numplots=0;
PLOTDAT plots[MAXPLOTS];
static int nsegs=0;
static int fontnsegs=0;
static int pennum=1;
int linenum=1;
static int backstat=0;
static double pad;
static double ticklen;
static int gridstatex=1;
static int gridstatey=1;
static int gridpenx=13;
static int gridpeny=13;
static int xgridline=3;
static int ygridline=3;
static int framepen=1;
static int scalemode=1;
static int tickmode=1;
static int boxmode=1;
static int symbolmode=0;
static int linemode=1;
static int symnum=0;
static int autopenflag=1;
static int autolineflag=0;
static int autosymflag=1;
static int isotropic=0;
static double isoscale=1.0;
static int symline=0;

static double fontsize;
static double xfontsize;
static double yfontsize;
static double charsize=1.0; 	// scales all characters
static double symbolsize=1.0;	// scales symbols
static double ticksize=1.0;	// scales ticks	
static double scalesize=1.0;    // scales axis labels
static double tagsize=1.0;      // scales grid tick values
static double titlesize=1.0;    // scales title	
static double labelsize=1.0;    // scales label
static double xtolerance=0.0;
static double ytolerance=0.0;
static double labelangle=0.0;

DATUM *datum_new(x,y)
double x,y;
{
    DATUM *tmp;
    tmp = (DATUM *) malloc(sizeof(DATUM));
    tmp->cmd = NULL;
    tmp->x = x;
    tmp->y = y;
    tmp->next = NULL;
    tmp->prev = tmp;    /* prev pointer points at last */
    return(tmp);
}

void iso(int mode, double scale) {
    isotropic=mode;
    isoscale=scale;
}

void tick(int mode) {
   tickmode=mode;
}

void scale(int mode) {
   scalemode=mode;
}

void box(int mode) {
    boxmode=mode;
}

void tickset(double scale) {
   ticksize=scale; 
}

void xgridstate(int mode) {
   gridstatex=mode;
}

void ygridstate(int mode) {
   gridstatey=mode;
}

void gridstate(int mode) {
   gridstatex=gridstatey=mode;
}

void frame_pen(int mode) {
   framepen=mode;
}

void xgridpen(int mode) {
   gridpenx=mode;
}

void ygridpen(int mode) {
   gridpeny=mode;
}

void gridpen(int mode) {
   gridpenx=gridpeny=mode;
}

void xgrid(int line) {
   xgridline=line;
}

void ygrid(int line) {
   ygridline=line;
}

void grid(int line) {
   xgridline=ygridline=line;
}


void setcharsize(double size) {
   charsize=size;
}

void xscaletol(double val) {
   xtolerance=val;
}

void yscaletol(double val) {
   ytolerance=val;
}

void scaletol(double val) {
   xtolerance=val;
   ytolerance=val;
}

void labelrotation(double angle) {
   labelangle=angle;
}

// buttons are 1,2,3 left->right
// button 4 is scroll forward, button 5 backward

void button(double x, double y, int buttonno, int pressdown) {
    int i;
    PLOTDAT *pd;
    int debug=0;
    static double x1,y1,x2,y2,x3,y3;
    static int npts1=0;		// FIXME, need a way to reset state when plotting second graph
    static int npts3=0;
    char buf[256];
    if (pressdown) {	// 1=button_down, 0_button_up
	for (i=0; i<=numplots; i++) {
	  pd = &(plots[numplots-i]); 		// compute bounding box for each graph 
	  if (debug) printf("==============================\n");
	  if (debug) printf("button #%d\n", buttonno);
	  if (debug) printf("ll=%g %g ur=%g %g\n", pd->llx, pd->lly, pd->urx, pd->ury);
	  if (debug) printf("min=%g %g max=%g %g\n", pd->xmin, pd->ymin, pd->xmax, pd->ymax);
	  if (debug) printf("xsetmin=%g %g xsetmax=%g %g\n", pd->xsetmin, pd->ysetmin, pd->xsetmax, pd->ysetmax);
	  if (debug) printf("xsetmin=%g %g xsetmax=%g %g\n", pd->xsetmin, pd->ysetmin, pd->xsetmax, pd->ysetmax);
	  if (debug) printf("gsetmin=%g %g gsetmax=%g %g\n", plots[0].xmin, pd->ysetmin, plots[0].xmax, pd->ysetmax);

	  if (x >= pd->llx && x <= pd->urx && y >= pd->lly && y <= pd->ury) {
	       x1= plots[0].xmin+(plots[0].xmax-plots[0].xmin)*(x-pd->llx)/(pd->urx-pd->llx);
	       y1= pd->ymin+(pd->ymax-pd->ymin)*(pd->ury-y)/(pd->ury-pd->lly);
	       if (buttonno == 1) {
		   if ((x2-x1) == 0.0  && (y2-y1) == 0.0) npts1=0;
		   if (npts1++ < 1) {
		      sprintf(buf,"x=%.3g y=%.3g", x1, y1);
		   } else {
		      if ((x1-x2) == 0) {
			  sprintf(buf,"x=%.3g y=%.3g dx=%.3g dy=%.3g", x1, y1, x1-x2, y1-y2);
		      } else {
			  sprintf(buf,"x=%.3g y=%.3g dx=%.3g dy=%.3g 1/dx = %.3g dy/dx = %.3g",
			    x1, y1, x1-x2, y1-y2,1.0/(x1-x2), (y1-y2)/(x1-x2));
		      }
		   }
		   xwin_set_pen_line(1,1);
		   xwin_annotate(buf);
		   if (debug) printf("plot %d: (%g,%g)->(%g,%g) %d %d\n", i,
			x, y, x1, y1,
			buttonno, pressdown);
		   x2=x1; y2=y1;
		   return;
	      } else if (buttonno == 3) {
		  if ((fabs(x3-x1) < fabs(pd->xmax-pd->xmin)/100.0) && 
		  	(fabs(y3-y1) < fabs(pd->ymax-pd->ymin)/100.0)) {
		      xset(0.0,0.0);	// zoom out
		      yset(0.0,0.0);
		      need_redraw++;
		      npts3=0;
		  } else if (++npts3 ==2) {
		      if (x3<x1) xset(x3,x1);	// zoom in
		      if (x1<x3) xset(x1,x3);
		      if (y3<y1) yset(y3,y1);	// zoom in
		      if (y1<y3) yset(y1,y3);
		      need_redraw++;
		      npts3=0;
		  }
		  x3=x1; y3=y1;
		  need_redraw++;
	          return;
	      }
	   }
	}
	if (debug) printf("In plot frame: %g %g %d %d\n",  x, y, buttonno, pressdown);
	if (buttonno == 3) {
	    xset(0.0,0.0);	// zoom out
	    yset(0.0,0.0);
	    need_redraw++;
        }
    }
}

void nextygraph() {
   if (numplots < MAXPLOTS-1) {
       numplots++;
   } else {
       printf("can't have more than %d plots\n", MAXPLOTS);
   }
}

char *strsave(s)   /* save string s somewhere */
char *s;
{
    char *p;
    if (s == NULL) 
	return(s);
    if ((p = (char *) malloc(strlen(s)+1)) != NULL) 
	strcpy(p,s);
    return(p);
}

void linkit(DATUM *d) // add a datum to the current list
{
    DATUM *tmp;
    if (plots[numplots].data == NULL) {
        plots[numplots].data = d;
    } else {
	tmp = plots[numplots].data->prev;     /* save pointer to last datum */
	tmp->next = d;
	tmp->next->prev = tmp;
	plots[numplots].data->prev = tmp->next;
    }
}


// Just save the scaled points.  then you
// don't have to bother about scaling again.
// However, this means that points read before
// the x or yscale will not get scaled.

void savepoint(double x, double y)
{
    // scale the points as they are read
    linkit(datum_new( x/plots[0].xscale,
	y/plots[numplots].yscale));
}

void savecmd(char *cmd) {
    DATUM *tmp;
    tmp=datum_new(0.0,0.0);
    tmp->cmd = strsave(cmd);
    linkit(tmp);
}

void dumppoints() {
    DATUM *p;
    int i;

    printf("#--------------------\n");
    for (i=0; i<MAXPLOTS; i++) {
        if (i && plots[i].data != NULL) {
	    printf("nextygraph\n");
	}
	for (p=plots[i].data; p!=(DATUM *)0; p=p->next) {
	    if (p->cmd == NULL) {
	        printf("%d: %g %g\n", i, p->x, p->y);
	    } else {
	        printf("%d: %s\n", i, p->cmd);
	    }
	}
    }
}

void freedata(DATUM *dp) {
    DATUM *p;
    for (p=dp; p!=(DATUM *)0; p=p->next) {
        free(p);
	// FIXME: make sure and free cmd strings
    }
}

void initplot(void) {
   static int initialized=0;
   PLOTDAT *dp;
   int i;
   numplots=0;
   for (i=0; i<MAXPLOTS; i++) {
      dp = &plots[i];
      if (!initialized) {
          dp->data = NULL;
          dp->title = NULL;
          dp->xaxis = NULL;
          dp->yaxis = NULL;
	  initialized++;
      } else {
	  freedata(dp->data);
	  dp->data = NULL;
	  free(dp->title);
          dp->title = NULL;
	  free(dp->xaxis);
          dp->xaxis = NULL;
	  free(dp->yaxis);
          dp->yaxis = NULL;
      }
      dp->xlogmode = dp->ylogmode = 0;
      dp->xsetmin = dp->xsetmax = 0.0;
      dp->ysetmin = dp->ysetmax = 0.0;
      dp->xscale = dp->yscale  = 1.0;
   }
   // now reset all globals to default condition
   numplots=0;
   nsegs=0;
   fontnsegs=0;
   pennum=1;
   linenum=1;
   backstat=0;
   gridstatex=1;
   gridstatey=1;
   framepen=1;
   gridpenx=13;		// default is dim off-white grid
   gridpeny=13;		// default is dim off-white grid
   xgridline=3;
   ygridline=3;
   boxmode=1;
   symbolmode=0;
   linemode=1;
   symnum=0;
   autopenflag=1;
   autolineflag=0;
   autosymflag=1;
   isotropic=0;
   isoscale=1.0;
   symline=0;
   scalemode=1;
   tickmode=1;
   xtolerance=0.0;
   ytolerance=0.0;
   labelangle=0.0;

   charsize=1.0;     // scales all characters
   symbolsize=1.0;   // scales symbols
   ticksize=1.0;     // scales ticks
   scalesize=1.0;    // scales axis labels
   tagsize=1.0;      // scales grid tick values
   titlesize=1.0;    // scales title
   labelsize=1.0;    // scales label
}

void main2() {
    initplot();
    savepoint(1.0,2.0);
    savepoint(3.0,4.0);
    dumppoints();
    nextygraph();
    savepoint(8.0,2.3);
    savepoint(9.0,4.7);
    savepoint(9.1,0.7);
    dumppoints();
    initplot();
    dumppoints();
    savepoint(1.0,1.0);
    dumppoints();
}

// set the axis mode 
// axis: x = 0, y = 1
// mode: lin = 0, log = 1, 20*log10(x)=2, 10*log10(x)=3 

double logscale(double x, int axis) {
    int mode;
    int debug=0;

    if (axis) {		// y
      	mode = plots[numplots].ylogmode;
    } else {		// x
        mode = plots[0].xlogmode;
    }

    if (debug) printf("mode: %d, val_in: %g", mode, x);

    switch (mode) {
    case 0: 		// linear mode, do nothing
        break;
    case 1: 		// just log the data and rearrange the labels
	if (x>0.0) {
	    x = log10(x);
	} else {
	    x = 0.0;
	}
        break;
    case 2: 		// 20*log(x)
	if (x>0.0) {
	    x = 20.0*log10(x);
	} else {
	    x = 0.0;
	}
        break;
    case 3:		// 10*log(x)
	if (x>0.0) {
	    x = 10.0*log10(x);
	} else {
	    x = 0.0;
	}
        break;
    default:
        break;
    }
    if (debug) printf(" val_out: %g\n", x);
    return(x);
}

void logmode(int axis, int mode) {
   if (axis) {
	plots[numplots].ylogmode = mode;
   } else {
	plots[0].xlogmode = mode;
   }
}

void gridtick(PLOTDAT *pd, double alpha, int x) {
    double tmp;
    xwin_set_pen_line(1,1);
    if (tickmode && ticklen>0) {
	if (x) { 			// xaxis grid lines
	    tmp = alpha*pd->urx+(1.0-alpha)*pd->llx;
	    xwin_draw_line( tmp, pd->lly, tmp, pd->lly+ticklen);
	    xwin_draw_line( tmp, pd->ury, tmp, pd->ury-ticklen);
	} else { 			// yaxis grid lines
	    tmp = alpha*pd->lly+(1.0-alpha)*pd->ury;
	    xwin_draw_line( pd->llx, tmp, pd->llx+ticklen, tmp);
	    xwin_draw_line( pd->urx, tmp, pd->urx-ticklen, tmp);
	}
    }
}

void gridline(PLOTDAT *pd, double alpha, int x) {
    double tmp;
    gridtick(pd, alpha, x);
    if (x) { 			// xaxis grid lines
	if (gridstatex) {
	    xwin_set_pen_line(gridpenx,xgridline);
	    tmp = alpha*pd->urx+(1.0-alpha)*pd->llx;
	    xwin_draw_line( tmp, pd->lly, tmp, pd->ury);
	}
    } else { 			// yaxis grid lines
	if (gridstatey) {
	    xwin_set_pen_line(gridpeny,ygridline);
	    tmp = alpha*pd->lly+(1.0-alpha)*pd->ury;
	    xwin_draw_line( pd->llx, tmp, pd->urx, tmp);
	}
    }
}

void axislabel(PLOTDAT *pd, char *str, int x) {
    double tmp,mid;
    xwin_set_pen_line(framepen,1);
    if (scalemode) {
	if (x) { 			// xaxis label
	    tmp = pd->lly-pad-xfontsize;
	    mid = (pd->llx+pd->urx)/2.0;
	    do_note(str, mid, tmp, MIRROR_OFF, 
	    	scalesize*charsize*xfontsize*.6, 1.0, 0.0, 0.0, 0, 4);
	} else { 			// yaxis label
	    tmp = pd->llx-pad-2.0*yfontsize;
	    mid = (pd->lly+pd->ury)/2.0;
	    do_note(str, tmp, mid , MIRROR_OFF, 
	    	scalesize*charsize*yfontsize*.6, 1.0, 0.0, 90.0, 0, 4);
	}
    }
}

void gridlabel(PLOTDAT *pd, char *str, double alpha, int x) {
    double tmp;
    xwin_set_pen_line(framepen,1);
    if (scalemode) {
	if (x) { 			// xaxis label
	    tmp = alpha*pd->urx+(1.0-alpha)*pd->llx;
	    do_note(str, tmp, pd->lly-pad, MIRROR_OFF,
	    	xfontsize*.6*charsize*tagsize, 1.0, 0.0, 0.0, 0, 7);
	} else { 			// yaxis label
	    tmp = alpha*pd->ury+(1.0-alpha)*pd->lly;
	    do_note(str, pd->llx-pad, tmp, MIRROR_OFF,
	    	yfontsize*.6*charsize*tagsize, 1.0, 0.0, 0.0, 0, 5);
	}
    }
}

void setbounds(PLOTDAT *pd) {		// simply set pd->[yx]minmax to bounds
    DATUM *p; 
    int ix=0;
    double xx, yy;

    for (p=pd->data; p!=(DATUM *)0; p=p->next) {
  	if (p->cmd == NULL) {
	    xx = logscale(p->x,0);
	    yy = logscale(p->y,1);
	    if (ix == 0) {	// initialize
		pd->xmin=pd->xmax=xx;
		pd->ymin=pd->ymax=yy;
		ix=1;
	    } else {	// accumulate
		if(xx < pd->xmin) pd->xmin=xx;
		if(xx > pd->xmax) pd->xmax=xx;
		if(yy < pd->ymin) pd->ymin=yy;
		if(yy > pd->ymax) pd->ymax=yy;
	    }
	}
    }
}

void xset(double xmin, double xmax) {
    plots[0].xsetmin = xmin;
    plots[0].xsetmax = xmax;
}

void yset(double ymin, double ymax) {
    plots[numplots].ysetmin = ymin;
    plots[numplots].ysetmax = ymax;
}

void xscale(char *s, double scale) {
    if (plots[0].xaxis != NULL) {
        free(plots[0].xaxis);
    } 
    plots[0].xaxis = strsave(s);
    plots[0].xscale = scale;
}

void yscale(char *s, double scale) {
    if (plots[numplots].yaxis != NULL) {
        free(plots[numplots].yaxis);
    } 
    plots[numplots].yaxis = strsave(s);
    plots[numplots].yscale = scale;
}

void title(char *s) {
    if (plots[0].title != NULL) {
        free(plots[0].title);
    } 
    plots[0].title = strsave(s);
}

void do_command(PLOTDAT *pd, char *s) {
    int penno;
    int lineno;
    int symno;
    double tmp;
    char buf[128];
    Symbol *sym;

   if (strncmp(s,"jump",4)==0) {
       jump();
   } else if (strncmp(s,"pen",3)==0) {
       if (sscanf(s, "%*s %d", &penno)==1) {
	  pen(penno);
       } else {
	  pen(++pennum);
       }
   } else if (strncmp(s,"autopen",7)==0) {
       autopenflag=1;
   } else if (strncmp(s,"noautopen",9)==0) {
       autopenflag=0;
   } else if (strncmp(s,"autosymbol",10)==0) {
       autosymflag=1;
   } else if (strncmp(s,"noautosymbol",12)==0) {
       autosymflag=0;
   } else if (strncmp(s,"line",4)==0) {
       if (sscanf(s, "%*s %d", &lineno)==1) {
	  line(lineno);
       } else {
	  line(++linenum);
       }
   } else if (strncmp(s,"autoline",7)==0) {
       autolineflag=1;
   } else if (strncmp(s,"noautoline",9)==0) {
       autolineflag=0;
   } else if (strncmp(s,"back",4)==0) {
       back(1);
   } else if (strncmp(s,"symbol+line",11)==0) {
       symline++;
       symbolmode = 1;
       linemode = 1;
   } else if (strncmp(s,"charsize",8)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp >= 0.1 && tmp <= 10.0) {
	       charsize=tmp;
	   } else {
	      fprintf(stderr,"bad argument to charsize cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to charsize cmd: %s\n", s);
       }
   } else if (strncmp(s,"tagsize",7)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp >= 0.1 && tmp <= 10.0) {
	       tagsize=tmp;
	   } else {
	      fprintf(stderr,"bad argument to tagsize cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to tagsize cmd: %s\n", s);
       }
   } else if (strncmp(s,"titlesize",9)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp >= 0.1 && tmp <= 10.0) {
	       titlesize=tmp;
	   } else {
	      fprintf(stderr,"bad argument to titlesize cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to titlesize cmd: %s\n", s);
       }
   } else if (strncmp(s,"labelsize",9)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp >= 0.1 && tmp <= 10.0) {
	       labelsize=tmp;
	   } else {
	      fprintf(stderr,"bad argument to labelsize cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to labelsize cmd: %s\n", s);
       }
   } else if (strncmp(s,"labeldir",8)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp <= 360.0 && tmp >= -360.0) {
	       labelangle=tmp;
	   } else {
	      fprintf(stderr,"angle too large in labeldir cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to labeldir cmd: %s\n", s);
       }
   } else if (strncmp(s,"scalesize",9)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp >= 0.1 && tmp <= 10.0) {
	       scalesize=tmp;
	   } else {
	      fprintf(stderr,"bad argument to scalesize cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to scalesize cmd: %s\n", s);
       }
   } else if (strncmp(s,"symbolsize",10)==0) {
       if (sscanf(s, "%*s %lg", &tmp)==1) {
	   if (tmp >= 0.1 && tmp <= 10.0) {
	       symbolsize=tmp;
	   } else {
	      fprintf(stderr,"bad argument to symbolsize cmd: %s\n", s);
	   }
       } else {
	   fprintf(stderr,"bad argument to symbolsize cmd: %s\n", s);
       }
   } else if (strncmp(s,"symbol",6)==0) {	
       if (!symline) {   // must follow symbol+line to keep line
	   symbolmode = 1;
	   linemode = 0;
       } else {
	   symbolmode = 1;
       }
       if (sscanf(s, "%*s %d", &symno)==1) {
	   symbol(symno);
       } else {
	  if (sscanf(s, "%*s %s", buf)==1) {
	      if ((sym=lookup(buf)) != 0) {
		  symbol(sym->index);
	      } else {
		  symbol(++symnum);
	      }
	  }
       }
   } else if (strncmp(s,"nosymbol",8)==0) {
	symbolmode = 0;
	linemode = 1;
   } else if (strncmp(s,"noline",6)==0) {
	symbolmode = 1;
	linemode = 0;
   } else if (strncmp(s,"noback",6)==0) {
       back(0);
   } 
}

void render() 	// this is where the image gets drawn
{		
    int i;
    double llx, lly, urx, ury, div, del;
    double width, height;
    PLOTDAT *pd;
    DATUM *p;
    int debug=0;
    extern double ticklen;
    extern double ticksize;
    double x,y;
    double xold, yold;
    double xmin, xmax, ymin, ymax;
    char buf[128];
    double xx,yy;

    xwin_size(&width, &height);

    llx=0.1*width; 
    lly=0.1*height;
    urx=0.95*width;
    ury=0.9*height;
    fontsize  = (height+width)/50.0;
    xfontsize = (height)/20.0;
    yfontsize = (width)/30.0;
    ticklen = ticksize*(height+width)/200.0;
    pad=fontsize/4.0;

    // we allow each graph to be 5 units tall, and spacing between
    // graphs is one unit...

    // place for a title
    if (plots[0].title != NULL && scalemode) {
	xwin_set_pen_line(1,1);
	do_note(plots[0].title, (llx+urx)/2.0, 
	   pad+ury, MIRROR_OFF, fontsize*charsize*titlesize,
	   1.28, 0.0, 0.0, 0, 1);
    }
    if (plots[0].xaxis != NULL) {
       xwin_set_pen_line(1,1);
       axislabel(&plots[numplots],plots[0].xaxis,1);
    }

    // now we do a dry run through the dataset using the bounds
    // calculation code in clip to set plot boundaries w.r.t.  xset,
    // yset commands

    xwin_display(0);	// turn off display

    for (i=0; i<=numplots; i++) {
      pd = &(plots[numplots-i]); 

      setbounds(pd);		// update pd->xy(min/max) to data		

      xmin=pd->xmin;
      xmax=pd->xmax;
      ymin=pd->ymin;
      ymax=pd->ymax;

      if (plots[0].xsetmin != plots[0].xsetmax) {	// xset was used	
         xmin = plots[0].xsetmin;
	 xmax = plots[0].xsetmax;
      }
      if (pd->ysetmin != pd->ysetmax) {			// yset was used
         ymin = pd->ysetmin;
	 ymax = pd->ysetmax;
      }
      pd->boundsflag=0;
      nsegs=0;

      // FIXME: only need to do this next loop through the
      // data if either xset or yset is used.  Otherwise,
      // just use the data bounds...

      // printf("graph %d: setting clip ymin/max = %g %g\n", numplots-i, ymin, ymax); // RCW

      for (p=pd->data; p!=(DATUM *)0; p=p->next) {
	  clip_set(xmin, xmax, ymin, ymax);
	  if (p->cmd == NULL) {
	     x=logscale(p->x,0);
	     y=logscale(p->y,1);
	     if (!backstat && nsegs > 1 && x<xold) nsegs=0;
	     nsegs++;
	     if (linemode && nsegs > 1) {
	         clip(pd, xold, yold, x, y);
	     }
	     if (symbolmode) {
	         clip(pd, x, y, x, y);
	     }
	     xold=x; yold=y;
	     // draw(pd, logscale(p->x,0),logscale(p->y,1));
	  } else {
		do_command(pd, p->cmd);
	  }
       }
       pd->xmin = pd->bbxmin;
       pd->xmax = pd->bbxmax;
       pd->ymin = pd->bbymin;
       pd->ymax = pd->bbymax;

      if (plots[0].xsetmin != plots[0].xsetmax) {	// xset was used	
         pd->xmin = plots[0].xsetmin;
	 pd->xmax = plots[0].xsetmax;
      }
      if (pd->ysetmin != pd->ysetmax) {			// yset was used
         pd->ymin = pd->ysetmin;
	 ymax = pd->ysetmax;
      }

	double a,b;
	if (xtolerance>=0.0) {
	   a = pd->xmax-(pd->xmax-pd->xmin)*xtolerance;
	   b = pd->xmin+(pd->xmax-pd->xmin)*xtolerance;
	} else {
	   a = pd->xmax+(pd->xmax-pd->xmin)*(-xtolerance);
	   b = pd->xmin-(pd->xmax-pd->xmin)*(-xtolerance);
	}
	pd->xmax = a; pd->xmin = b;
	if (ytolerance>=0.0) {
	   a = pd->ymax-(pd->ymax-pd->ymin)*ytolerance;
	   b = pd->ymin+(pd->ymax-pd->ymin)*ytolerance;
	} else {
	   a = pd->ymax+(pd->ymax-pd->ymin)*(-ytolerance);
	   b = pd->ymin-(pd->ymax-pd->ymin)*(-ytolerance);
	}
	pd->ymax = a; pd->ymin = b;
    }


    xwin_display(1);	// turn display on

    for (i=0; i<=numplots; i++) {
      xmin = plots[0].xmin;
      xmax = plots[0].xmax;
      pd = &(plots[numplots-i]); 		// compute bounding box for each graph 
      div=5.0*(numplots+1.0)+(numplots);	// number of vertical divisions
      del=(ury-lly)/div;

      // printf("graph %d: ymin/max = %g %g\n", numplots-i, pd->ymin, pd->ymax); // RCW

      pd->llx = llx;
      pd->lly = (lly+6.0*del*(double)i);
      pd->urx = urx;
      pd->ury = (lly+(5.0+6.0*(double)i)*del);

      if (debug) printf("numplots = %d: %g %g %g %g\n",
      	numplots, pd->llx, pd->lly, pd->urx, pd->ury);

      double uppx, uppy;  	// units per pixel
      double del, max, min, mid;

      // do a dry run on labels to get exact scales

      if (debug) printf("calling ll: %g %g %g %g\n", xmin, pd->ymin, xmax, pd->ymax);
      loose_label(pd,&(pd->ymin),&(pd->ymax),20/(numplots+2),0, 1, pd->ylogmode, 1); 
      loose_label(pd,&(xmin),&(xmax),8, 1, (i==0), plots[0].xlogmode, 1);		   
      if (debug) printf("bounds %g %g %g %g\n", xmin, pd->ymin, xmax, pd->ymax);
      plots[0].xmin = xmin;
      plots[0].xmax = xmax;

      if (isotropic) {
	  uppx=(xmax - xmin)/(pd->urx - pd->llx);
          uppy=isoscale*(pd->ymax - pd->ymin)/(pd->ury - pd->lly);
	  if (uppx > uppy) {
	      del = pd->ury-pd->lly;
	      mid = (pd->ury+pd->lly)/2.0;
	      max = mid + del/(2.0*uppx/uppy);
	      min = mid - del/(2.0*uppx/uppy);
	      pd->ury=max;
	      pd->lly=min;
	  } else {
	      del = pd->urx-pd->llx;
	      mid = (pd->urx+pd->llx)/2.0;
	      max = mid + del/(2.0*uppy/uppx);
	      min = mid - del/(2.0*uppy/uppx);
	      pd->urx=max;
	      pd->llx=min;
	  }
      }

      if (pd->yaxis != NULL) {
  	  xwin_set_pen_line(1,1);
          axislabel(pd,pd->yaxis,0);
      }

      back(0);		// defaults on a per graph basis...		
      pen(2);		// select red pen
      symbol(1);	// select first symbol
      line(1);
      autopenflag=1;
      autosymflag=1;
      // gridpenx=1;
      // gridpeny=1;
      // xgridline=3;
      // ygridline=3;
      linemode=1;
      symbolsize=1.0;
      if (i==0) {
	   symbolmode=0;
	   autolineflag=0;
      }
      jump();

      for (p=pd->data; p!=(DATUM *)0; p=p->next) {
	 // clip_set(pd->llx, pd->urx, pd->lly, pd->ury);
	 clip_set(xmin, xmax, pd->ymin, pd->ymax);
  	 if (p->cmd == NULL) {
	    draw(pd, logscale(p->x,0),logscale(p->y,1));
         } else if (strncmp(p->cmd,"label ",6)==0) {
	     if (sscanf(p->cmd, "%*s %lg%% %lg%% %[^#]", &x, &y, buf)==3) {
	        xx=(pd->urx-pd->llx)*(x/100.0)+pd->llx;
		yy=(pd->ury-pd->lly)*(y/100.0)+pd->lly; 
		do_note(buf, xx, yy, MIRROR_OFF , 0.6*fontsize*charsize*labelsize,
		   1.0, 0.0, labelangle, 0, 3);
	     } else if (sscanf(p->cmd, "%*s %lg %lg %[^#]", &x, &y, buf)==3) {
		xx=(pd->urx-pd->llx)*(logscale(x,0)-xmin)/(xmax-xmin)+pd->llx;
		yy=(pd->ury-pd->lly)*(logscale(y,1)-pd->ymin)/(pd->ymax-pd->ymin)+pd->lly; 
		do_note(buf, xx, yy, MIRROR_OFF , 0.6*fontsize*charsize*labelsize,
		    1.0, 0.0, labelangle, 0, 3);
	     } else { 
	        fprintf(stderr,"bad argument to label cmd: %s\n", p->cmd);
	     }
	 } else {
	    do_command(pd, p->cmd);
	 }
      }

      xwin_set_pen_line(framepen,1);
      if (boxmode) {
	  xwin_draw_box(pd->llx, pd->lly, pd->urx, pd->ury);	// plot boundary
      }

      // now run labels with possibly tweaked urxy, llxy for isotropic plot
      if (debug) printf("calling ll: %g %g %g %g\n", xmin, pd->ymin, xmax, pd->ymax);
      loose_label(pd,&(pd->ymin),&(pd->ymax),20/(numplots+2),0, 1, pd->ylogmode, 0); 
      loose_label(pd,&(xmin),&(xmax),8, 1, (i==0), plots[0].xlogmode, 0);		   
      if (debug) printf("%g %g %g %g\n", xmin, pd->ymin, xmax, pd->ymax);
    }
}


//------------------------------------------------------------------------------
// Core drawing functions
//------------------------------------------------------------------------------

/***************** coordinate transformation utilities ************/

XFORM *compose(xf1, xf2)
XFORM *xf1, *xf2;
{
     XFORM *xp;
     xp = (XFORM *) malloc(sizeof(XFORM)); 

     xp->r11 = (xf1->r11 * xf2->r11) + (xf1->r12 * xf2->r21);
     xp->r12 = (xf1->r11 * xf2->r12) + (xf1->r12 * xf2->r22);
     xp->r21 = (xf1->r21 * xf2->r11) + (xf1->r22 * xf2->r21);
     xp->r22 = (xf1->r21 * xf2->r12) + (xf1->r22 * xf2->r22);
     xp->dx  = (xf1->dx  * xf2->r11) + (xf1->dy  * xf2->r21) + xf2->dx;
     xp->dy  = (xf1->dx  * xf2->r12) + (xf1->dy  * xf2->r22) + xf2->dy;

     return (xp);
}

/* in-place rotate a transform matrix by theta degrees */
void mat_rotate(xp, theta)
XFORM *xp;
double theta;
{
    double s,c,t;
    s=sin(2.0*M_PI*theta/360.0);
    c=cos(2.0*M_PI*theta/360.0);

    t = c*xp->r11 - s*xp->r12;
    xp->r12 = c*xp->r12 + s*xp->r11;
    xp->r11 = t;

    t = c*xp->r21 - s*xp->r22;
    xp->r22 = c*xp->r22 + s*xp->r21;
    xp->r21 = t;

    t = c*xp->dx - s*xp->dy;
    xp->dy = c*xp->dy + s*xp->dx;
    xp->dx = t;
}

/* in-place scale transform */
void mat_scale(xp, sx, sy) 
XFORM *xp;
double sx, sy;
{
    xp->r11 *= sx;
    xp->r12 *= sy;
    xp->r21 *= sx;
    xp->r22 *= sy;
    xp->dx  *= sx;
    xp->dy  *= sy;
}

void mat_print(xa)
XFORM *xa;
{
     printf("\n");
     printf("\t%g\t%g\t%g\n", xa->r11, xa->r12, 0.0);
     printf("\t%g\t%g\t%g\n", xa->r21, xa->r22, 0.0);
     printf("\t%g\t%g\t%g\n", xa->dx, xa->dy, 1.0);
}   

/* in-place slant transform (for italics) */
void mat_slant(xp, theta) 
XFORM *xp;
double theta;
{
    double s,c;
    double a;

    int debug = 0;
    if (debug) printf("in mat_slant with theta=%g\n", theta);
    if (debug) mat_print(xp);

    s=sin(2.0*M_PI*theta/360.0);
    c=cos(2.0*M_PI*theta/360.0);
    a=s/c;

    xp->r21 += a*xp->r11;
    xp->r22 += a*xp->r12;

    if (debug) mat_print(xp);
}

void do_symbol(c, x, y, size)
int c;
double x, y;
double size;
{
    char s[2];
    s[0]=(char) ((c%55)+32);
    s[1]='\0';
    fontclip(1);	// turn on clipping
    do_note(s, x, y, MIRROR_OFF , size, 1.0, 0.0, 0.0, 1, 4);
    fontclip(0);	// turn off clipping
}

void do_note(string, x, y, mirror, size, aspect, slant, rotation, id, jf)
char *string;
double x, y;
int mirror;
double size;
double aspect;
double slant;
double rotation;
int id;
int jf;
{
    // printf("do_note: %s, (%g,%g), m:%d sl:%g a:%g sz:%g r:%g, id:%d, jf:%d\n",
    //	string, x, y, mirror, size, aspect, slant, rotation, id, jf);

    XFORM *xp;
    /* create a unit xform matrix */

    xp = (XFORM *) malloc(sizeof(XFORM));  
    xp->r11 = 1.0; xp->r12 = 0.0; xp->r21 = 0.0;
    xp->r22 = 1.0; xp->dx  = 0.0; xp->dy  = 0.0;

    /* NOTE: To work properly, these transformations have to */
    /* occur in the proper order, for example, rotation must come */
    /* after slant transformation */

    switch (mirror) {
	case MIRROR_OFF:
	    break;
	case MIRROR_X:
	    xp->r22 *= -1.0;
	    break;
	case MIRROR_Y:
	    xp->r11 *= -1.0;
	    break;
	case MIRROR_XY:
	    xp->r11 *= -1.0;
	    xp->r22 *= -1.0;
	    break;
    }

    mat_scale(xp, size, size);
    mat_scale(xp, 1.0/aspect, 1.0);
    mat_slant(xp, slant);
    mat_rotate(xp, rotation);

    xp->dx += x;
    xp->dy += y;

    writestring(string, xp, id, jf);
    free(xp);
}

void back(int x) {
   backstat = x;
}

void symbol(int sym_no) {
   symnum = sym_no%55;
}

void line(int line_no) {
   if (line_no >= 6) {
       linenum = line_no%6+1;
   } else {
       linenum = line_no;
   }
   xwin_set_pen_line(pennum,linenum);
}

void pen(int pen_no) {
   if (pen_no >= 15) {
       pennum = pen_no%15+1;
   } else {
       pennum = pen_no;
   }
   xwin_set_pen_line(pennum,linenum);
}

void fontjump() {
   fontnsegs=0;
}

static int clipmode=0;

void fontclip(int mode) {
   clipmode=mode;
};

void fontdraw(double x, double y) {
    static double xxold, yyold;
    extern int clipmode;
    fontnsegs++;
    if (fontnsegs > 1) {
	//if (clipmode) {
	//    clip(xxold, yyold, x, y);
	//} else {
	    xwin_draw_line(xxold, yyold, x, y);
	//}
    }
    xxold=x; yyold=y;
}

void jump() {
   nsegs=0;
}

void draw(PLOTDAT *pd, double x, double y) {
    static double xold, yold;
    double xs, ys;	// screen coords
    xs =(pd->urx-pd->llx)*(x-xmin)/(xmax-xmin)+pd->llx;
    ys =(pd->ury-pd->lly)*(y-pd->ymin)/(pd->ymax-pd->ymin)+pd->lly; 
    if (!backstat && (nsegs >= 1) && x<xold) {
       nsegs=0;
       if (autopenflag)	 pen(++pennum);
       if (autolineflag) line(++linenum);
    }
    if (symbolmode) {
      do_symbol(symnum, xs, ys, fontsize*symbolsize);
    } 
    if (nsegs==0) {
       if (autosymflag) symbol(++symnum);
    }
    nsegs++;
    if (nsegs > 1) {
	if (linemode) {
	    clip(pd, xold, yold, x, y);
	}
    }
    xold=x; yold=y;
}
