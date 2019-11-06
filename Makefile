all: build/mysh

build/:
	mkdir -p build

build/mysh.tab.c build/mysh.tab.h: build/ src/mysh_parser.y
	bison -o build/mysh.tab.c -d src/mysh_parser.y
	
build/lex.yy.c: build/ src/mysh_parser.l build/mysh.tab.h
	flex -o build/mysh.yy.c src/mysh_parser.l

SOURCE_FILES=src/main.c src/mysh.c

build/mysh: build/ build/lex.yy.c build/mysh.tab.c build/mysh.tab.h $(SOURCE_FILES)
	gcc src/mysh.c build/mysh.tab.c build/lex.yy.c src/main.c -ll -o build/mysh
