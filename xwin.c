#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include "eventnames.h"
#include "xwin.h"
#include "points.h"

void load_font(XFontStruct **font_info);
void debug(char *s, int dbug);
int init_colors();
void getGC(Window win, GC *gc, XFontStruct *font_info);
void doXevent(char *s);
void xwin_top();

#define TICKSIZE 0.005  /* fraction of xsize+ysize for default ticks */
#define MAX_COLORS 16
#define MAX_LINETYPE 7
unsigned long colors[MAX_COLORS];    /* will hold pixel values for colors */

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
    class_hints->res_class = "hoe";

    XSetWMProperties(dpy, win, &windowName, &iconName,
             (char **) NULL, (int) NULL, size_hints, wm_hints, class_hints);

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
    Visual *default_visual;
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
    default_visual = DefaultVisual(dpy, screen_num);
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
    int dbug=0;

    while (XCheckMaskEvent(dpy, all, &xe)) { /* pending X Event */
        switch (xe.type) {
        case Expose:
            debug("got Expose",dbug);
            if (xe.xexpose.count != 0)
                break;
            if (xe.xexpose.window == win) {
	       //if (!need_redraw) redraw();
	       need_redraw++;
            } 
            break;
	case ReparentNotify:
	    break;
	case MapNotify:
	    // if (!need_redraw) redraw();
	    need_redraw++;
	    break;
        case ConfigureNotify:
            debug("got Configure Notify",dbug);
	    width = xe.xconfigure.width;
	    height = xe.xconfigure.height;
	    if (XEventsQueued(dpy, QueuedAfterFlush) == 0) {
	        // if (!need_redraw) redraw();
    		need_redraw++;
	    }
            break;
        case MotionNotify:
            debug("got Motion",dbug);
            // x = (double) xe.xmotion.x;
            // y = (double) xe.xmotion.y;
            break;
        case ButtonRelease:
            x = (double) xe.xmotion.x;
            y = (double) xe.xmotion.y;
	    button(x,y,xe.xbutton.button,0);
            debug("got ButtonRelease",dbug);
            break;
        case ButtonPress:
            x= (double) xe.xmotion.x;
            y= (double) xe.xmotion.y;
	    button(x,y,xe.xbutton.button,1);
            debug("got ButtonPress",dbug);
            break;
        case KeyPress:
            debug("got KeyPress",dbug);
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
    XDrawPoint(dpy, win, gc, 
    	(int) rint(x),
	height-(int) rint(y));
    XDrawPoint(dpy, win, gc, 
    	(int) rint(x)+1,
	height-(int) rint(y)+1);
    XDrawPoint(dpy, win, gc, 
    	(int) rint(x)+1,
	height-(int) rint(y));
    XDrawPoint(dpy, win, gc, 
    	(int) rint(x),
	height-(int) rint(y)+1);
}

void xwin_draw_line(x1, y1, x2, y2)
double x1,y1,x2,y2;
{
    XDrawLine(dpy, win, gc, 
    	(int) rint(x1),
	height-(int) rint(y1),
	(int) rint(x2),
	height-(int) rint(y2));
}

void xwin_set_pen_line(int pen, int line) 
{
    int dash_n;
    int dash_offset;
    int line_style;
    char dash_list[5];

    /* FIXME: should avoid accessing out of bounds of colors[]
     * also should cache different pen colors to avoid having to keep
     * sending messages to server.  Currently this is enforced because
     */

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
    XSetLineAttributes(dpy, gc, 0, line_style, CapButt, JoinRound); 
    XSetDashes(dpy, gc, dash_offset, dash_list, dash_n);

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
   XClearArea(dpy, win, 0, height-(int)(size*1.5), width, 20, False);
    do_note(buf, (double) width/2, (double) 5, MIRROR_OFF,
                    size, 1.0, 0.0, 0.0, 0, 1);

   // XDrawImageString(dpy, win, gc, 20, height-5, buf, strlen(buf));
}

void debug(char *s, int dbug)
{
    if (dbug) {
	fprintf(stderr,"%s\n",s);
    }
}
