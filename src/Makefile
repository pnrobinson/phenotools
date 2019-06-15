CXX=g++
INC=-I=.
CXXFLAGS ?= $(INC_FLAGS) -Wall -g -O0 --std=c++17 $(INC) -pthread
LIBS=-lprotobuf


TARGET_EXEC=phenotools


ALL=$(TARGET_EXEC)

#SRCS := $(shell find -name *.cc)
SRCS = base.pb.cc interpretation.pb.cc phenopackets.pb.cc
#OBJS := $(SRCS:%=%.o)
OBJS =  base.pb.o interpretation.pb.o phenopackets.pb.o phenotools.o

$(TARGET_EXEC): $(OBJS) main.cpp
	$(CXX) $(OBJS) main.cpp $(CXXFLAGS) -o $@ $(LIBS)

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	-rm *.o
	-rm phenotools
	-rm base.pb.*
	-rm interpretation.pb.*
	-rm phenopackets.pb.*
