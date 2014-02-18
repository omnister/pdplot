#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include "eventnames.h"
#include "xwin.h"
#include "points.h"
#include "postscript.h"

void load_font(XFontStruct **font_info);
void dbug(char *s, int dbug);
int init_colors();
void getGC(Window win, GC *gc, XFontStruct *font_info);
void doXevent(char *s);
void xwin_top();

#define MAX_COLORS 16
#define MAX_LINETYPE 7
unsigned long colors[MAX_COLORS];    /* will hold pixel values for colors */
int displayon=1;

#define icon_bitmap_width 20
#define icon_bitmap_height 20
static char icon_bitmap_bits[] = {
    0x60, 0x00, 0x01, 0xb0, 0x00, 0x07, 0x0c, 0x03, 0x00, 0x04, 0x04, 0x00,
    0xc2, 0x18, 0x00, 0x03, 0x30, 0x00, 0x01, 0x60, 0x00, 0xf1, 0xdf, 0x00,
    0xc1, 0xf0, 0x01, 0x82, 0x01, 0x00, 0x02, 0x03, 0x00, 0x02, 0x0c, 0x00,
    0x02, 0x38, 0x00, 0x04, 0x60, 0x00, 0x04, 0xe0, 0x00, 0x04, 0x38, 0x00,
    0x84, 0x06, 0x00, 0x14, 0x14, 0x00, 0x9c, 0x34, 0x00, 0x00, 0x00, 0x00};

#define BITMAPDEPTH 1
#define TOO_SMALL 0
#define BIG_ENOUGH 1

Display *dpy;
int screen_num;

int need_redraw=0;
char *progname;

#define BUF_SIZE 2000

Window win;
Drawable *dd=&win;
unsigned int width, height;
GC gc,gcx;

void initX() {
    int x,y;
    unsigned int border_width = 4;
    unsigned int display_width, display_height;
    char *window_name = "postplot";
    char *icon_name = "postplot";
    Pixmap icon_pixmap;
    XSizeHints *size_hints;
    XIconSize *size_list;
    XWMHints *wm_hints;
    XClassHint *class_hints;
    XTextProperty windowName, iconName;
    XSetWindowAttributes attr;
    int icount;
    XFontStruct *font_info;
    char *display_name = NULL;

    int dbug=0;

    if (!(size_hints = XAllocSizeHints())) {
        fprintf(stderr, "%s: failure allocating SizeHints memory\n", progname);
        exit (0);
    }
    if (!(wm_hints = XAllocWMHints())) {
        fprintf(stderr, "%s: failure allocating WMHints memory\n", progname);
        exit (0);
    }
    if (!(class_hints = XAllocClassHint())) {
        fprintf(stderr, "%s: failure allocating ClassHint memory\n", progname);
        exit (0);
    }

    /* Connect to X server */
    if ( (dpy=XOpenDisplay(display_name)) == NULL ) {
        (void) fprintf( stderr, "%s: cannot connect to X server %s\n",
            progname, XDisplayName(display_name));
        exit(-1);
    }

    /* Get screen size from display structure macro */
    screen_num = DefaultScreen(dpy);
    display_width = DisplayWidth(dpy, screen_num);
    display_height = DisplayHeight(dpy, screen_num);

    /* eventually we want to set x,y from command line or
       resource database */

    x=y=0;

    /* Size window */
    width = display_width/2, height = display_height/2;


    /* Create opaque window */
    win = XCreateSimpleWindow(dpy, RootWindow(dpy,screen_num),
        x,y,width, height, border_width, WhitePixel(dpy,screen_num),
        BlackPixel(dpy,screen_num));

    /* Get available icon sizes from window manager */

    if (XGetIconSizes(dpy, RootWindow(dpy, screen_num),
            &size_list, &icount) == 0) {
        if (dbug) fprintf(stderr, 
		"%s: WM didn't set icon sizes - using default.\n", progname);
    } else {
        /* should eventually create a pixmap here */
        ;
    }

    /* Create pixmap of depth 1 (bitmap) for icon */
    icon_pixmap = XCreateBitmapFromData(dpy, win,
        icon_bitmap_bits, icon_bitmap_width, icon_bitmap_height);
    
    size_hints->flags = PPosition | PSize | PMinSize;
    size_hints->min_width =  300;
    size_hints->min_height = 200;

    if (XStringListToTextProperty(&window_name, 1, &windowName) == 0) {
        fprintf(stderr, "%s: structure allocation for windowName failed.\n",
            progname);
        exit(-1);
    }

    if (XStringListToTextProperty(&icon_name, 1, &iconName) == 0) {
        fprintf(stderr, "%s: structure allocation for iconName failed.\n",
            progname);
        exit(-1);
    }
    
    wm_hints->initial_state = NormalState;
    wm_hints->input = True;
    wm_hints->icon_pixmap = icon_pixmap;

    wm_hints->flags = StateHint | IconPixmapHint | InputHint;

    class_hints->res_name = progname;
    class_hints->res_class = "pdp";

    XSetWMProperties(dpy, win, &windowName, &iconName,
             (char **) NULL, (int) 0, size_hints, wm_hints, class_hints);

    /* Select event types wanted */
    XSelectInput(dpy, win, ExposureMask | KeyPressMask |
        ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | 
	Button1MotionMask );

    init_colors();
    load_font(&font_info);

    /* Create GC for text and drawing */
    getGC(win, &gc, font_info);
    getGC(win, &gcx, font_info);
    XSetFunction(dpy, gcx, GXxor);

    /* Display window */
    XMapWindow(dpy, win);

    /* turn on backing store */
    attr.backing_store = Always;
    XChangeWindowAttributes(dpy,win,CWBackingStore,&attr);     
}

static char *visual_class[] = {
    "StaticGray",
    "GrayScale",
    "StaticColor",
    "PseudoColor",
    "TrueColor",
    "DirectColor"
};

int init_colors() 
{
    int default_depth;
    // Visual *default_visual;
    static char *name[] = {
	"#000000", // blk
	"#ffffff", // wht
	"#ff0000", // red
	"#00ff00", // grn
	"#5050ff", // blu	/* brighten up our blues a la Ken.P. :-)*/
	"#00ffff", // aqu
	"#ff00ff", // mag
	"#ffff00", // yel
	"#ff7f7f", // pnk
	"#7fff7f", // lime
	"#7f7fff", // pale blue
	"#007f7f", // blue-green
	"#7f007f", // pur
	"#7f7f00", // khaki
	"#b0b0b0", // light grey
	"#707070"  // dark grey
    };


    XColor exact_def;
    Colormap default_cmap;
    int ncolors = 0;
    int i;
    XVisualInfo visual_info;
    int debug=0;

    /* try to allocate colors for PseudoColor, TrueColor,
     * DirectColor, and StaticColor; use black and white
     * for StaticGray and GrayScale */

    default_depth = DefaultDepth(dpy, screen_num);
    // default_visual = DefaultVisual(dpy, screen_num);
    default_cmap = DefaultColormap(dpy, screen_num);
    if (default_depth == 1) {
	/* Must be StaticGray, use black and white */
	colors[0] = BlackPixel(dpy, screen_num);
	for (i=1; i<MAX_COLORS; i++) {
	    colors[i] = WhitePixel(dpy, screen_num);
	};
	return(0);
    }

    i=5;
    while (!XMatchVisualInfo(dpy, screen_num, default_depth,
	/* visual class */ i--, &visual_info)) 
	;
    if (debug) printf("found a %s class visual at default depth\n", visual_class[++i]);

    if ( i < StaticColor ) {	/* Color visual classes are 2 to 5 */
	/* No color visual available at default depth;
	 * some applications might call XMatchVisualInfo
	 * here to try for a GrayScale visual if they can
	 * use gray to advantage, before giving up and
	 * using black and white */

	colors[0] = BlackPixel(dpy, screen_num);
	for (i=1; i<MAX_COLORS; i++) {
	    colors[i] = WhitePixel(dpy, screen_num);
	};

	return(0);
    }

    /* Otherwise, got a color visual at default depth */

    /* The visual we found is not necessarily the default
     * visual, and therefore it is not necessarily the one we used to
     * creat our window; however, we now know for sure that color is
     * supported, so the following code will work (or fail in a
     * controlled way) */

     for (i=0; i < MAX_COLORS; i++) {
	/* printf("allocating %s\n", name[i]); */
	if (!XParseColor(dpy, default_cmap, name[i], &exact_def)) {
	    printf("color name %s not in database\n", name[i]);
	}
	/* printf("The RGB values from the database are %d, %d, %d\n",
	    exact_def.red, exact_def.green, exact_def.blue); */
	if (!XAllocColor(dpy, default_cmap, &exact_def)) {
	    printf("no matching colors cells can be allocated\n");
	    exit(0);
	}
	/* printf("The RGB values actually allocated are %d, %d, %d\n",
	    exact_def.red, exact_def.green, exact_def.blue); */
	colors[i] = exact_def.pixel;
	ncolors++;
    }
    /* printf("%s: allocated %d read-only color cells\n", progname, ncolors); */
    return(1);
}

void xwin_size(double *x, double *y) {
   *x=(double) width;
   *y=(double) height;
}

void redraw() {
    XClearArea(dpy, win, 0, 0, 0, 0, False);
    render();
    XFlush(dpy);
}

int procXevent(char *s, int n) {
    /* readline select stuff */
    int nf, nfds, cn, in;

    //struct timeval *timer = (struct timeval *) 0;       /* select blocks indefinitely */
    struct timeval timer;
    timer.tv_sec = 0;              /* wait times */
    timer.tv_usec = 0;

    fd_set rset, tset;
    // static char *s = NULL;
  
    /* courtesy g_plt_X11.c code from gnuplot */
    cn = ConnectionNumber(dpy);
    in = fileno(stdin);

    FD_ZERO(&rset);
    FD_SET(cn, &rset);
    FD_SET(in, &rset);

    nfds = (cn > in) ? cn + 1 : in + 1;

    while(1) {
	tset = rset;
	nf = select(nfds, &tset, (fd_set *) 0, (fd_set *) 0, &timer);
	if (nf < 0) {
	    if (errno == EINTR)
		continue;
	    printf("select failed. errno:%d", errno);
	}

	if (nf > 0) {
	    XFlush(dpy);
	}

 	if (need_redraw) {
	    redraw();
            need_redraw = 0;
        }

	if (FD_ISSET(cn, &tset)) {              /* pending X Event */
	    doXevent(NULL);			// ignore returned string for now
	}

	if (FD_ISSET(in, &tset)) {      /* pending stdin */
	    if (fgets(s,n,stdin) == NULL)  {
	       return EOF;
	    } else {
	       return 1;
	    }
	}

    }
}

void doXevent(char *s) {

    double x,y;
    XEvent xe;
    unsigned long all = 0xffffffff;
    int debug=0;

    while (XCheckMaskEvent(dpy, all, &xe)) { /* pending X Event */
        switch (xe.type) {
        case Expose:
            dbug("got Expose",debug);
            if (xe.xexpose.count != 0)
                break;
            if (xe.xexpose.window == win) {
	       //if (!need_redraw) redraw();
	       need_redraw++;
            } 
            break;
	case ReparentNotify:
	    break;
	case UnmapNotify:
	case MapNotify:
	    // if (!need_redraw) redraw();
	    need_redraw++;
	    break;
        case ConfigureNotify:
            dbug("got Configure Notify",debug);
	    width = xe.xconfigure.width;
	    height = xe.xconfigure.height;
	    if (XEventsQueued(dpy, QueuedAfterFlush) == 0) {
	        // if (!need_redraw) redraw();
    		need_redraw++;
	    }
            break;
        case MotionNotify:
            dbug("got Motion",debug);
            // x = (double) xe.xmotion.x;
            // y = (double) xe.xmotion.y;
            break;
        case ButtonRelease:
            x = (double) xe.xmotion.x;
            y = (double) xe.xmotion.y;
	    button(x,y,xe.xbutton.button,0);
            dbug("got ButtonRelease",debug);
            break;
        case ButtonPress:
            x= (double) xe.xmotion.x;
            y= (double) xe.xmotion.y;
	    button(x,y,xe.xbutton.button,1);
            dbug("got ButtonPress",debug);
            break;
        case KeyPress:
            dbug("got KeyPress",debug);
            break;
        default:
            fprintf(stderr,"got unexpected default event: %s\n",
		event_names[xe.type]);
            break;
        }
    }
}

void xwin_top() {
 XRaiseWindow(dpy, win);
}

void getGC(win, gc, font_info)
Window win;
GC *gc;
XFontStruct *font_info;
{
    unsigned long valuemask = 0;

    XGCValues values;
    unsigned int line_width = 0;
    /* int line_style = LineOnOffDash; */
    int line_style = LineSolid;
    int cap_style = CapButt;
    int join_style = JoinMiter;
    int dash_offset = 0;
    static char dash_list[] = {1,2};
    int list_length = 2;

    /* Create default Graphics Context */
    *gc = XCreateGC(dpy, win, valuemask, &values);

    /* Specify font */
    XSetFont(dpy, *gc, font_info->fid);

    /* Specify black foreground since default window background
    is white and default foreground is undefined */

    XSetBackground(dpy, *gc, BlackPixel(dpy, screen_num));
    XSetForeground(dpy, *gc, WhitePixel(dpy, screen_num));

    /* set line attributes */
    XSetLineAttributes(dpy, *gc, line_width, line_style,
        cap_style, join_style);

    /* set dashes */
    XSetDashes(dpy, *gc, dash_offset, dash_list, list_length); 
}

void load_font(font_info)
XFontStruct **font_info;
{
    char *fontname = "9x15";

    /* Load font and get font information structure */
    if ((*font_info = XLoadQueryFont(dpy, fontname)) == NULL) {
        fprintf(stderr, "%s: can't open %s font.\n", progname, fontname);
        exit(-1);
    }
}

void xwin_draw_point(double x, double y)
{
    if (displayon) {
	if (dd!=NULL) {
	    XDrawPoint(dpy, *dd, gc, 
		(int) rint(x),
		height-(int) rint(y));
	    XDrawPoint(dpy, *dd, gc, 
		(int) rint(x)+1,
		height-(int) rint(y)+1);
	    XDrawPoint(dpy, *dd, gc, 
		(int) rint(x)+1,
		height-(int) rint(y));
	    XDrawPoint(dpy, *dd, gc, 
		(int) rint(x),
		height-(int) rint(y)+1);
	} else {	// postscript mode
	    ps_start_line(x, y);
	    ps_continue_line(x+1.0, y);
	    ps_continue_line(x+1.0, y);
	    ps_continue_line(x, 1.0+y);
	    ps_continue_line(x, y);
	}
    }
}

void xwin_draw_line(x1, y1, x2, y2)
double x1,y1,x2,y2;
{
    static double xold, yold;
    if (displayon) {
	if (dd!=NULL) {
	    XDrawLine(dpy, *dd, gc, 
		(int) rint(x1),
		height-(int) rint(y1),
		(int) rint(x2),
		height-(int) rint(y2));
	} else {	// postscript mode
	    if (x1==xold && y1==yold) {
		ps_continue_line(x2,y2);
	    } else {
		ps_start_line(x1,y1);
		ps_continue_line(x2,y2);
	    }
	    xold=x2; yold=y2;
	}
    }
}


void xwin_set_pen_line(int pen, int line) 
{
    int dash_n;
    int dash_offset;
    int line_style;
    int line_width;
    char dash_list[5];

    /* FIXME: should avoid accessing out of bounds of colors[]
     * also should cache different pen colors to avoid having to keep
     * sending messages to server.  Currently this is enforced because
     */

    if (dd!=NULL) {

	static int oldpen=(-9999); /* optimize out unnecessary Xserver calls */
	static int oldlinetype=(-9999);

	pen = abs(pen)%MAX_COLORS;

	if (pen != oldpen) {
	    XSetForeground(dpy, gc, colors[pen]);	/* for lines */
	}

	if (line == oldlinetype) return;
	line = abs(line)%MAX_LINETYPE;

	/* there are two switches here because even if you XSetDashes(),
	 * the drawn line will still be solid *unless* you also change
	 * the line style to LineOnOffDash or LineDoubleDash.  So we set
	 * line zero to be LineSolid and all the rest to LineOnOffDash
	 * and then the XSetDashes will take effect.  (45 minutes to debug!)
	 * - with much appreciation to Ken Poulton for an example of this in 
	 * autoplot(1) code.
	 */

	switch (line) {
	   case 0:     line_style = LineSolid;     break;	/* solid */
	   case 1:     line_style = LineSolid;     break;	/* solid */
	   case 2:     line_style = LineOnOffDash; break;	/* dotted */
	   case 3:     line_style = LineOnOffDash; break;   /* broken */
	   case 4:     line_style = LineOnOffDash; break;   /* dot center */
	   case 5:     line_style = LineOnOffDash; break;   /* dash center */
	   case 6:     line_style = LineOnOffDash; break;   /* long dash */
	   case 7:     line_style = LineOnOffDash; break;   /* long dotted */
	   default:
	       printf("line type %d out of range\n", line);
	}        
	
	switch (line) {
	   case 0:				
	   case 1:				
	   case 2:     dash_list[0]=7; dash_list[1]=5; dash_n=2; break;

	   case 3:     dash_list[0]=2; dash_list[1]=2; dash_n=2; break;

	   case 4:     dash_list[0]=7; dash_list[1]=2;
		       dash_list[2]=3; dash_list[3]=2; dash_n=4; break;

	   case 5:     dash_list[0]=7; dash_list[1]=2;
		       dash_list[2]=1; dash_list[3]=2; dash_n=4; break;


	   case 6:     dash_list[0]=9; dash_list[1]=5; dash_n=2; break;

	   case 7:     dash_list[0]=4; dash_list[1]=4; dash_n=2; break;
	}

	dash_offset=0;

	if (width + height > 1500) {
	   line_width=2;
	} else {
	   line_width=0;
	}

	XSetLineAttributes(dpy, gc, line_width, line_style, CapButt, JoinRound); 
	XSetDashes(dpy, gc, dash_offset, dash_list, dash_n);
    } else {
	ps_set_pen(pen);
	ps_set_line(line);
    }
}

void xwin_draw_box(double x1, double y1, double x2, double y2)
{
    xwin_draw_line(x1, y1, x1, y2);
    xwin_draw_line(x1, y2, x2, y2);
    xwin_draw_line(x2, y2, x2, y1);
    xwin_draw_line(x2, y1, x1, y1);
}

void xwin_annotate(char *buf) {
   double size;
   size = (double) ((width+height)/80);
   XClearArea(dpy, win, 0, height-(int)(size*1.5), width, 1.5*size, False);
    do_note(buf, (double) width/2, (double) 5, MIRROR_OFF,
                    size, 1.0, 0.0, 0.0, 0, 1);

   // XDrawImageString(dpy, win, gc, 20, height-5, buf, strlen(buf));
}

void xwin_display(int mode) {
   displayon = mode;
}

void dbug(char *s, int debug)
{
    if (debug) {
	fprintf(stderr,"%s\n",s);
    }
}


/* given a shiftmask, return how many bits to shift */
/* it into position... look at most max bits */

int shift_from_mask(int mask, int max) {

    int bit, tmp, shift;

    tmp = mask;
    bit = tmp & 1;
    shift=0;
    while(bit != 1 && shift<max) {
       tmp = tmp>>1;	/* shift down to 1 */
       bit = tmp & 1;
       shift++;
    }
    bit = tmp & 128;
    while(bit != 128 && shift>1) {
       tmp = tmp<<1;	/* shift up to 128 */
       bit = tmp & 128;
       shift--;
    }
    return (shift);
}

int xwin_dump_graphics(char *cmd) 
{
    XImage *xi;
    int x, y;
    unsigned long pixel;
    char buf[256];
    int R,G,B;
    FILE *fd;
    int i;
    int debug=0;
    int rshift, gshift, bshift;
    // int err=0;
    int status;
    Visual *default_visual;

    Pixmap pixmap;
    pixmap = XCreatePixmap(dpy, RootWindow(dpy,screen_num), width, 
    	height, DefaultDepth(dpy, DefaultScreen(dpy)));
    dd=&pixmap;

    XSetForeground(dpy, gc, BlackPixel(dpy, screen_num));
    XFillRectangle (dpy, pixmap, gc, 0, 0, width, height);

    render();
    xi = XGetImage(dpy, pixmap, 0,0, width, height, AllPlanes, ZPixmap);

    // an X11 bug!  XGetImage(pixmap) does not set masks 
    // so we get the masks from default_visual
    // Thanks to :http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/
    // x11-wm/windowmaker/work/WindowMaker-0.92.0/wrlib/xpixmap.c
    // for this observation!

    default_visual = DefaultVisual(dpy, screen_num);
    xi->red_mask=default_visual->red_mask;
    xi->green_mask=default_visual->green_mask;
    xi->blue_mask=default_visual->blue_mask;

    dd=&win;
    XFreePixmap(dpy, pixmap);

    // xi = XGetImage(dpy, win, 0,0, width, height, AllPlanes, ZPixmap);

    if (debug) {
	printf("width  = %d\n", xi->width );
	printf("height = %d\n", xi->height);

	if (xi->byte_order == LSBFirst) {
	    printf("byte_order = LSBFirst\n");
	} else if (xi->byte_order == MSBFirst) {
	    printf("byte_order = MSBFirst\n");
	} else {
	    printf("unknown byte_order: %d\n", xi->byte_order);
	}

	printf("bitmap unit=%d\n", xi->bitmap_unit);

	if (xi->bitmap_bit_order == LSBFirst) {
	    printf("bitmap_bit_order = LSBFirst\n");
	} else if (xi->byte_order == MSBFirst) {
	    printf("bitmap_bit_order = MSBFirst\n");
	} else {
	    printf("unknown bitmap_bit_order: %d\n", xi->bitmap_bit_order);
	}

	printf("bitmap pad = %d\n", xi->bitmap_pad);
	printf("depth = %d\n", xi->depth);
	printf("bytes_per_line = %d\n", xi->bytes_per_line);
	printf("bit_per_pixel = %d\n", xi->bits_per_pixel);
	printf("red mask= %08lx\n", xi->red_mask);
	printf("green mask= %08lx\n", xi->green_mask);
	printf("blue mask= %08lx\n",  xi->blue_mask);
    }

    /* compute bit shift values */

    rshift=shift_from_mask(xi->red_mask, 8*xi->bitmap_unit);
    gshift=shift_from_mask(xi->green_mask, 8*xi->bitmap_unit);
    bshift=shift_from_mask(xi->blue_mask, 8*xi->bitmap_unit);
    // printf("rs: %d, gs: %d, bs: %d\n",rshift, gshift, bshift);

    if ((fd = popen(cmd, "w")) == 0 ) {
    	printf("can't open dump pipeline: %s\n", cmd);
	return(0);
    }

    fflush(stdout);

    sprintf(buf, "P6\n%d\n%d\n%d\n", xi->width, xi->height, 255);
    fwrite(buf, 1, strlen(buf), fd);

    i=0;
    for (y=0; y<xi->height; y++) {
	for (x=0; x<xi->width && (waitpid(-1, &status, WNOHANG) != -1); x++) {
	   if (++i==10000) {
	       i=0;
	       printf(".");
	       fflush(stdout);
	   }
	   pixel = XGetPixel(xi, x, y);	

	   R=pixel & xi->red_mask;
	   R = R>>rshift;
	   buf[0] = (unsigned char) R;

	   G=pixel & xi->green_mask;
	   G = G>>gshift;
	   buf[1] = (unsigned char) G;

	   B=pixel & xi->blue_mask;
	   B = B>>bshift;
	   buf[2] = (unsigned char) B;

	   fwrite(buf, 3, 1, fd);
	   // err=fwrite(buf, 3, 1, fd);
	}
    }
    printf("done \n");
    fflush(stdout);

    return(pclose(fd));
    XDestroyImage(xi);
    return(1);
}

int xwin_dump_postscript(char *cmd) 
{

    extern FILE *fp;
    if ((fp = popen(cmd, "w")) == 0 ) {
    	printf("can't open dump pipeline: %s\n", cmd);
	return(0);
    }

    ps_set_file(fp);
    // printf("%d %d\n", width, height);
    ps_preamble("title", "Pdplot", 8.5, 11.0, (double) 0, (double) 0, (double) width, (double) height);

    dd=NULL;
    render();
    dd=&win;

    ps_postamble(fp);
    return(1);
}
