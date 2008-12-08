#include <stdlib.h>	/* for malloc() */
#include <stdio.h>
#include "symbol.h"

static Symbol *symlist = 0; /* symbol table: linked list */

void initsym() {
    install(1,"circle");
    install(2,"box");
    install(3,"up-triangle");
    install(4,"diamond");
    install(5,"star");
    install(6,"plus");
    install(7,"x");
    install(8,"asterisk");
    install(9,"filled-circle");
    install(10,"filled-box");
    install(11,"filled-up-triangle");
    install(12,"filled-left-triangle");
    install(13,"filled-star");
    install(14,"filled-down-triangle");
    install(15,"filled-right-triangle");
    install(16,"suit-spade");
    install(17,"suit-heart");
    install(18,"suit-diamond");
    install(19,"suit-club");
    install(20,"suit-clover");
    install(21,"suit-fleur-de-lys");
    install(22,"palm");
    install(23,"pine");
    install(24,"deciduous");
    install(25,"oak");
    install(26,"cross");
    install(27,"crescent");
    install(28,"star-of-david");
    install(29,"tee-flag");
    install(30,"anchor");
    install(31,"mast");
    install(32,"pick-axes");
    install(33,"bell");
    install(34,"sun");
    install(35,"mercury");
    install(36,"venus");
    install(37,"earth");
    install(38,"mars");
    install(39,"jupiter");
    install(40,"saturn");
    install(41,"uranus");
    install(42,"neptune");
    install(43,"pluto");
    install(44,"moon");
    install(45,"comet");
    install(46,"fixed-star");
    install(47,"ascending-node");
    install(48,"descending-node");
    install(49,"filled-pointed-triangle");
    install(50,"filled-cap");
    install(51,"filled-scalene");
    install(52,"section");
    install(53,"dagger");
    install(54,"double-dagger");
}

Symbol *lookup(s)           /* find s in symbol table */
char *s;
{
    Symbol *sp;

    for (sp=symlist; sp!=(Symbol *)0; sp=sp->next)
        if (strcmp(sp->name, s)==0)
            return sp;
    return 0;               /* 0 ==> not found */
}

Symbol *install(index,s)      /* install s in symbol table */
int index;
char *s;
{
    Symbol *sp;
    char *emalloc();

    sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s)+1); /* +1 for '\0' */
    strcpy(sp->name, s);
    sp->index = index;
    sp->next = symlist; /* put at front of list */
    symlist = sp;
    return sp;
}

char *emalloc(n)    /* check return from malloc */
unsigned n;
{
    char *p;

    p = (char *) malloc(n);
    if (p == 0) {
        fprintf(stderr,"out of memory\n");
	exit(2);
    }
    return p;
}
