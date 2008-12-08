
#define TICKSIZE 0.005  /* fraction of xsize+ysize for default ticks */

void initX(void);
int procXevent(void); 
void xwin_draw_line(double x1, double y1, double x2, double y2);
void xwin_draw_point(double x1, double y1);
void xwin_draw_box(double x1, double y1, double x2, double y2);
void xwin_set_pen_line(int pen, int line);
void xwin_size(double *x, double *y);
void xwin_top();
void xwin_annotate(char *buf);

extern char *progname;
extern int need_redraw;		// setting TRUE causes redraw
