#include Make.inc
# include Make_msys2.inc
#include Make_osx.inc
include Make_linux.inc

CXXFLAGS = -std=c++11 -fPIC
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -D_GLIBCXX_DEBUG
else
CXXFLAGS += -O3 -march=native -Wall
endif

ALL=matvecRows.exe matvecCols.exe Mandelbrot.exe MandelbortEsclave.exe

default:	help

all: $(ALL)

clean:
	@rm -fr *.o *.exe *~ *.tga

MandelbrotEsclave.exe:	MandelbrotEsclave.cpp
	$(CXX) $(CXXFLAGS) -o MandelbrotEsclave.exe MandelbrotEsclave.cpp $(LIB)

%.exe: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)


help:
	@echo "Available targets : "
	@echo "    all            			: compile all executables"
	@echo "    matvecRows.exe     		: compile matrix vector product executable"
	@echo "    matvecCols.exe     		: compile matrix vector product executable"
	@echo "    Mandelbrot.exe 			: compile Mandelbrot set computation executable"
	@echo "    MandelbrotEsclave.exe 	: compile MandelbrotEsclave set computation executable"
	@echo "Add DEBUG=yes to compile in debug"
	@echo "Configuration :"
	@echo "    CXX      :    $(CXX)"
	@echo "    CXXFLAGS :    $(CXXFLAGS)"
