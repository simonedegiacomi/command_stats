.PHONY: build tests clean

build: $(wildcard src/**/*.c,src/**/*.h)
	mkdir -p bin
	gcc -Wall -std=gnu90 src/*.c src/**/*.c -o bin/command_stats

tests: $(wildcard src/**/*.c,src/**/*.h,test/*.c,test/*.h)
	mkdir -p bin
	gcc -Wall -std=gnu90 test/all.c test/utils.c src/*/*.c -o bin/tests
	bin/tests

clean:
	rm -r bin/
