default: export.bin


TOP_NAME?=
V_ROOT=$(shell $(shell echo $$SHELL) -c  "verilator -V | grep VERILATOR_ROOT | awk '{print \$$3}' | head -n1")

CXXFLAGS += -I ${V_ROOT}/include -I ${V_ROOT}/include/vltstd/ \
		-I ../build/DPI${TOP_NAME} \
		-L../build -Wl,--whole-archive -lDPI${TOP_NAME} -Wl,--no-whole-archive \
		-lpthread -lz ${CFLAGS}

# OBJS contains the list of object files generated from all .cpp files in the directory
OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

export.bin: $(OBJS)
	$(CXX) $(CXXFLAGS) -o export.bin $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
