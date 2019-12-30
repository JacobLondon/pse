TARGET=pse
CXX=g++
CXXFLAGS=-std=c++17 -march=native -O2 -pipe -lSDL2
OBJS=src/context.o src/draw.o src/util.o src/main.o \
	src/modules/demo.o src/modules/rogue.o

.PHONY: clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)
