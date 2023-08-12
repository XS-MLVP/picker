CPPFLAGS = {{cpp_flages}}

include {{dut_name}}.mk

CPPLIB: $(VK_USER_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.o $(VM_HIER_LIBS)
	$(LINK) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(LIBS) $(SC_LIBS) -shared -fPIC -o cpp/{{cpplib_filename}}
