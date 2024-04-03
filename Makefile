# Makefile

CXX = g++ # Compiler
CXXFLAGS = -Ilib -Wall # Compiler flags, -Ilib adds the lib directory as include path
SRC = src/main.cpp # Source files
OUT = build/app # Output executable

all: $(OUT)

$(OUT): $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -rf build/