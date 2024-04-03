CC := g++

CFLAGS := -Wall -std=c++11 -O2 $(shell sdl2-config --cflags)

LDFLAGS := $(shell sdl2-config --libs) -lfftw3 -lm

TARGET := program

SRCS := main.cpp

OBJS := $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f *.o $(TARGET)

.PHONY: clean
