CXXFLAGS += -MMD -std=c++14 -Wall -Wextra -Werror -g -fmax-errors=4

.PHONY: all
all: a.out

a.out: Main.o
	$(CXX) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(wildcard *.o)
	$(RM) $(wildcard *.d)
	$(RM) a.out

-include $(wildcard *.d)
