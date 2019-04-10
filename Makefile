CXX=g++
INC=-I=.
CXXFLAGS=-Wall --std=c++17 $(INC)
LIBS=-lprotobuf


TARGET=phenopacket_demo

all:$(TARGET)


$(TARGET):main.cpp phenopackets.pb.o
	$(CXX)   $< $(CXXFLAGS) ${LIBS} -O0 -o $@

phenopackets.pb.o:phenopackets.pb.cc
	$(CXX) $(CXXFLAGS)  phenopackets.pb.cc -o $@

phenopackets.pb.cc: phenopackets.proto
	protoc --proto_path=. --cpp_out=. phenopackets.proto


clean:
	rm -f $(TARGETS)
