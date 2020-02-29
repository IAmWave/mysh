CFLAGS=-std=c99 -Wall -Wextra -Wshadow \
	   -Werror=incompatible-pointer-types \
	   -Werror=implicit-function-declaration

all: build/mysh

build/:
	mkdir -p build

clean:
	rm -r build

build/mysh.tab.c build/mysh.tab.h: build/ src/mysh_parser.y
	bison -o build/mysh.tab.c -d src/mysh_parser.y
	
build/mysh.yy.c: build/ src/mysh_parser.l build/mysh.tab.h
	flex -o build/mysh.yy.c src/mysh_parser.l

SOURCE_FILES=src/main.c src/mysh.c src/command.c src/util.c src/pipeline.c

build/mysh: build/ build/mysh.yy.c build/mysh.tab.c build/mysh.tab.h $(SOURCE_FILES)
	gcc \
		-D_XOPEN_SOURCE=700 \
		-I build/ -I src/ \
		build/mysh.tab.c build/mysh.yy.c $(SOURCE_FILES) \
		$(CFLAGS) $(EXTRA_FLAGS) \
		-o build/mysh
