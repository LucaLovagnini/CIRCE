appname := CloudCache

LOCAL=$(HOME)/ParallelOpenCV/install_gcc
INCLUDE=$(LOCAL)/include
OPENCVLIB=$(LOCAL)/lib
BOOST=$(INCLUDE)/boost
VLFEAT=$(HOME)/vlfeat
VLFEATLIB=$(VLFEAT)/bin/glnxa64

PARENT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

DISABLECUDA=true

ifeq ($(DISABLECUDA),true)
	CXXDISABLED := -DCC_DISABLE_CUDA
endif

LCS=-DRN -DLCS -DSSR

CXX := icpc
CXXFLAGS := -DRATIOTHRESHOLD=0.3 $(LCS) $(CXXDISABLED) -I/home/luca/eigen/ -I$(PARENT_DIR) -I$(INCLUDE) -I$(VLFEAT) -g -Werror -Wall -xCORE-AVX2 -O3 -qopenmp -parallel -no-prec-div -ansi-alias -std=c++11


LDFLAGS= -L$(OPENCVLIB) -L$(VLFEATLIB)
LDLIBS= -lpython2.7 -lopencv_core -lboost_filesystem -lboost_system -lopencv_xfeatures2d -lopencv_highgui -lopencv_imgproc -lvl -lopencv_imgcodecs -qopenmp

ifeq ($(DISABLECUDA),true)
	srcfiles := $(shell find .. -name "*.cpp" ! -name "SURF_CUDAOpenCV.cpp")
	headerfiles := $(shell find .. -name "*.hpp" ! -name "SURF_CUDAOpenCV.hpp")
else
	srcfiles := $(shell find .. -name "*.cpp")
	headerfiles := $(shell find .. -name "*.hpp")
endif

objects := $(patsubst %.cpp,%.o,$(notdir $(srcfiles)))

VPATH := $(sort $(dir $(srcfiles)))

all: $(srcfiles) $(appname)

$(appname): $(objects)
	$(CXX) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS)

depend: .depend

.depend: $(srcfiles) $(headerfiles)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) $(CXXDISALBE) -MM $^>>./.depend;

clean:
	rm -f $(appname) $(objects)

dist-clean: clean
	rm -f *~ .depend

include .depend

