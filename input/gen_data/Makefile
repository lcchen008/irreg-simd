PROG   = gen
CC     = gcc
CFLAGS = -O4 -DDBL_MAX -DBUILD_LIST -DCOORD -DBLOCK_ADAPTIVE -DTIME -DCSTYLE -I..
LIBS   = -lm 
MISC   = misc
TIMER  = timer

default:
	$(CC) $(CFLAGS) -o gen gen.c $(MISC).c $(TIMER).c $(LIBS)
clean:
	-rm gen
