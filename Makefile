
OBJS = target/objs/lattice.o
IMAGEMAGICK_BIN_PATH = /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16

CPPFLAGS = `$(IMAGEMAGICK_BIN_PATH)/Magick++-config --cppflags --cxxflags --ldflags --libs`

all: prepare compile

prepare:
	mkdir -p target/objs

target/objs/%.o: lattice/%.cpp
	g++ $(CPPFLAGS) -c $< -o $@

compile: $(OBJS)
	g++ -o target/lattice $(OBJS) $(CPPFLAGS)

