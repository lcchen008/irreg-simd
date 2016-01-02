CC=icc -Wall

CFLAGS = -O3 -I. $(SSE) -openmp -parallel -std=c++0x

#LIBS = -lpthread

LDFLAGS = $(SSE) $(LIBS) -openmp -parallel

SRCS = $(wildcard ./*.cpp) $(wildcard ./framework/*.cpp)

$(info $(SRCS))

OBJS = $(SRCS:.cpp=.o)

main: $(OBJS) $(wildcard ./*.h)
	$(CC) $(LIBS) $(LDFLAGS) -o main $(OBJS) -O3
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	-rm main $(OBJS)
