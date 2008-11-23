OBJS=main.o points.o readfont.o xwin.o label.o

CC=cc -ggdb -Wall

bw: points.h $(OBJS)
	cc -Wall -ggdb $(OBJS) -o bw -L/usr/X11R6/lib -lX11 -lm

clean: 
	rm -f *.o bw

tar: $(TARS)
	(  d=`date +%F`;\
	   q=`pwd`;\
	   p=`basename $$q`;\
	   rm -rf $$p$$d;\
	   mkdir $$p$$d;\
	   cp -rp $(TARS) $$p$$d;\
	   mkdir $$p$$d/cells;\
	   cp -rp $(CELLS) $$p$$d/cells;\
	   mkdir $$p$$d/man;\
	   cp -rp $(MANS) $$p$$d/man;\
	   tar czvf - $$p$$d >$$p$$d.tar.gz;\
	)

depend: ${OBJ}
	cp Makefile Makefile.bak
	sed -n -e '1,/^# DO NOT DELETE OR MODIFY THIS LINE/p' Makefile \
                > newmakefile
	grep '^#include[ 	]*"' *.c \
                | sed -e 's/:#include[  "]*\([a-z0-9\._A-Z]*\).*/: \1/' \
                | sed -e 's/\.c:/.o:/' \
                | sort | uniq >> newmakefile
	mv Makefile Makefile.bak
	mv newmakefile Makefile

#-----------------------------------------------------------------
# DO NOT PUT ANY DEPENDENCIES AFTER THE NEXT LINE -- they will go away
# DO NOT DELETE OR MODIFY THIS LINE -- make depend uses it
label.o: points.h
main.o: points.h
main.o: xwin.h
points.o: points.h
readfont.o: points.h
readfont.o: readfont.h
xwin.o: eventnames.h
xwin.o: points.h
xwin.o: xwin.h
