CXX = g++
CFLAGS := `wx-config --cxxflags` -I../../src -g -Wall
CXXFLAGS = $(CFLAGS)
LDFLAGS =
LIBS := `wx-config --libs`
EXE = gamemonitor

SRCS = SlowReader.cpp MainWindow.cpp ReadThread.cpp SliderDialog.cpp Utils.cpp main.cpp
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

run:
	./$(EXE) master.log

clean:
	rm -f $(OBJS) $(EXE)
	rm -f core*
