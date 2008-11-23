#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "xwin.h"
#include "points.h"
#include "readfont.h"

#define BUF_SIZE 1000
char *progname;

int isblank(int c);	// for some reason, this is missing in ctype.h...

// get a line of no more than n chars
// return 0 on success, EOF on end of file
// chop of any trailing newline

int getz(char *s, int n) {	
    int c;
    int i=0;
    while((c=procXevent())!='\n' && i++<n && c!=EOF) {
       *(s++)=c;
    }
    *s='\0';
    if (c==EOF) {
       return(EOF);
    } else {
       return(0);
    }
}

// return a pointer to the first 
// non-blank character in s
char *skipblanks(char *s) {
   while (isblank(*s) && s!='\0') {
      s++;
   }
   return(s);
}

int main(argc,argv)
int argc;
char **argv;
{
    int n;
    double x,y;
    char s[BUF_SIZE];
    char *sp;
    int line=0;
    progname = argv[0];
    double xmin,xmax,ymin,ymax;

    setvbuf(stdin, NULL, _IONBF, 0);  // make stdin unbuffered 

    initX();
    initplot();

    loadfont("./NOTEDATA.F", 0);
    loadfont("./SYMBOL.F", 1);

    while(getz(s,BUF_SIZE) != EOF) {
       line++;
       sp=skipblanks(s);
       if (isalpha(*sp)) {		// first non-white char is alpha, so a cmd
	   if (strncmp(sp,"nextygraph",10)==0) {
	       nextygraph();
	   } else if (strncmp(sp,"dump",4)==0) {
	       dumppoints();
	   } else if (strncmp(sp,"plot",4)==0) {
	       need_redraw++;
	   } else if (strncmp(sp,"clear",5)==0) {
	       initplot();
	   } else if (strncmp(sp,"xset",4)==0) {
	       if (sscanf(sp+4,"%lg %lg", &xmin, &xmax ) != 2 || xmin > xmax) {
	          fprintf(stderr, "bad xset values: sp\n");
	       } else {
	           fprintf(stderr, "xset values: %g %g\n", xmin, xmax);
		   xset(xmin, xmax);	
	       }
	   } else if (strncmp(sp,"yset",4)==0) {
	       if (sscanf(sp+4,"%lg %lg", &ymin, &ymax ) != 2 || ymin > ymax) {
	          fprintf(stderr, "bad yset values: sp\n");
	       } else {
	           fprintf(stderr, "yset values: %g %g\n", ymin, ymax);
		   yset(ymin, ymax);	
	       }
	   } else if (strncmp(sp,"xscale",6)==0) {
	       xscale(sp+7,1.0);	// cheat on parsing scale for now
	   } else if (strncmp(sp,"yscale",6)==0) {
	       yscale(sp+7,1.0);	// cheat on parsing scale for now
	   } else if (strncmp(sp,"title",5)==0) {
	       title(sp+6);
	   } else {
	       savecmd(sp);
	   }
       } else if (*sp == '#') {		// else, might be a comment line
       	   ; 
       } else {				// nothing left but data point
           n=sscanf(sp,"%lf %lf",&x,&y);
	   if (n==2) {
	      savepoint(x,y);
	   } else {
	      if (*sp != '\0' && *sp != '\n') {
		  fprintf(stderr,"%s: syntax error on line %d: \"%s\"\n", progname, line,s);
	      }
	   }
       }
    }
    
    need_redraw++;

    while(1) {		// no more input, but keep X alive
       procXevent();
    }
    exit(1);
}
