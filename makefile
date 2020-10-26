parser: fileio.c main.c printParseTree.c readGrammar.c stack.c tokeniseSourcecode.c
	gcc fileio.c main.c printParseTree.c readGrammar.c stack.c tokeniseSourcecode.c -o parser
dev_parser: fileio.c main_dev.c printParseTree.c readGrammar.c stack.c tokeniseSourcecode.c
	gcc fileio.c main_dev.c printParseTree.c readGrammar.c stack.c tokeniseSourcecode.c -o dev_parser