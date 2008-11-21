#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "points.h"

#define MAXPLOTS 10

int numplots=0;
PLOTDAT plots[MAXPLOTS];
static int nsegs=0;
static int fontnsegs=0;
static int pennum=1;
static int linenum=0;
static int backstat=0;
static double pad;
static double fontsize;
static double ticklen;
static int gridmode=1;
static int symbolmode=1;
static int linemode=0;
static int symnum=1;
static int autopenflag = 1;
static int autosymflag = 1;

static double symbolsize=1.0;

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

void grid(mode) {
   gridmode=mode;
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

void savepoint(double x, double y)
{
    linkit(datum_new(x,y));
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

freedata(DATUM *dp) {
    DATUM *p;
    for (p=dp; p!=(DATUM *)0; p=p->next) {
        free(p);
	// make sure and free cmd strings
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
	  free(dp->xaxis);
	  free(dp->yaxis);
      }
      dp->xsetmin = dp->xsetmax = 0.0;
      dp->ysetmin = dp->ysetmax = 0.0;
      dp->xscale = dp->yscale  = 1.0;
   }
}

main2() {
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

void gridtick(PLOTDAT *pd, double alpha, int x) {
    double tmp;
    if (gridmode) {
	xwin_set_pen_line(0,0);
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
    if (gridmode) {
	gridtick(pd, alpha, x);
	xwin_set_pen_line(0,1);
	if (x) { 			// xaxis grid lines
	    tmp = alpha*pd->urx+(1.0-alpha)*pd->llx;
	    xwin_draw_line( tmp, pd->lly, tmp, pd->ury);
	} else { 			// yaxis grid lines
	    tmp = alpha*pd->lly+(1.0-alpha)*pd->ury;
	    xwin_draw_line( pd->llx, tmp, pd->urx, tmp);
	}
    }
}

void axislabel(PLOTDAT *pd, char *str, int x) {
    double tmp,mid;
    xwin_set_pen_line(0,0);
    if (x) { 			// xaxis label
	tmp = pd->lly-pad-fontsize;
	mid = (pd->llx+pd->urx)/2.0;
        do_note(str, mid, tmp, MIRROR_OFF, fontsize*.6, 1.0, 0.0, 0.0, 0, 4);
    } else { 			// yaxis label
	tmp = pd->llx-pad-2.0*fontsize;
	mid = (pd->lly+pd->ury)/2.0;
        do_note(str, tmp, mid , MIRROR_OFF, fontsize*.6, 1.0, 0.0, 90.0, 0, 4);
    }
}

void gridlabel(PLOTDAT *pd, char *str, double alpha, int x) {
    double tmp;
    xwin_set_pen_line(0,0);
    if (x) { 			// xaxis label
	tmp = alpha*pd->urx+(1.0-alpha)*pd->llx;
        do_note(str, tmp, pd->lly-pad, MIRROR_OFF, fontsize*.6, 1.0, 0.0, 0.0, 0, 7);
    } else { 			// yaxis label
	tmp = alpha*pd->ury+(1.0-alpha)*pd->lly;
        do_note(str, pd->llx-pad, tmp, MIRROR_OFF, fontsize*.6, 1.0, 0.0, 0.0, 0, 5);
    }
}

void setbounds(PLOTDAT *pd) {		// FIXME: later on this should obey any xsets...
    DATUM *p; 
    int i=0;
    pd->xmin=pd->ymin=0.0;
    pd->xmax=pd->ymax=1.0;
    for (p=pd->data; p!=(DATUM *)0; p=p->next) {
  	if (p->cmd == NULL) {
	    if (i++ == 0) {
	      pd->xmin=pd->xmax=p->x;
	      pd->ymin=pd->xmax=p->y;
	    } else {
	      if(p->x < pd->xmin) pd->xmin=p->x;
	      if(p->x > pd->xmax) pd->xmax=p->x;
	      if(p->y < pd->ymin) pd->ymin=p->y;
	      if(p->y > pd->ymax) pd->ymax=p->y;
	    }
	}
    }
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

void render() {	// this is where the image gets drawn
    int i;
    double llx, lly, urx, ury, div, del;
    double xmax, xmin, ymax, ymin;
    double x,y;
    double width, height;
    PLOTDAT *pd;
    DATUM *p;
    int debug=0;
    int penno;
    int symno;
    double ticklen;
    double tmp;

    xwin_size(&width, &height);

    llx=0.1*width; 
    lly=0.1*height;
    urx=0.95*width;
    ury=0.9*height;
    fontsize = (height+width)/50.0;
    ticklen = (height+width)/200.0;
    pad=fontsize/4.0;
    double alpha;

    // we allow each graph to be 5 units tall, and spacing between
    // graphs is one unit...

    // place for a title
    if (plots[0].title != NULL) {
	xwin_set_pen_line(0,0);
	do_note(plots[0].title, (llx+urx)/2.0, pad+ury, MIRROR_OFF, fontsize, 1.28, 0.0, 0.0, 0, 1);
    }
    if (plots[0].xaxis != NULL) {
       xwin_set_pen_line(0,0);
       axislabel(&plots[numplots],plots[0].xaxis,1);
    }

    for (i=0; i<=numplots; i++) {
      pd = &(plots[numplots-i]); 		// compute bounding box for each graph 
      setbounds(pd);
      if (i==0) {
	  xmin=pd->xmin;
	  xmax=pd->xmax;
	  ymin=pd->ymin;
	  ymax=pd->ymax;
      } else {
	  if(xmin > pd->xmin) xmin=pd->xmin;
	  if(xmax < pd->xmax) xmax=pd->xmax;
	  if(ymin > pd->ymin) ymin=pd->ymin;
	  if(ymax < pd->ymax) ymax=pd->ymax;
      }
    }

    for (i=0; i<=numplots; i++) {
      pd = &(plots[numplots-i]); 		// compute bounding box for each graph 
      div=5.0*(numplots+1.0)+(numplots);	// number of vertical divisions
      del=(ury-lly)/div;

      pd->llx = llx;
      pd->lly = (lly+6.0*del*(double)i);
      pd->urx = urx;
      pd->ury = (lly+(5.0+6.0*(double)i)*del);

      if (debug) printf("numplots = %d: %g %g %g %g\n", numplots, pd->llx, pd->lly, pd->urx, pd->ury);

      xwin_set_pen_line(0,0);
      xwin_draw_box(pd->llx, pd->lly, pd->urx, pd->ury);	// plot boundary

      loose_label(pd, pd->ymin, pd->ymax, 9, 0);		// each graph with own y
      loose_label(pd, xmin, xmax, 9, 1);			// all graphs with same x 

      if (pd->yaxis != NULL) {
  	  xwin_set_pen_line(0,0);
          axislabel(pd,pd->yaxis,0);
      }

      back(0);		
      jump();	
      pen(1);		// select red pen
      symbol(0);	// select first symbol
      autopenflag=1;
      autosymflag=1;
      gridmode=1;
      symbolmode=0;
      linemode=1;
      symbolsize=1.0;

      for (p=pd->data; p!=(DATUM *)0; p=p->next) {
  	 if (p->cmd == NULL) {
	    // FIXME - needs to use actual xmax/min chosen by loose_label...
	    x =(pd->urx-pd->llx)*(p->x-xmin)/(xmax-xmin)+pd->llx;
	    y =(pd->ury-pd->lly)*(p->y-pd->ymin)/(pd->ymax-pd->ymin)+pd->lly; 
    	    if (symbolmode) {
	    	do_symbol(symnum, x, y, fontsize*symbolsize);
	    } 
	    if (linemode) {
		draw(x,y);
	    }
	 } else {
	   if (strncmp(p->cmd,"jump",4)==0) {
	       jump();
	   } else if (strncmp(p->cmd,"pen",3)==0) {
	       if (sscanf(p->cmd, "%*s %d", &penno)==1) {
		  pen(penno);
	       } else {
       		  pen(++pennum);
	       }
	   } else if (strncmp(p->cmd,"nogrid",6)==0) {
	       grid(0);
	   } else if (strncmp(p->cmd,"grid",4)==0) {
	       grid(1);
	   } else if (strncmp(p->cmd,"autopen",7)==0) {
	       autopenflag=1;
	   } else if (strncmp(p->cmd,"noautopen",9)==0) {
	       autopenflag=0;
	   } else if (strncmp(p->cmd,"autosymbol",10)==0) {
	       autosymflag=1;
	   } else if (strncmp(p->cmd,"noautosymbol",12)==0) {
	       autosymflag=0;
	   } else if (strncmp(p->cmd,"back",4)==0) {
	       printf("calling back1\n");
	       back(1);
	   } else if (strncmp(p->cmd,"symbol+line",11)==0) {
		symbolmode = 1;
		linemode = 1;
	   } else if (strncmp(p->cmd,"symbolsize",10)==0) {
	       if (sscanf(p->cmd, "%*s %lg", &tmp)==1) {
		   if (tmp >= 0.1 && tmp <= 10.0) {
		       symbolsize=tmp;
		   } else {
		      fprintf(stderr,"bad argument to symbolsize cmd: %s\n", p->cmd);
		   }
	       } else {
		   fprintf(stderr,"bad argument to symbolsize cmd: %s\n", p->cmd);
	       }
	   } else if (strncmp(p->cmd,"symbol",6)==0) {	// must follow symbol+line...
	       if (!symbolmode || !linemode) {
		   symbolmode = 1;
		   linemode = 0;
	       }
	       if (sscanf(p->cmd, "%*s %d", &symno)==1) {
		  symbol(symno);
	       } else {
		  symbol(++symnum);
	       }
	   } else if (strncmp(p->cmd,"nosymbol",8)==0) {
		symbolmode = 0;
		linemode = 1;
	   } else if (strncmp(p->cmd,"noline",6)==0) {
		symbolmode = 1;
		linemode = 0;
	   } else if (strncmp(p->cmd,"noback",6)==0) {
	       printf("calling back0\n");
	       back(0);
	   } 
	 }
      }
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
    s[0]=(char) ((c%54)+33);
    s[1]='\0';
    do_note(s, x, y, MIRROR_OFF , size, 1.0, 0.0, 0.0, 1, 4);
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
    // printf("do_note called with string %s, (%g,%g), m:%d sl:%g a:%g sz:%g r:%g, id:%d, jf:%d\n",
    //		string, x, y, mirror, size, aspect, slant, rotation, id, jf);

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
   backstat = 0;
}

void symbol(int sym_no) {
   symnum = sym_no%54;
}

void pen(int pen_no) {
   pennum = pen_no%6;
   xwin_set_pen_line(pennum,linenum);
}

void fontjump() {
   fontnsegs=0;
}

void fontdraw(double x, double y) {
    static double xxold, yyold;
    fontnsegs++;
    if (fontnsegs > 1) {
	xwin_draw_line(xxold, yyold, x, y);
    }
    xxold=x; yyold=y;
}

void jump() {
   nsegs=0;
}

void draw(double x, double y) {
    static double xold, yold;
    if (!backstat && nsegs > 1 && x<xold) {
       nsegs=0;
       if (autopenflag)	pen(++pennum);
       if (autosymflag) symbol(++symnum);
    }
    nsegs++;
    if (nsegs > 1) {
	xwin_draw_line(xold, yold, x, y);
    }
    xold=x; yold=y;
}
