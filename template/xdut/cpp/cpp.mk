CPPFLAGS = -DVL_DEBUG=1

include V{{__TOP_MODULE_NAME__}}.mk

default: libV{{__TOP_MODULE_NAME__}}.so

libV{{__TOP_MODULE_NAME__}}.so: $(VK_USER_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.a $(VM_HIER_LIBS)
	# whole-archive is used to force linking of all objects in the archive
	$(LINK) $(LDFLAGS) -Wl,--whole-archive $^ -Wl,--no-whole-archive \
		$(LOADLIBES) $(LDLIBS) $(LIBS) $(SC_LIBS) -shared -fPIC -o libV{{__TOP_MODULE_NAME__}}.so