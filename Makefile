TARGET=pse
CXX=g++
CXXFLAGS=-std=c++17 -march=native -O2 -pipe -lSDL2 -lSDL2_image -Wall -Iinclude -lm
OBJS=src/ctx_draw.o src/ctx.o src/main.o src/util.o \
	src/pse-modules/demo.o \
	src/pse-modules/rogue/entity.o src/pse-modules/rogue/gen.o \
	src/pse-modules/rogue/globals.o src/pse-modules/rogue/draw.o \
	src/pse-modules/rogue/rogue.o src/pse-modules/rogue/types.o \
	src/pse-modules/trace.cpp

.PHONY: clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)
