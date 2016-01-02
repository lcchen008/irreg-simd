CC=icc -Wall

SSE=-mmic

DEBUG=-g

CFLAGS = -O3 -I. $(SSE) -openmp -parallel -pthread -mcmodel=medium -std=c++0x $(DEBUG)

LIBS = -lpthread 

LDFLAGS = $(SSE) $(LIBS) $(DEBUG) -openmp -parallel -O3

SRCS = $(wildcard ./*.cpp) $(wildcard ./framework/*.cpp)

$(info $(SRCS))

OBJS = $(SRCS:.cpp=.o)

main: $(OBJS) $(wildcard ./*.h)
	$(CC) $(LIBS) $(LDFLAGS) -o main $(OBJS)
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	-rm main $(OBJS)
