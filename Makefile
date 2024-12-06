.PHONY: all clean
all: $(PROG) doxystamp
SRCS = $(wildcard *.c)
PROG = $(shell basename $$PWD)
OBJ = $(SRCS:.c=.o)
CPPFLAGS = $(shell getconf LFS_CFLAGS 2>/dev/null)
CFLAGS := $(CFLAGS) -g3 -gdwarf-4 \
  -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes \
  -Wmissing-prototypes
$(PROG) : $(OBJ)
	$(CC) $^ -o $@ $(LIBS)
clean :
	rm -f *.o *.d $(PROG) doxystamp

# Auto dependency stuff (from info make)
%.d: %.c
	$(CC) -MM -MT $(@:.d=.o) -MT $@ $(CPPFLAGS) $< -o $@
ifneq ($(MAKECMDGOALS),clean)
-include $(SRCS:.c=.d)
endif

# Run doxygen
doxystamp: $(SRCS) $(wildcard *.h) build_man.sh Makefile Doxyfile
	rm -rf html man
	doxygen >/dev/null
	./build_man.sh
	touch doxystamp
