
void initX(void);
int procXevent(char *s, int n); 
void xwin_draw_line(double x1, double y1, double x2, double y2);
void xwin_draw_point(double x1, double y1);
void xwin_draw_box(double x1, double y1, double x2, double y2);
void xwin_set_pen_line(int pen, int line);
void xwin_size(double *x, double *y);
void xwin_top();
void xwin_annotate(char *buf);
void xwin_display(int mode);
int xwin_dump_graphics(char *command);
int xwin_dump_postscript(char *command);

extern char *progname;
extern int need_redraw;		// setting TRUE causes redraw
