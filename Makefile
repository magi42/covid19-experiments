
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
	target/lattice generations=150 R0=4.8 neighbourhood=uniform animation=$(UNIFORM_ID).gif report=$(UNIFORM_ID).csv
	gnuplot -p -e "plot \"$(UNIFORM_ID).csv\" using 1:2 title \"Alttiit\" with lines, \"$(UNIFORM_ID).csv\" using 1:3 title \"Tarttuneet\" with lines, \"$(UNIFORM_ID).csv\" using 1:4 title \"Tartuttavat\" with lines, \"$(UNIFORM_ID).csv\" using 1:5 title \"Toipuneet\" with lines"

TIGHT_ID=results/tight-2.4-1000g
run-tight:
	mkdir -p results
	target/lattice generations=850 R0=2.4 neighbourhood=small suppression=0.2 animation=$(TIGHT_ID).gif report=$(TIGHT_ID).csv
	gnuplot -p -e "plot \"$(TIGHT_ID).csv\" using 1:2 title \"Alttiit\" with lines, \"$(TIGHT_ID).csv\" using 1:3 title \"Tarttuneet\" with lines, \"$(TIGHT_ID).csv\" using 1:4 title \"Tartuttavat\" with lines, \"$(TIGHT_ID).csv\" using 1:5 title \"Toipuneet\" with lines"

SMALL_ID=results/small-2.4-400g
run-small:
	mkdir -p results
	target/lattice generations=230 R0=2.4 neighbourhood=small suppression=0.02 animation=$(SMALL_ID).gif report=$(SMALL_ID).csv
	gnuplot -p -e "plot \"$(SMALL_ID).csv\" using 1:2 title \"Alttiit\" with lines, \"\" using 1:3 title \"Tarttuneet\" with lines, \"\" using 1:4 title \"Tartuttavat\" with lines, \"\" using 1:5 title \"Toipuneet\" with lines"
