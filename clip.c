#include <stdio.h>
#include "xwin.h"
#include "points.h"

double xmin = 0.0;
double xmax = 0.0;
double ymin = 0.0;
double ymax = 0.0;

/* Cohen-Sutherland 2-D Clipping Algorithm */
/* See: Foley & Van Dam, p146-148 */

void clip_set(xn, xx, yn, yx)
double xn, xx, yn, yx;
{
    xmin = xn;
    xmax = xx;
    ymin = yn;
    ymax = yx;
}

void updatebounds(pd, x1, y1, x2, y2)
PLOTDAT *pd;
double x1, y1, x2, y2;
{
    double xmin, xmax, ymin, ymax;

    xmin=x1; xmax=x2;
    if (x2 < x1) {
       xmin = x2;
       xmax = x1;
    }
    ymin=y1; ymax=y2;
    if (y2 < y1) {
       ymin = y2;
       ymax = y1;
    }

    if (pd->boundsflag == 0) {
       pd->bbxmin = xmin;
       pd->bbxmax = xmax;
       pd->bbymin = ymin;
       pd->bbymax = ymax;
    } else {
       if (xmin < pd->bbxmin) pd->bbxmin = xmin;
       if (xmax > pd->bbxmax) pd->bbxmax = xmax;
       if (ymin < pd->bbymin) pd->bbymin = ymin;
       if (ymax > pd->bbymax) pd->bbymax = ymax;
    }
    pd->boundsflag=1;
}

void clip(pd, x1, y1, x2, y2)
PLOTDAT *pd;
double x1, y1, x2, y2;
{

    extern double xmin, xmax, ymin, ymax;
    int debug=0;
    int done=0;
    int accept=0;
    int code1=0;
    int code2=0;
    double tmp;
    double xs1, xs2, ys1, ys2;

    while (!done) {
        /* compute "outcodes" */
	code1=0;
	if((ymax-y1) < 0) code1 += 1;
	if((y1-ymin) < 0) code1 += 2;
	if((xmax-x1) < 0) code1 += 4;
	if((x1-xmin) < 0) code1 += 8;

	code2=0;
	if((ymax-y2) < 0) code2 += 1;
	if((y2-ymin) < 0) code2 += 2;
	if((xmax-x2) < 0) code2 += 4;
	if((x2-xmin) < 0) code2 += 8;

	if (debug) printf("code1: %d, code2: %d\n", code1, code2);

    	if (code1 & code2) {
	    if (debug) printf("trivial reject\n");
	    done++;	/* trivial reject */
	} else { 
	    if ((accept = !((code1 | code2)))) {
		if (debug) printf("trivial accept\n");
	    	done++;
	    } else {
	        if (!code1) { /* x1,y1 inside box, so SWAP */
		    if (debug) printf("swapping\n");
		    tmp=y1; y1=y2; y2=tmp;
		    tmp=x1; x1=x2; x2=tmp;
		    tmp=code1; code1=code2; code2=tmp;
		}

		if (debug) printf("preclip: %g,%g %g,%g\n", x1, y1, x2, y2);
		if (code1 & 1) {		/* divide line at top */
			x1 = x1 + (x2-x1)*(ymax-y1)/(y2-y1);
                        y1 = ymax;
		} else if (code1 & 2) {	/* divide line at bot */
			x1 = x1 + (x2-x1)*(ymin-y1)/(y2-y1);
                        y1 = ymin;
		} else if (code1 & 4) {	/* divide line at right */
			y1 = y1 + (y2-y1)*(xmax-x1)/(x2-x1);
                        x1 = xmax;
		} else if (code1 & 8) {	/* divide line at left */
			y1 = y1 + (y2-y1)*(xmin-x1)/(x2-x1);
                        x1 = xmin;
                }
		if (debug) printf("after: %g,%g %g,%g\n", x1, y1, x2, y2);
	    }
	}
    }
    if (debug) printf("accept = %d\n", accept);
    if (accept) {
	updatebounds(pd, x1, y1, x2, y2);
	xs1 =(pd->urx-pd->llx)*(x1-xmin)/(xmax-xmin)+pd->llx;
        ys1 =(pd->ury-pd->lly)*(y1-pd->ymin)/(pd->ymax-pd->ymin)+pd->lly; 
	xs2 =(pd->urx-pd->llx)*(x2-xmin)/(xmax-xmin)+pd->llx;
        ys2 =(pd->ury-pd->lly)*(y2-pd->ymin)/(pd->ymax-pd->ymin)+pd->lly; 
	if (linenum != 0) {
	    xwin_draw_line(xs1,ys1,xs2,ys2);
	} else {
	    xwin_draw_point(xs1,ys1);
	    xwin_draw_point(xs2,ys2);
	}
    	// printf("%g %g\n", xs1,ys1);
    	// printf("%g %g\n", xs2,ys2);
    	// printf("jump\n");
    }
}

