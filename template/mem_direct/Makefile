 .PHONY: default
default: export.bin

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    # macOS: need full path of library
    WHOLE_ARCHIVE_LDFLAGS = -Wl,-force_load,$(LIB_DUT_PATH)
    # macOS need this, according to https://github.com/verilator/verilator/blob/master/include/verilated.mk.in#L114-L118
    LDFLAGS += -Wl,-U,__Z15vl_time_stamp64v,-U,__Z13sc_time_stampv
else
    # Linux/GNU: need the name of library
    WHOLE_ARCHIVE_LDFLAGS = -Wl,--whole-archive -l$(LIB_DUT_NAME) -Wl,--no-whole-archive
endif

{% if __SIMULATOR__ == "verilator" %}
LIB_DUT_NAME = DPI${TOP_NAME}
LIB_DUT_PATH := ../build/libDPI${TOP_NAME}.a

# Use the last VERILATOR_ROOT reported by `verilator -V`
V_ROOT := $(shell verilator -V | grep ROOT | grep verilator | tail -n 1 | awk '{print $$3}')

CXXFLAGS += -I ${V_ROOT}/include -I ${V_ROOT}/include/vltstd/ \
		-I ../build/DPI${TOP_NAME} \
		-L../build  ${CFLAGS}
LDFLAGS += $(WHOLE_ARCHIVE_LDFLAGS)
LDLIBS += -lpthread -lz -lsystemc
{% endif %}

{% if __SIMULATOR__ == "gsim" %}
LIB_DUT_NAME := UT${TOP_NAME}
LIB_DUT_PATH := ../build/libUT${TOP_NAME}.a

CXXFLAGS += -I ../build/_SRC \
		-L../build  \
		 ${CFLAGS} -Wno-unused-command-line-argument
LDFLAGS += $(WHOLE_ARCHIVE_LDFLAGS) -Wl,-rpath,./build # FIXME: Doesn't work in macOS
LDLIBS += -lpthread -lz -ldl
{% endif %}

# OBJS contains the list of object files generated from all .cpp files in the directory
OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

export.bin: $(OBJS)
	$(CXX) $(CXXFLAGS) -o export.bin $^ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS) $(LDLIBS)
