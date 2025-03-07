CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE

PROGS := find cp0 cp1 ps

all: $(PROGS)

find: find.o find_tools.c
cp1: cp1.o cp1_tools.c
cp0: cp0.o
ps: ps.o ps_tools.c

test:
	./tests/run $(realpath .)

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean format test
