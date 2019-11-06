build/mysh.tab.c build/mysh.tab.h: src/mysh.y
	bison -d src/mysh.y -o build/mysh.tab.c

build/lex.yy.c: src/mysh.l build/mysh.tab.h
	flex -o build/lex.yy.c src/mysh.l

build/mysh: build/lex.yy.c build/mysh.tab.c build/mysh.tab.h
	gcc build/mysh.tab.c build/lex.yy.c -lfl -o build/mysh
