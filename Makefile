CXX=g++
INC=-I=.
CXXFLAGS=-Wall --std=c++17 $(INC)
LIBS=-lprotobuf


TARGET=phenopacket_demo

all:$(TARGET)

OBJS=phenopackets.pb.o


$(TARGET):main.cpp phenopackets.pb.o
	$(CXX)   $< $(OBJS) $(CXXFLAGS) ${LIBS} -O0 -o $@

phenopackets.pb.o:phenopackets.pb.cc
	$(CXX) $(CXXFLAGS) -c  phenopackets.pb.cc 

phenopackets.pb.cc: phenopackets.proto
	protoc --proto_path=. --cpp_out=. phenopackets.proto


clean:
	rm -f $(TARGETS)
