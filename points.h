#ifndef __PDP_POINTS_H
#define __PDP_POINTS_H

typedef struct datum {
    char *cmd;		// either NULL or cmd string.  If str, then x,y unused.
    double x;
    double y;
    struct datum *next;
    struct datum *prev;
} DATUM;

typedef struct plotdat {
     DATUM  *data;
     double llx;	// raw screen coords for this plot
     double lly;
     double urx;
     double ury;
     int    boundsflag;	// set to 0 to force initialization of bounding box
     double bbxmin;	// bounding box captured during a dry run plot
     double bbxmax;
     double bbymin;
     double bbymax;
     double xmin;	// computed bounds 
     double xmax;
     double ymin;
     double ymax;
     double xsetmin;	// xset command
     double xsetmax;
     double ysetmin;	// yset command
     double ysetmax;
     int    xlogmode;	// log/lin scale mode
     int    ylogmode;
     double xscale;	// xscale multiplying factor
     double yscale;	// yscale multiplying factor
     char * title;	// plot title (only stored in plots[0])
     char * xaxis;	// xscale label
     char * yaxis;	// yscale label
} PLOTDAT;

typedef struct xform {
     double r11;
     double r12;
     double r21;
     double r22;
     double dx;
     double dy;
} XFORM;

extern void initplot(void);
extern void nextygraph(void);
extern void savepoint(double x, double y);
extern void savecmd(char *cmd);
extern void dumppoints(void);
extern void render(void);
extern void title(char *);
extern void xset(double xmin, double xmax);
extern void yset(double ymin, double ymax);
extern void xscale(char *, double scale);
extern void yscale(char *, double scale);
extern void draw(PLOTDAT *pd, double x, double y);
extern void jump(void);
extern void fontdraw(PLOTDAT *pd, double x, double y);
extern void fontjump(void);
extern void pen(int penno);
extern void line(int lineno);
extern void symbol(int symno);
extern void do_note();
void do_symbol(PLOTDAT *pd, int c, double x, double y, double size);
extern void back(int backstat);
extern void loose_label(PLOTDAT *pd, double *min, double *max, int nticks, int axis,\
	int dolabel, int logmode, int dryrun);
extern void axislabel(PLOTDAT *pd, char *temp, int axis);
extern void gridlabel(PLOTDAT *pd, char *temp, double alpha, int axis);
extern void gridline(PLOTDAT *pd, double alpha, int axis);
extern void box(int mode);
extern void grid(int mode);
extern void logmode(int axis, int mode);
extern void scale(int mode);
extern void tick(int mode);
extern void clip();
extern void clip_set(double xn, double xx, double yn, double yx);
extern void dim(int mode);
extern void iso(int mode, double scale);
extern void tickset(double scale);
extern void setcharsize(double size);
extern void button(double x, double y, int buttonno, int state);
extern void frame_pen(int pen);
extern void gridpen(int pen);
extern void xgridpen(int pen);
extern void ygridpen(int pen);
extern void xgrid(int line);
extern void ygrid(int line);
extern void grid(int line);
extern void gridstate(int mode);
extern void xgridstate(int mode);
extern void ygridstate(int mode);
extern void xscaletol(double tolerance);
extern void yscaletol(double tolerance);
extern void scaletol(double tolerance);
extern void labelrotation(double angle);
extern int linenum;

extern double xmin, xmax, ymin, ymax;


#define MIRROR_OFF 0
#define MIRROR_X   1
#define MIRROR_Y   2
#define MIRROR_XY  3

#endif
