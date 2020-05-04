
OBJS = target/objs/lattice.o \
	target/objs/population.o

IMAGEMAGICK_BIN_PATH = /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16

CPPFLAGS = `$(IMAGEMAGICK_BIN_PATH)/Magick++-config --cppflags --cxxflags --ldflags --libs`

all: prepare compile

prepare:
	mkdir -p target/objs

target/objs/%.o: lattice/%.cpp
	g++ $(CPPFLAGS) -c $< -o $@

compile: $(OBJS)
	g++ -o target/lattice $(OBJS) $(CPPFLAGS)

clean:
	rm target/objs/*.o target/lattice

	
#######################################
# Experimental runs

UNIFORM_ID=results/uniform-2.4-200g
run-uniform:
	mkdir -p results
	target/lattice generations=200 R0=2.4 neighbourhood=uniform animation=$(UNIFORM_ID).gif report=$(UNIFORM_ID).csv
	gnuplot -p -e "plot \"$(UNIFORM_ID).csv\" using 1:2 title \"Alttiit\" with lines, \"$(UNIFORM_ID).csv\" using 1:3 title \"Tarttuneet\" with lines, \"$(UNIFORM_ID).csv\" using 1:4 title \"Tartuttavat\" with lines, \"$(UNIFORM_ID).csv\" using 1:5 title \"Toipuneet\" with lines"
