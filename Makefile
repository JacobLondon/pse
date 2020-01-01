TARGET=pse
CXX=g++
CXXFLAGS=-std=c++17 -march=native -O2 -pipe -lSDL2 -Wall
OBJS=src/context.o src/draw.o src/util.o src/main.o \
	src/modules/demo.o \
	src/modules/rogue/entity.o src/modules/rogue/gen.o \
	src/modules/rogue/globals.o src/modules/rogue/interface.o \
	src/modules/rogue/rogue.o src/modules/rogue/types.o

.PHONY: clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)
