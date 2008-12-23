extern void ps_preamble(FILE *fp, char *dev, char *prog, double pdx, 
                 double pdy, double llx, double lly, double urx, double ury);
extern void ps_set_line(int line);
extern void ps_set_pen(int pen);
extern void ps_start_line(double x1, double y1);
extern void ps_continue_line(double x1, double y1);;
extern void ps_postamble();
