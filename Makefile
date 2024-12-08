.PHONY: all clean
all: doxystamp
clean :
	rm -f doxystamp

# Run doxygen
doxystamp: *.c *.h build_man.sh Makefile Doxyfile
	rm -rf html man
	doxygen
	./build_man.sh
	touch doxystamp
