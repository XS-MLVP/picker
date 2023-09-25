CPPFLAGS = 

include Vtop.mk

default: libmv.so

SHIM_LIBS += mvcontext.o mvtop.o mvwave.o

libmv.so: $(VK_USER_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.o $(VM_HIER_LIBS) $(SHIM_LIBS)
	$(LINK) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(LIBS) $(SC_LIBS) -shared -fPIC -o libmv.so

clean:
	rm -rf *.o *.so *.d bin

