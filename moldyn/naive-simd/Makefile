CC=icc -Wall -w

SSE=-mmic

DEBUG=

CFLAGS = -O3 -I. -I../../tools $(SSE) -openmp -parallel -mcmodel=medium -std=c++0x -D MIC $(DEBUG)

#LIBS = -lpthread

LDFLAGS = $(SSE) $(LIBS) -openmp -parallel $(DEBUG)

SRCS = $(wildcard ./*.cpp) $(wildcard ./framework/*.cpp)

$(info $(SRCS))

OBJS = $(SRCS:.cpp=.o)

main: $(OBJS) $(wildcard ./*.h)
	$(CC) $(LIBS) $(LDFLAGS) -o main $(OBJS) -O3
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	-rm main $(OBJS)
