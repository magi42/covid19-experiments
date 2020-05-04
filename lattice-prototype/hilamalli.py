#!/usr/bin/python

import os
import numpy as np
import moviepy
import random
import math
from moviepy.editor import ImageSequenceClip

# From https://stackoverflow.com/questions/13041893/generating-an-animated-gif-in-python
def create_gif(filename, array, fps=10, scale=1.0):
    """creates a gif given a stack of ndarray using moviepy
    Parameters
    ----------
    filename : string
        The filename of the gif to write to
    array : array_like
        A numpy array that contains a sequence of images
    fps : int
        frames per second (default: 10)
    scale : float
        how much to rescale each image by (default: 1.0)
    """
    fname, _ = os.path.splitext(filename)   #split the extension by last period
    filename = fname + '.gif'               #ensure the .gif extension
    if array.ndim == 3:                     #If number of dimensions are 3,
        array = array[..., np.newaxis] * np.ones(3)   #copy into the color
                                                      #dimension if images are
                                                      #black and white
    clip = ImageSequenceClip(list(array), fps=fps).resize(scale)
    clip.write_gif(filename, fps=fps)
    return clip

# A xy population with timescale iterations
xSize=300
ySize=300
timescale=200
lattice = np.zeros((timescale,xSize,ySize))

SUSCEPTIBLE = 0
EXPOSED     = 1
INFECTED    = 2
RECOVERED   = 3

statecolors = [(0,0,0),
               (128,255,128),
               (255,0,0),
               (128,128,255)]

# First infected
lattice[0,xSize/2,ySize/2] = INFECTED

def randomNeighbourLog():
    while True:
        rel = (int(-math.log(random.random())/math.log(2))*(random.randint(0,1)*2-1),
               int(-math.log(random.random())/math.log(2))*(random.randint(0,1)*2-1))
        if rel != (0,0):
            return rel

def randomNeighbourDiv():
    while True:
        rel = (int(0.5/(random.random()*2.0-1.0)),
               int(0.5/(random.random()*2.0-1.0)))
        if rel != (0,0):
            return rel

# Run the simulation
def runSimulation():
    for i in xrange(1,timescale):
        # Copy the state to current timestep
        for y in xrange(0,ySize):
            for x in xrange(0,xSize):
                # By default, the state doesn't change
                lattice[i,x,y] = lattice[i-1,x,y]

        for y in xrange(0,ySize):
            for x in xrange(0,xSize):
                # Latency period ends
                if (lattice[i-1,x,y] == EXPOSED) and random.random() > 0.7:
                    lattice[i,x,y] = INFECTED

                # Contagion
                if lattice[i-1,x,y] == INFECTED:
                    neighbour = randomNeighbourDiv()
                    infX = int(int(round(x)) + neighbour[0]) % xSize
                    infY = int(int(round(y)) + neighbour[1]) % ySize
                    # print "x,y=%d,%d inf=%d,%d" % (x,y,infX,infY)
                    if lattice[i,infX,infY] == SUSCEPTIBLE:
                        lattice[i,infX,infY] = EXPOSED

                # Recovering
                if (lattice[i-1,x,y] == INFECTED) and random.random() > 0.5:
                    lattice[i,x,y] = RECOVERED
    return lattice

def visualize(lattice):
    movie = np.zeros((timescale,xSize,ySize,3))
    for i in xrange(1,timescale):
        for y in xrange(0,ySize):
            for x in xrange(0,xSize):
                movie[i,x,y] = statecolors[int(lattice[i,x,y])]

    create_gif('test.gif', movie, fps=10, scale=2)

if __name__ == "__main__":
    lattice = runSimulation()
    visualize(lattice)
