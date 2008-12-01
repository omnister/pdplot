#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "xwin.h"
#include "points.h"
#include "readfont.h"
#include <unistd.h>

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
    char scratch[BUF_SIZE];
    char *sp;
    int line=0;
    progname = argv[0];
    double xmin,xmax,ymin,ymax;
    double tmp;
    int opt;

    setvbuf(stdin, NULL, _IONBF, 0);  // make stdin unbuffered 

    while ((opt=getopt(argc, argv, "")) != -1) {
        switch (opt) {
	//case 'r':
	// com_ci(optarg); // open rawfile 
	// break;
	default:	/* '?' */
	   fprintf(stderr, "usage: %s [-r <rawfile>] <script>\n", argv[0]);
	   exit(1);
	}
    }

    if (optind >= argc) {	/* fake an argument list */
	// static char *stdinonly[] = { "-" };
	// gargv = stdinonly;
	// gargc = 1;
    } else {
	// gargv = &argv[optind];
	// gargc = --argc;
    }

    initX();
    initplot();

    loadfont("./NOTEDATA.F", 0);
    loadfont("./SYMBOL.F", 1);

  while (1) {
    while(getz(s,BUF_SIZE) != EOF) {
       line++;
       sp=skipblanks(s);
       if (isalpha(*sp)) {		// first non-white char is alpha, so a cmd
	   if (strncmp(sp,"nextygraph",10)==0) {
	       nextygraph();
	   } else if (strncmp(sp,"linx",4)==0) {
	       logmode(0,0);
	   } else if (strncmp(sp,"liny",4)==0) {
	       logmode(1,0);
	   } else if (strncmp(sp,"logxy",5)==0) {
	       logmode(0,1);
	       logmode(1,1);
	   } else if (strncmp(sp,"logx",4)==0) {
	       logmode(0,1);
	   } else if (strncmp(sp,"logy",4)==0) {
	       logmode(1,1);
	   } else if (strncmp(sp,"loglog",6)==0) {
	       logmode(0,1);
	       logmode(1,1);
	   } else if (strncmp(sp,"dbxy",4)==0) {
	       logmode(0,2);
	       logmode(1,2);
	   } else if (strncmp(sp,"dbx",3)==0) {
	       logmode(0,2);
	   } else if (strncmp(sp,"dby",3)==0) {
	       logmode(1,2);
	   } else if (strncmp(sp,"dbpxy",5)==0) {
	       logmode(0,3);
	       logmode(1,3);
	   } else if (strncmp(sp,"dbpx",4)==0) {
	       logmode(0,3);
	   } else if (strncmp(sp,"dbpy",4)==0) {
	       logmode(1,3);
	   } else if (strncmp(sp,"dump",4)==0) {
	       dumppoints();
	   } else if (strncmp(sp,"grid",4)==0) {
	       grid(1);
	   } else if (strncmp(sp,"nogrid",6)==0) {
	       grid(0);
	   } else if (strncmp(sp,"box",3)==0) {
	       box(1);
	   } else if (strncmp(sp,"nobox",5)==0) {
	       box(0);
	   } else if (strncmp(sp,"plot",4)==0) {
	       need_redraw++;
	       xwin_top();
	   } else if (strncmp(sp,"clear",5)==0) {
	       initplot();	// FIXME: needs to set all globals: grid, frame ...
	   } else if (strncmp(sp,"xset",4)==0) {
	       if (sscanf(sp+4,"%lg %lg", &xmin, &xmax ) != 2 || xmin > xmax) {
	          fprintf(stderr, "bad xset values: sp\n");
	       } else {
	           // fprintf(stderr, "xset values: %g %g\n", xmin, xmax);
		   xset(xmin, xmax);	
	       }
	   } else if (strncmp(sp,"yset",4)==0) {
	       if (sscanf(sp+4,"%lg %lg", &ymin, &ymax ) != 2 || ymin > ymax) {
	          fprintf(stderr, "bad yset values: sp\n");
	       } else {
	           // fprintf(stderr, "yset values: %g %g\n", ymin, ymax);
		   yset(ymin, ymax);	
	       }
	   } else if (strncmp(sp,"xscale",6)==0) {
	       if (sscanf(sp,"xscale %lg %s", &tmp, scratch ) != 2) {
	          fprintf(stderr, "bad xscale values: sp\n");
	       } else {
	       	  xscale(scratch,tmp);	
	       }
	   } else if (strncmp(sp,"yscale",6)==0) {
	       if (sscanf(sp,"yscale %lg %s", &tmp, scratch ) != 2) {
	          fprintf(stderr, "bad yscale values: sp\n");
	       } else {
	       	  yscale(scratch,tmp);	
	       }
	   } else if (strncmp(sp,"title",5)==0) {
	       title(sp+6);
	   } else if (strncmp(sp,"exit",4)==0) {
	       exit(2);
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
    ungetc(procXevent(), stdin);
    sleep(1);
  }
    while(1) {		// no more input, but keep X alive
       procXevent();
    }
    exit(1);
}
