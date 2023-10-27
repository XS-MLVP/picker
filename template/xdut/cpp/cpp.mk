CPPFLAGS = -DVL_DEBUG=1

include V{{__TOP_MODULE_NAME__}}.mk

default: libV{{__TOP_MODULE_NAME__}}.a

libV{{__TOP_MODULE_NAME__}}.a: $(VK_USER_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.o $(VM_HIER_LIBS)
	ar rcs -o libV{{__TOP_MODULE_NAME__}}.a $^ 