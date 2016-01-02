CC=icc -Wall

SSE=-mmic

CFLAGS = -O3 -I. $(SSE) -pthread -mcmodel=medium -std=c++0x

LIBS = -lpthread 

LDFLAGS = $(SSE) $(LIBS)

SRCS = $(wildcard ./*.cpp) $(wildcard ./framework/*.cpp)

$(info $(SRCS))

OBJS = $(SRCS:.cpp=.o)

main: $(OBJS) $(wildcard ./*.h)
	$(CC) $(LIBS) $(LDFLAGS) -o main $(OBJS)
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	-rm main $(OBJS)
