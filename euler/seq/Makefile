CC = gcc 
CCC = icc 

LDFLAGS=$(OPT) -O3  -I . -std=c++0x -mmic

CFLAGS=-std=c++0x -mmic

LIBS = -lm

OBJS=euler.o 

main: $(OBJS)
	$(CCC) $(LDFLAGS) -o main $(OBJS)
%.o : %.cpp
	$(CCC) -O3 $(CFLAGS) -c $< -o $*.o

clean:
	-rm main $(OBJS)
