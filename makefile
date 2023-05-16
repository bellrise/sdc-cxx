BASEFLAGS := -Wall -Wextra -I. -std=c++2a -fsanitize=address
CFLAGS := $(BASEFLAGS) -xc++
TFLAGS := $(BASEFLAGS)
SRC := $(shell find . -type f -name '*.cc' | grep -v 'main.cc')
OBJ := $(patsubst %.cc,build/%.o,$(SRC))

all: setup lib

setup: compile_flags.txt
	mkdir -p build

clean:
	rm -rf build

compile_flags.txt: makefile
	echo $(CFLAGS) | tr ' ' '\n' > compile_flags.txt

lib: $(OBJ)
	ar rcs build/libsdc-cxx.a $(OBJ)

build/%.o: %.cc
	clang++ -c -o $@ $(CFLAGS) $<

test: all
	clang++ $(TFLAGS) main.cc build/libsdc-cxx.a
