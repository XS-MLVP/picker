CXXFLAGS=-I /usr/local/share/verilator/include -I ../../../../example/hello/obj_dir


default: MVTop.a

binary: main.o MVTop.a
	$(CXX) $(OPT_FAST) $(CXXFLAGS) $(CPPFLAGS) -o binary MVTop.a main.o$(LDFLAGS) $(LDLIBS)

MVTop.a: Vhello__ALL.a MVShim.a
	ar rcs MVTop.a Vhello__ALL.a MVShim.a

MVShim.a: mvtop.o mvcontext.o
	ar rcs MVShim.a mvtop.o mvcontext.o

%.o: %.cpp
	$(OBJCACHE) $(CXX) $(OPT_FAST) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf MVShim.a MVTop.a *.o binary
