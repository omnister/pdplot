/* 
 * label.c: nice graph labeling
 *     adapted from from:
 *     "Graphic Gems in C"
 *     Paul Heckbert 2 Dec 88
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "points.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

static double nicenum();

// nticks corresponds to maxxdiv, maxydiv in autoplot

void loose_label(PLOTDAT *pd, double *min, double *max, int nticks, int axis, int dolabel,
int logmode, int dryrun) {
    char str[6], temp[20];
    // int nfrac;
    double d;	/* tick mark spacing */
    double graphmin, graphmax;
    double range, x;

    if (logmode != 1) {	// just do linear scaling

	range = nicenum(*max-*min, 0); 	/* we expect min!=max */
	d = nicenum(range/(double)(nticks),1);
	if (dryrun) {
	    graphmin = floor(*min/d)*d;
	    graphmax = ceil(*max/d)*d;
	} else {
	    graphmin = *min;
	    graphmax = *max;
	}
	// nfrac = MAX(-floor(log10(d)),0);	/* # frac digits to show */
	// sprintf(str,"%%.%df", nfrac); 	/* simplest axis labels */
	sprintf(str,"%%g"); 	

	if (!dryrun) {
	    // printf("graphmin=%g graphmax=%g increment=%g\n", graphmin, graphmax, d);
	    for (x=graphmin+d; x<graphmax-0.5*d; x+=d) {
		gridline(pd, (x-graphmin)/(graphmax-graphmin), axis);
	    }
	    for (x=graphmin; x<graphmax+0.5*d; x+=d) {
		x  = (d/100.0)*rint(x/(d/100.0));
		sprintf(temp, str, x);
		if (dolabel) {
		    gridlabel(pd, temp, (x-graphmin)/(graphmax-graphmin), axis);
		}
	    }
	}
	*min  = graphmin;
	*max  = graphmax;

    } else {	// log mode, so do scaling by decades

	if (dryrun) {
	    graphmin = floor(*min);
	    graphmax =  ceil(*max);
	} else {
	    graphmin = *min;
	    graphmax = *max;
	}

	if (!dryrun) {
	    for (x=graphmin+1; x<graphmax-0.5; x++) {
		gridline(pd, (x-graphmin)/(graphmax-graphmin), axis);
	    }
	    for (x=graphmin; x<graphmax+0.5; x++) {
		sprintf(temp, "%g", pow(10.0,x));
		if (dolabel) {
		    gridlabel(pd, temp, (x-graphmin)/(graphmax-graphmin), axis);
		}
	    }
	}

	*min = graphmin;
	*max = graphmax;
    }
}

/*
 * nicenum: find a "nice" number approximately equal to x.
 * Round the number if round=1, take ceiling if round=0
 */

static double nicenum(x,round)
double x;
int round;
{
    int exp;	/* exponent of x */
    double f;	/* fractional part of x */
    double nf;	/* nice, rounded fraction */


    exp = floor(log10(x));
    f = x/pow(10.0, (double) exp);	/* between 1 and 10 */

    if (round) {
        if (f<1.5) nf = 1.0;
	else if (f<3.0) nf = 2.0;
	else if (f<7.0) nf = 5.0;
	else nf = 10.0;
    } else {
        if (f<1.0) nf = 1.0;
	else if (f<2.0) nf = 2.0;
	else if (f<5.0) nf = 5.0;
	else nf = 10.0;
    }

    return nf*pow(10.0, (double) exp);
}

#define NTICK 9	/* desired number of tick marks */

void testmain(int ac, char ** av) {
    double min, max;
    PLOTDAT foo;
    if (ac!=3) {
    	fprintf(stderr,"usage: label <min> <max>\n");
	exit(1);
    }
    min = atof(av[1]);
    max = atof(av[2]);
    loose_label(&foo, &min,&max,NTICK,1,1,0,0);
}
