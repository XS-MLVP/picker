CPPFLAGS = $(shell python3 -m pybind11 --includes) {{cpp_flages}}
PYLIBFILE = {{mode_name}}$(shell python3-config --extension-suffix)

include {{dut_name}}.mk

{{dut_name}}_python.o: {{dut_name}}_python.cpp

PYTHONLIB: $(VK_USER_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.o $(VM_HIER_LIBS) {{dut_name}}_python.o
	$(LINK) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) $(LIBS) $(SC_LIBS) -shared -fPIC -o python/$(PYLIBFILE)
