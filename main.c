#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include "points.h"
#include "xwin.h"
#include "readfont.h"
#include "symbol.h"
#include "postscript.h"

// now that I'm running on x86-ia64 this next line isn't needed anymore
// int isblank(int c);	// for some reason, this was missing in ctype.h...

#define BUF_SIZE 1000
char *progname;

// get a line of no more than n chars
// return 0 on success, EOF on end of file
// chop of any trailing newline

int getz(char *s, int n) {	
    return procXevent( s, n);
}

// return a pointer to the first 
// non-blank character in s
char *skipblanks(char *s) {
   while (isblank(*s) && (*s != '\0')) {
      s++;
   }
   return s;
}

int main(int argc, char *argv[]) {
    int n;
    double x,y;
    char s[BUF_SIZE];
    char scratch[BUF_SIZE];
    char scratch2[BUF_SIZE];
    char *sp;
    int line=0;
    double xmin,xmax,ymin,ymax;
    double tmp;
    int itmp;
    int opt;

    progname = argv[0];
    setvbuf(stdin, NULL, _IONBF, 0);  // make stdin unbuffered 

    while ((opt=getopt(argc, argv, "D:")) != -1) {
        switch (opt) {
	case 'D':
	    // DISPLAY string is in optarg;
	    break;
	//case 'r':
	    // com_ci(optarg); // open rawfile 
	    // break;
	default:	/* '?' */
	   fprintf(stderr, "usage: %s [-D display] <script>\n", argv[0]);
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
    initsym();
    initplot();

    if (access("./NOTEDATA.F", R_OK) == 0) {
	loadfont("./NOTEDATA.F", 0);
    } else if (access("/usr/local/lib/pdplot/NOTEDATA.F", R_OK) == 0) {
	loadfont("/usr/local/lib/pdplot/NOTEDATA.F", 0);
    } else {
        fprintf(stderr,"can't find NOTEDATA.F font file\n");
	exit(0);
    }

#ifdef DEBUG
    if (access("./SYMBOL.F", R_OK) == 0) {
	loadfont("./SYMBOL.F", 1);
    } else
#endif
    if (access("/usr/local/lib/pdplot/SYMBOL.F", R_OK) == 0) {
	loadfont("/usr/local/lib/pdplot/SYMBOL.F", 1);
    } else {
        fprintf(stderr,"can't find SYMBOL.F font file\n");
	exit(0);
    }

  while (1) {
    while(getz(s,BUF_SIZE) != EOF) {
       line++;
       sp=skipblanks(s);
       if (isalpha(*sp)) {		// first non-white char is alpha, so a cmd
	   if (strncmp(sp,"nextygraph",10)==0) {
	       nextygraph();
	   } else if (strncmp(sp,"noframe",7)==0) {
	       gridstate(0);
	       box(0);
	       scale(0);
	       tick(0);
   	   } else if (strncmp(sp,"framepen",8)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  frame_pen(itmp); 
	       } else { 
		  fprintf(stderr, "bad framepen value: %s\n", s);
	       }
   	   } else if (strncmp(sp,"gridpen",7)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  gridpen(itmp); 
	       } else { 
		  fprintf(stderr, "bad grid value: %s\n", s);
	       }
   	   } else if (strncmp(sp,"xgridpen",8)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  xgridpen(itmp); 
	       } else {
		  fprintf(stderr, "bad xgrid value: %s\n", s);
	       }
   	   } else if (strncmp(sp,"ygridpen",8)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  ygridpen(itmp); 
	       } else { 
		  fprintf(stderr, "bad ygrid value: %s\n", s);
	       }
   	   } else if (strncmp(sp,"xgrid ",6)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  xgrid(itmp); 
	       } else { 
		  fprintf(stderr, "bad xgrid value: %s\n", s);
	       }
   	   } else if (strncmp(sp,"ygrid ",6)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  ygrid(itmp); 
	       } else { 
		  fprintf(stderr, "bad ygrid value: %s\n", s);
	       }
   	   } else if (strncmp(sp,"grid ",5)==0) {
       	       if (sscanf(s, "%*s %d", &itmp)==1) { 
	       	  grid(itmp); 
	       } else { 
		  fprintf(stderr, "bad grid value: %s\n", s);
	       }
	   } else if (strncmp(sp,"brightgrid",10)==0) {
	       gridpen(1);
	   } else if (strncmp(sp,"dimgrid",7)==0) {
	       gridpen(12);
	   } else if (strncmp(sp,"linxy",4)==0) {
	       logmode(0,0);
	       logmode(0,0);
	       logmode(1,0);
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
	       gridstate(1);
	   } else if (strncmp(sp,"nogrid",6)==0) {
	       gridstate(0);
	   } else if (strncmp(sp,"box",3)==0) {
	       box(1);
	   } else if (strncmp(sp,"nobox",5)==0) {
	       box(0);
	   } else if (strncmp(sp,"noisotropic",11)==0) {
	       iso(0,1.0);
	   } else if (strncmp(sp,"isotropic",9)==0) {
	       if (sscanf(sp+9,"%lg", &tmp) != 1) {
		   iso(1,1.0);
	       } else {
		   iso(1,tmp);
	       }
	   } else if (strncmp(sp,"plot",4)==0) {
	       need_redraw++;
	       xwin_top();
	   } else if (strncmp(sp,"clear",5)==0) {
	       initplot();	
	   } else if (strncmp(sp,"xscaletol",9)==0) {
	       if (sscanf(sp,"%*s %lg", &tmp) != 1 || tmp >= 0.4 || tmp < -1.0) {
	          fprintf(stderr, "bad xscaletol values: %s\n",sp);
	       } else {
		  xscaletol(tmp);	
	       }
	   } else if (strncmp(sp,"yscaletol",9)==0) {
	       if (sscanf(sp,"%*s %lg", &tmp) != 1 || tmp >= 0.4 || tmp < -1.0) {
	          fprintf(stderr, "bad yscaletol values: %s\n",sp);
	       } else {
		  yscaletol(tmp);	
	       }
	   } else if (strncmp(sp,"scaletol",8)==0) {
	       if (sscanf(sp,"%*s %lg", &tmp) != 1 || tmp >= 0.4 || tmp < -1.0) {
	          fprintf(stderr, "bad scaletol values: %s\n",sp);
	       } else {
		  scaletol(tmp);	
	       }
	   } else if (strncmp(sp,"ticklength",10)==0) {
	       if (sscanf(sp+10,"%lg", &tmp) != 1 || tmp < 0.1 || tmp > 10.0) {
		  tickset(-1.0);
	       } else {
		   tickset(tmp);	
	       }
	   } else if (strncmp(sp,"xset",4)==0) {
	       if (sscanf(sp+4,"%lg %lg", &xmin, &xmax ) != 2 || xmin > xmax) {
	          fprintf(stderr, "bad xset values: %s\n",sp);
	       } else {
	           // fprintf(stderr, "xset values: %g %g\n", xmin, xmax);
		   xset(xmin, xmax);	
	       }
	   } else if (strncmp(sp,"yset",4)==0) {
	       if (sscanf(sp+4,"%lg %lg", &ymin, &ymax ) != 2 || ymin > ymax) {
	          fprintf(stderr, "bad yset values: %s\n",sp);
	       } else {
	           // fprintf(stderr, "yset values: %g %g\n", ymin, ymax);
		   yset(ymin, ymax);	
	       }
	   } else if (strncmp(sp,"xscale",6)==0) {
	       if (sscanf(sp,"xscale %lg %[^#]", &tmp, scratch ) == 2) {
	       	  xscale(scratch,tmp);	
	       } else if (sscanf(sp,"xscale %[^#]", scratch ) == 1) {
	       	  xscale(scratch,1.0);	
	       } else {
		  fprintf(stderr, "bad xscale values: %s\n", sp);
	       }
	   } else if (strncmp(sp,"yscale",6)==0) {
	       if (sscanf(sp,"yscale %lg %[^#]", &tmp, scratch ) == 2) {
	       	  yscale(scratch,tmp);	
	       } else if (sscanf(sp,"yscale %[^#]", scratch ) == 1) {
	       	  yscale(scratch,1.0);	
	       } else {
		  fprintf(stderr, "bad yscale values: %s\n", sp);
	       }
	   } else if (strncmp(sp,"titlesize",9)==0) {	
	       savecmd(sp); 	// prevent "title" from eating titlesize
	   } else if (strncmp(sp,"title",5)==0) {
	       title(sp+6);
	   } else if (strncmp(sp,"style",5)==0) {
       	       sp=skipblanks(sp+6);
	       if (sp[0] == 'p') {
	       	  gridstate(0); 
		  setcharsize(1.6);
	       } else if (sp[0] == 'w') {
	          gridstate(1);
		  setcharsize(1.0);
	       }
	   } else if (strncmp(sp,"graph",5)==0) {
	       if (sscanf(sp,"graph %s", scratch) == 1) {
		   sprintf(scratch2, "pnmtopng > %s.png", scratch);
		   xwin_dump_graphics(scratch2);
	       } else {
		   xwin_dump_graphics("pnmtopng > pddump.png");
	       }
	   } else if (strncmp(sp,"post",4)==0) {
	       ps_set_outputtype(POSTSCRIPT);
	       if (sscanf(sp,"post %s", scratch) == 1) {
		   sprintf(scratch2, "cat > %s.ps", scratch);
		   xwin_dump_postscript(scratch2);
	       } else {
		   xwin_dump_postscript("cat > pddump.ps");
	       }
	   } else if (strncmp(sp,"svg",3)==0) {
	       ps_set_outputtype(SVG);
	       if (sscanf(sp,"svg %s", scratch) == 1) {
		   sprintf(scratch2, "cat > %s.svg", scratch);
		   xwin_dump_postscript(scratch2);
	       } else {
		   xwin_dump_postscript("cat > pddump.svg");
	       }
	   } else if (strncmp(sp,"dxf",3)==0) {
	       ps_set_outputtype(DXF);
	       if (sscanf(sp,"dxf %s", scratch) == 1) {
		   sprintf(scratch2, "cat > %s.dxf", scratch);
		   xwin_dump_postscript(scratch2);
	       } else {
		   xwin_dump_postscript("cat > pddump.dxf");
	       }
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
    usleep(500);
  }
  exit(1);
}
