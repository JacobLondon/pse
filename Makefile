TARGET=pse
CXX=g++
CXXFLAGS=-std=c++17 -march=native -O2 -pipe -lSDL2
OBJS=src/main.o

.PHONY: clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)
