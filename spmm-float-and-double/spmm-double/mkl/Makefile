CC=icc -Wall

SSE=-mmic

CFLAGS = -O3 -I. $(SSE) -mkl -pthread -mcmodel=medium -std=c++0x

LIBS = -lpthread -mkl -L$(MKLROOT)/lib/mic

LDFLAGS = $(SSE) $(LIBS)

SRCS = $(wildcard ./*.cpp) $(wildcard ./*.c)

$(info $(SRCS))

OBJS = $(SRCS:.cpp=.o)

main: $(OBJS) $(wildcard ./*.h)
	$(CC) $(LIBS) $(LDFLAGS) -o main $(OBJS)
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	-rm main $(OBJS)
