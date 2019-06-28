CXX=g++
INC=-I=.
CXXFLAGS ?= $(INC_FLAGS) -Wall -g -O0 --std=c++17 $(INC) -pthread
LIBS=-lprotobuf


TARGET_EXEC=phenotools


ALL=$(TARGET_EXEC)

OBJS =  base.pb.o interpretation.pb.o phenopackets.pb.o phenotools.o jsonobo.o ontology.o

$(TARGET_EXEC): $(OBJS) main.cpp
	$(CXX) $(OBJS) main.cpp $(CXXFLAGS) -o $@ $(LIBS)

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


unittests: unittests.cpp $(OBJS)
	 $(CXX) $(CXXFLAGS) unittests.cpp $(OBJS) -o $@ $(LIBS)


.PHONY: clean

clean:
	-rm *.o
	-rm phenotools
	-rm base.pb.*
	-rm interpretation.pb.*
	-rm phenopackets.pb.*
