CXX=g++
INC=-I=.
LIBS=-lprotobuf


TARGET_EXEC=phenotools


SRC_DIR = src

SRCS := $(shell find $(SRC_DIRS) -name *.cc)
OBJS := $(SRCS:%=build/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CXXFLAGS ?= $(INC_FLAGS) -Wall -g -O0 --std=c++17 $(INC) -pthread

build/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LIBS)


# c++ source
build/%.cpp.o: %.cpp
	echo "build $(BUILD_DIR)"
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

