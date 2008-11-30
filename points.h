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
     double xmin;	// computed bounds 
     double xmax;
     double ymin;
     double ymax;
     double xsetmin;	// xset command
     double xsetmax;
     double ysetmin;	// yset command
     double ysetmax;
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
extern void draw(double x, double y, double xmin, double xmax, double ymin, double ymax);
extern void jump(void);
extern void fontdraw(double x, double y);
extern void fontjump(void);
extern void pen(int penno);
extern void symbol(int symno);
extern void do_note();
void do_symbol(int c, double x, double y, double size);
extern void back(int backstat);
extern void loose_label(PLOTDAT *pd, double *min, double *max, int nticks, int axis, int dolabel);
extern void axislabel(PLOTDAT *pd, char *temp, int axis);
extern void gridlabel(PLOTDAT *pd, char *temp, double alpha, int axis);
extern void gridline(PLOTDAT *pd, double alpha, int axis);
extern void box(int mode);
extern void grid(int mode);
extern void clip();

#define MIRROR_OFF 0
#define MIRROR_X   1
#define MIRROR_Y   2
#define MIRROR_XY  3

