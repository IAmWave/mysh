build/snazzle.tab.c build/snazzle.tab.h: src/snazzle.y
	bison -d src/snazzle.y -o build/snazzle.tab.c

build/lex.yy.c: src/snazzle.l build/snazzle.tab.h
	flex -o build/lex.yy.c src/snazzle.l

build/snazzle: build/lex.yy.c build/snazzle.tab.c build/snazzle.tab.h
	gcc build/snazzle.tab.c build/lex.yy.c -lfl -o build/snazzle
