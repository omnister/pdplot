#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "points.h"
#include "readfont.h"
#define MAXPOINT 5000

/* 

The TEXT and FONT files are named "TEXTDATA.F" and "NOTEDATA.F"
respectively.  The format for both files is plain ASCII.  The file
starts with two integers separated by a comma.  The first gives the
height of each character definition and the second, the width.  Each
character is then listed in printed form and the glyph is defined by a
set of vector coordinates.  It is assumed that each successive x,y
coordinate pair is connected by a stroke unless the path is broken by
the special coordinate "-64,0".  Each definition is terminated by the
special coordinate "-64,-64".  Here is an example of the first three
definitions in "NOTEDATA.F" for the characters <exclam>, <double-quote>,
and <pound>:

16,12
! 6,2 6,3 -64,0 6,5 6,14 -64,-64
" 4,14 4,10 -64,0 8,14 8,10 -64,-64
# 2,6 10,6 -64,0 2,10 10,10 -64,0 4,2 4,14 -64,0 8,2 8,14 -64,-64

*/


int getxy();
int eatwhite();
int getint();
int linecnt=1;

int dx[2];		/* size of font cell */
int dy[2];		/* size of font cell */	
int fonttab[2][256];

int xdef[2][MAXPOINT];
int ydef[2][MAXPOINT];

int fillable[2] = {0,1};

void writechar(int c,double x,double y,XFORM *xf,int id)
{
    int i;
    double xp,yp,xt,yt;

    /* printf("# %c %d\n",c); */
    
    if (c==' ') {
	return;
    }

    i = fonttab[id][c];

    fontjump();
    while (xdef[id][i] != -64 || ydef[id][i] != -64) {		/* -64,-64 == END */
	if (xdef[id][i] != -64) {				/* end of polygon */
	    xp = x + (0.8 * ( (double) xdef[id][i] / (double) dy[id]));
	    yp = y + (0.8 * ( (double) ydef[id][i] / (double) dy[id]));
	    if (id) yp+=.1;	// correct baseline between two fonts
	    xt = xp*xf->r11 + yp*xf->r21 + xf->dx;
	    yt = xp*xf->r12 + yp*xf->r22 + xf->dy;
	    fontdraw(xt,yt); 
	} else {
	    fontjump();
	}
	i++;
    }
}

int chartoindex(int c) {
   switch (c) {
   case 'D': return(90);
   case 'O': return(101);
   case 'F': return(92);
   case 'G': return(93);
   case 'Y': return(108);
   case 'I':			// iota
   case 'C': return(96);
   case 'L': return(98);
   case 'W': return(109);
   case 'P': return(102);
   case 'H': return(103);
   case 'S': return(105);
   case 'X': return(110);
   case 'Q': return(111);
   case 'a': return(119);
   case 'b': return(120);
   case 'd': return(122);
   case 'e': return(123);
   case 'f': return(124);
   case 'g': return(125);
   case 'y': return(126);
   case 'i': return(127);
   case 'c': return(128);
   case 'k': return(129);
   case 'l': return(130);
   case 'm': return(131);
   case 'n': return(132);
   case 'o': return(133);
   case 'p': return(134);
   case '@': return(135);
   case 'r': return(136);
   case 's': return(137);
   case 't': return(138);
   case 'u': return(140);
   case 'w': return(141);
   case 'x': return(142);
   case 'q': return(143);
   case 'z': return(144);
   default: return(32);
   };
}

void writestring(char *s, XFORM *xf, int id, int jf)
{
    double yoffset=0.0;
    int debug=0;
    double xoffset=0.0;
    double xoff, yoff;
    
    double width=(((double)(dx[id]))*0.80*strlen(s))/((double)(dy[id]));
    double height=0.8;
    xoff = yoff = 0.0;

    switch (jf) {	// justification
        case 0:		/* SW */
	    xoff = 0.0;
	    yoff = 0.0;
	    break;
        case 1:		/* S */
	    xoff = -width/2.0;
	    yoff = 0.0;
	    break;
        case 2:		/* SE */
	    xoff = -width;
	    yoff = 0.0;
	    break;
        case 3:		/* W */
	    xoff = 0.0;
	    yoff = -height/2.0;
	    break;
        case 4:		/* C */
	    xoff = -width/2.0;
	    yoff = -height/2.0;
	    break;
        case 5:		/* E */
	    xoff = -width;
	    yoff = -height/2.0;
	    break;
        case 6:		/* NW */
	    xoff = 0.0;
	    yoff = -height;
	    break;
        case 7:		/* N */
	    xoff = -width/2.0;
	    yoff = -height;
	    break;
        case 8:		/* NE */
	    xoff = -width;
	    yoff = -height;
	    break;
	default:
	    printf("bad justification: %d in writestring()\n", jf);
	    break;
    }

    if (debug) printf("in writestring id=%d\n", id);

    /* void writechar(c,x,y,xf,id) */

    /*
      autoplot formatting escape commands:
      [x] backspace one char  \<
      [x] forward one char    \>
      [ ] begin subscript     \[
      [ ] end subscript       \]
      [ ] begin superscript   \{
      [ ] end superscript     \}
    */

    while(*s != 0) {
	if (*s == '\n') {
	    xoffset=0.0;
	    yoffset+=1.0; 
	} else if (*s == '\\' && *(s+1) == '<') {	// backspace
	    xoffset-=1.0;
	    ++s;
	} else if (*s == '\\' && *(s+1) == '>') {	// fwdspace
	    xoffset+=1.0;
	    ++s;
	} else if (*s == '\\' && *(s+1) == '{') {	// start super
	    yoffset-=0.5;
	    ++s;
	} else if (*s == '\\' && *(s+1) == '}') {	// stop super
	    yoffset+=0.5;
	    ++s;
	} else if (*s == '\\' && *(s+1) == '[') {	// start sub
	    yoffset+=0.5;
	    ++s;
	} else if (*s == '\\' && *(s+1) == ']') {	// stop sub
	    yoffset-=0.5;
	    ++s;
	} else if (*s == '\\' && *(s+1) == '\\') {	// literal backslash
	    writechar('\\',
	        (((double)(dx[id]))*0.80*xoffset)/((double)(dy[id]))+xoff,
		-yoffset+yoff,xf,id);
	    xoffset+=1.0;
	    ++s;
	} else if (*s == '\\' && *(s+1)!='\0') {	// possible greek
	    // printf("inside greek, char = %c, index=%d\n", *(s+1), chartoindex(*(s+1)));
	    writechar(chartoindex(*(s+1)),
	        (((double)(dx[0]))*0.80*xoffset)/((double)(dy[0]))+xoff,
		-yoffset+yoff,xf,1);
	    xoffset+=1.0;
	    ++s;
	} else {
	    writechar(*s,
	        (((double)(dx[id]))*0.80*xoffset)/((double)(dy[id]))+xoff,
		-yoffset+yoff,xf,id);
	    if (debug) printf("writing %c, dx:%d dy:%d id:%d\n",
	    	*s, dx[id], dy[id], id);
	    xoffset+=1.0;
	}
	++s;
    }
}

void loadfont(char *file, int id)
{
    FILE *fp;
    int i;
    int x;
    int y;
    int done;
    int next;
    int lit;
    extern int linecnt;
    int index=0;	/* index into font table */
    int debug=0;

    if (debug) printf("opening %s\n", file);

    /* initialize font table */
    for (i=0; i<MAXPOINT; i++) {
	xdef[id][i] = ydef[id][i] = -64;
    }
    for (i=0; i<=255; i++) {
	fonttab[id][i]=-1;
    }

    if((fp=fopen(file,"r")) == NULL) {
	fprintf(stderr, "readfont error: could not fopen %s\n",file);
	exit(1);
    } else {
    	// printf("loading %s\n",file);
    }

    linecnt=0;
    done=0;

    /* note reversed order of arguments */

    next=getxy(fp,&dy[id],&dx[id]);
    if (debug) printf("got %d, %d next=%x id=%d\n", dx[id],dy[id],next, id); 

    /* make first line look properly terminated */
    x=-64;
    y=-64;

    while (!done) {
	if (next == '\n') {
	    linecnt++;
	    getc(fp);
	    if (x==-64 && y==-64) {
		if ((lit=eatwhite(fp)) != EOF) {
		    if (debug) printf("lit=%c\n",lit);
		    getc(fp);
		    fonttab[id][(int) lit] = index;
		} else {
		    done++;
		}
	    }
	} else if (next == EOF) {
	    done++;
	} 

	if (!done) {
	    next=getxy(fp,&x,&y);
	    if (debug) printf("line %d: got %d, %d next=%c\n", linecnt, x,y,next); 
	    xdef[id][index] = x;
	    ydef[id][index] = y;
	    index++;
	}
    }
    if (debug) printf("index = %d\n", index);
}


int getxy(fp,px,py)
FILE *fp;
int *px;
int *py;
{
    int c;
    extern int linecnt;

    c=eatwhite(fp);
    /* printf("eating white, next=%c\n",c); */
    if(getint(fp,px) != 1) {
	fprintf(stderr,"readfont error at line %d: expected a digit\n", linecnt);
	exit(3);
    };		

    eatwhite(fp);
    if ((c=getc(fp)) != ',') {
	ungetc(c,fp);
	/* make comma optional to read graffy fonts */
	/* fprintf(stderr,"readfont error at line %d: expected a comma\n", linecnt); exit(2); */
    }

    eatwhite(fp);
    if(getint(fp,py) != 1) {
	fprintf(stderr,"readfont error at line %d: expected a digit\n", linecnt);
	exit(3);
    };		

    return(eatwhite(fp));
}

int eatwhite(fp)
FILE *fp;
{
    int c;
    int done=0;

    while (!done) {
	c=getc(fp);
	if (c != ' ' && c != '\t') {
	    done++;
	}
    }
    ungetc(c,fp);
    return(c);
}

/* parse an input of the form [+-][0-9]* */
/* using just getc(fp) and ungetc(c,fp) */
/* returns 0 if no digit found, 1 if successful */

int getint(fp,pi) 	
FILE *fp;
int *pi;
{
    int c;
    int state=0;
    int done=0;
    int sign=1;
    int val=0;
    int err=0;
    
    state=0;

    while (!done) {
	c = getc(fp);
	switch (state) {
	    case 0:	/* exponent sign */
		if (c=='+') {
		    state=1;
		} else if (c=='-') {
		    sign=-1;
		    state=1;
		} else if (isdigit(c)) {
		    ungetc(c,fp);
		    state=1;
		} else {
		    err++;
		    ungetc(c,fp);
		    done++;
		}
		break;
	    case 1:	/* first digit */
		if (isdigit(c)) {
		    val=10*val+(int) (c-'0');	
		    state=2;
		} else {
		    err++;
		    ungetc(c,fp);
		    done++;
		}
		break;
	    case 2:	/* remaining digits */
		if (isdigit(c)) {
		    val=10*val+(int) (c-'0');	
		} else {
		    ungetc(c,fp);
		    done++;
		}
		break;
	}
    }

    if (err) {
	return(0);
    } 

    *pi = val*sign;
    return(1);
}


