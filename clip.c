#include <stdio.h>


/* Cohen-Sutherland 2-D Clipping Algorithm */
/* See: Foley & Van Dam, p146-148 */

clip(x1, y1, x2, y2, xmin, ymin, xmax, ymax)
double x1, y1, x2, y2;
double xmin, ymin, xmax, ymax;
{

    int debug=0;
    int done=0;
    int accept=0;
    int code1=0;
    int code2=0;
    double tmp;

    if (x2 < x1) {	/* canonicalize the line */
	tmp = x2; x2 = x1; x1 = tmp;
    }
    if (y2 < y1) {
	tmp = y2; y2 = y1; y1 = tmp;
    }

    if (debug) printf("canonicalized: %g,%g %g,%g\n", x1, y1, x2, y2);

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
	    if (accept = !((code1 | code2))) {
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
	// xwin_drawline() here...
    	printf("%g %g\n", x1,y1);
    	printf("%g %g\n", x2,y2);
    	printf("jump\n");
    }
}

testmain() {
   char buf[128];
   double x1, y1, x2, y2;

   printf("back\n");

   printf("-1.2 -1.2\n");
   printf("1.2 -1.2\n");
   printf("1.2 1.2\n");
   printf("-1.2 1.2\n");
   printf("-1.2 -1.2\n");

   printf("jump\n");

   while (fgets(buf, 128, stdin) != 0) {
   	sscanf(buf, "%lf %lf %lf %lf", &x1, &y1, &x2, &y2);
	/* printf("got %g %g %g %g\n", x1, y1, x2, y2);  */
	clip(x1, y1, x2, y2, -1.0, -1.0, 1.0, 1.0);
   }
}

