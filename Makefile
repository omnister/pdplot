OBJS=main.o points.o readfont.o xwin.o label.o clip.o symbol.o postscript.o

FONTS=SYMBOL.F NOTEDATA.F

TARS=main.c points.c readfont.c xwin.c label.c clip.c points.h symbol.c symbol.h \
xwin.h readfont.h eventnames.h postscript.c postscript.h ${FONTS} pd pd.1

CC=cc -ggdb -Wall
#CC=cc -pg -Wall

BINDIR = /usr/local/bin
LIBDIR = /usr/local/lib/pdplot
MANDIR = /usr/local/man/man1
MANPAGE = pd.1

pdplot: points.h $(OBJS)
	$(CC) $(OBJS) -o pdplot -L/usr/X11R6/lib -lX11 -lm

clean: 
	rm -f *.o pdplot

install: pd pdplot pd.1 ${FONTS}
	-/bin/mv -f ${BINDIR}/pd ${BINDIR}/pd.old
	-/bin/cp pd $(BINDIR)/pd 
	-/bin/mv -f ${BINDIR}/pdplot ${BINDIR}/pdplot.old
	-/bin/cp pdplot $(BINDIR)/pdplot
	if [ ! -d ${LIBDIR} ] ; \
            then mkdir ${LIBDIR} || exit 1 ; \
	else \
             exit 0 ; \
	fi
	-/bin/cp ${FONTS} ${LIBDIR}
	if [ ! -d ${MANDIR} ] ; \
            then mkdir ${MANDIR} || exit 1 ; \
	else \
             exit 0 ; \
	fi
	-/bin/cp ${MANPAGE} ${MANDIR}

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
clip.o: points.h
clip.o: xwin.h
label.o: points.h
main.o: points.h
main.o: readfont.h
main.o: symbol.h
main.o: xwin.h
points.o: points.h
points.o: readfont.h
points.o: symbol.h
points.o: xwin.h
postscript.o: postscript.h
pwin.o: eventnames.h
pwin.o: points.h
pwin.o: xwin.h
readfont.o: points.h
readfont.o: readfont.h
symbol.o: symbol.h
xwin.o: eventnames.h
xwinold.o: eventnames.h
xwinold.o: points.h
xwinold.o: xwin.h
xwin.o: points.h
xwin.o: xwin.h
