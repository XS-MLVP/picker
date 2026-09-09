 .PHONY: default
default: export.bin

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    # macOS: need full path of library
    WHOLE_ARCHIVE_LDFLAGS = -Wl,-force_load,$(LIB_DUT_PATH)
else
    # Linux/GNU: need the name of library
    WHOLE_ARCHIVE_LDFLAGS = -Wl,--whole-archive -l$(LIB_DUT_NAME) -Wl,--no-whole-archive
endif

{% if __SIMULATOR__ == "verilator" %}
LIB_DUT_NAME = DPI${TOP_NAME}
LIB_DUT_PATH := ../build/libDPI${TOP_NAME}.a

# Use the last VERILATOR_ROOT reported by `verilator -V`
V_ROOT := $(shell verilator -V | grep ROOT | grep verilator | tail -n 1 | awk '{print $$3}')
V_VERSION := $(shell verilator --version 2> /dev/null | cut -f2 -d' ' | tr -d '.')

CXXFLAGS += -I ${V_ROOT}/include -I ${V_ROOT}/include/vltstd/ \
		-I ../build/DPI${TOP_NAME} \
		-L../build  ${CFLAGS}
LDFLAGS += $(WHOLE_ARCHIVE_LDFLAGS) -Wl,-undefined,dynamic_lookup -Wl,-rpath,{{__XSPCOMM_LIB__}}
LDLIBS += -lpthread

# Verilator 5.050+ introduced a new FST writer backed by liblz4.
# Link that after DUT and before -lz to follow Verilator upstream practice.
ifeq ($(TRACE),fst)
ifeq ($(shell expr $(V_VERSION) \>= 5050 2> /dev/null),1)
LDLIBS += -llz4
endif
endif

LDLIBS += -lz -L{{__XSPCOMM_LIB__}} -lxspcomm
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
	$(CXX) -o export.bin $^ $(LDFLAGS) $(LDLIBS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
