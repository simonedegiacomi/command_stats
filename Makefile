.PHONY: build clean

build: $(wildcard src/**/*.c,src/**/*.h)
	mkdir -p bin
	gcc -std=gnu90 src/*.c src/**/*.c -o bin/run


clean:
	rm -r bin/
