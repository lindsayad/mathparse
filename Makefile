CXX ?= clang++

OBJS := SymbolicMathTokenizer.o SymbolicMathParser.o SymbolicMathSymbols.o

mathparse: main.C $(OBJS)
	$(CXX) -std=c++11 $(LDFLAGS) -o mathparse main.C $(OBJS)

-include $(OBJS:.o=.d)

%.o : %.C
	$(CXX) -std=c++11 -c $(CXXFLAGS) $(CPPFLAGS) $*.C -o $@
	$(CXX) -std=c++11 -MM $(CXXFLAGS) $*.C > $*.d

clean:
	rm -rf $(OBJS) *.o *.d
