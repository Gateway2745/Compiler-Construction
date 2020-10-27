parser: fileio.c main.c printParseTree.c printTypeExpressionTable.c readGrammar.c stack.c tokeniseSourcecode.c traverseParseTree.c
	gcc fileio.c main.c printParseTree.c printTypeExpressionTable.c readGrammar.c stack.c tokeniseSourcecode.c traverseParseTree.c -o parser
dev_parser: fileio.c main_dev.c printParseTree.c printTypeExpressionTable.c readGrammar.c stack.c tokeniseSourcecode.c traverseParseTree.c
	gcc fileio.c main_dev.c printParseTree.c printTypeExpressionTable.c readGrammar.c stack.c tokeniseSourcecode.c traverseParseTree.c -o dev_parser