PROG = moldyn
MISC = misc
TIME = timer

LOCAL_CFLAGS = -DCSTYLE 
INC = 
DEFS = -DTIME -DLREORDER -DCOMPUTE_SORT -DCONVERT 
CFLAGS = -O3 $(DEFS) $(LOCAL_CFLAGS) $(INC) -mmic
CPPFLAGS = -O3 $(DEFS) $(LOCAL_CFLAGS) $(INC) -std=c++0x -mmic
LDFLAGS = -O3 -mmic
CC = gcc 
CCC = icc 

LIBS = -lm

OBJS=moldyn.o \
         misc.o\
	 timer.o

main: $(OBJS)
	$(CCC) $(LDFLAGS) -o main $(OBJS)
%.o : %.cpp
	$(CCC) $(CPPFLAGS) -c $< -o $*.o -mmic
%.o : %.c
	$(CCC) $(CFLAGS) -DBLOCK_ADPATIVE -c $< -o $*.o $(LIBS) -mmic

clean:
	-rm main $(OBJS)

#-- Sequential running -----------------------------
#org: $(PROG).c $(TIME).c
#	$(CC) $(CFLAGS) -DBLOCK_ADPATIVE -o $(PROG) $(PROG).c $(MISC).c $(TIME).c $(LIBS)
#
#org0: $(PROG).c $(TIME).c
#	$(CC) $(CFLAGS) -DORIG_ADAPTIVE -o $(PROG)0 $(PROG).c $(MISC).c $(TIME).c $(LIBS)
