 .PHONY: default
default: export.bin

TOP_NAME?=

{% if __SIMULATOR__ == "verilator" %}
# Use the last VERILATOR_ROOT reported by `verilator -V`
V_ROOT := $(shell verilator -V | grep ROOT | grep verilator | tail -n 1 | awk '{print $$3}')

CXXFLAGS += -I ${V_ROOT}/include -I ${V_ROOT}/include/vltstd/ \
		-I ../build/DPI${TOP_NAME} \
		-L../build -Wl,--whole-archive -lDPI${TOP_NAME} -Wl,--no-whole-archive \
		-lpthread -lz ${CFLAGS}
{% endif %}

{% if __SIMULATOR__ == "gsim" %}
CXXFLAGS += -I ../build/_SRC \
		-L../build -Wl,--whole-archive -lUT${TOP_NAME} -Wl,--no-whole-archive \
		-lpthread -lz ${CFLAGS} -Wno-unused-command-line-argument \
		-ldl \
		-Wl,-rpath,./build
{% endif %}

# OBJS contains the list of object files generated from all .cpp files in the directory
OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

export.bin: $(OBJS)
	$(CXX) $(CXXFLAGS) -o export.bin $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
