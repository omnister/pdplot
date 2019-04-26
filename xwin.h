#include "points.h"

void initX(void);
int procXevent(char *s, int n); 
void xwin_draw_line(double x1, double y1, double x2, double y2);
void xwin_draw_point(double x1, double y1);
void xwin_draw_box(double x1, double y1, double x2, double y2);
void xwin_set_pen_line(int pen, int line);
void xwin_size(double *x, double *y);
void xwin_top();
void xwin_annotate(PLOTDAT *pd, char *buf);
void xwin_display(int mode);
int xwin_dump_graphics(char *command);
int xwin_dump_postscript(char *command);

extern char *progname;
extern int need_redraw;		// setting TRUE causes redraw

extern void init_stipples();
extern int get_stipple_index(int fill, int pen);
extern char *get_stipple_bits(int i);
extern const char * xwin_ps_dashes(int line);
extern const char * xwin_svg_dashes(int line);
extern const char * get_hpgl_fill(int line);

