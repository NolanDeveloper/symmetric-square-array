CXXFLAGS += -MMD -std=c++14

.PHONY: all
all: a.out

a.out: main.o
	$(CXX) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o)
	$(RM) $(wildcard *.d)
	$(RM) a.out

-include $(wildcard *.d)
