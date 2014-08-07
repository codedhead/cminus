#ifndef _YANJUN_PARSER_H_
#define _YANJUN_PARSER_H_

#include <stdio.h>
#include <string.h>

#include "YYSTYPE.h"
#include "yacclimits.h"

#include "quadruple.h"
#include<stack>
using namespace std;

struct StackEntry
{
	StackEntry(){state=-1;memset(&val,0,sizeof(YYSTYPE));}
	StackEntry(int s):state(s){memset(&val,0,sizeof(YYSTYPE));}
	StackEntry(int s,YYSTYPE v):state(s),val(v){}
	int state;
	YYSTYPE val;
};

class Stack
{
public:
	Stack();
	~Stack();
	bool isEmpty();
	bool isFull();
	void init();
	StackEntry& getAt(int n);
	int top();//top position, not value
	void pop();
	bool push(const StackEntry& x);
	const StackEntry& peek();
private:
	StackEntry element[PARSESTACK_SIZE];
	int _top;
};

struct IterationLabel
{
	IterationLabel(int t,ForLabel& fr):type(t),for_label(fr){}
	IterationLabel(int t,WhileLabel& wh):type(t),while_label(wh){}
	IterationLabel(int t,DoLabel& d):type(t),do_label(d){}
	int type;
	union
	{
		ForLabel for_label;
		WhileLabel while_label;
		DoLabel do_label;
	};
};

class Parser
{
private:
	Stack sstack;

	bool func_def_showup;
	SymbolInfo* func_def_info;
	int block_level;
	stack<IterationLabel> iteration_stack;
	
	int (*yylex)();
	void handleSematicAction(int rnum);
		
public:	
	YYSTYPE yylval;
	
	Parser(int (*lex)()):yylex(lex){}
	void parseerror(char*);
	bool yyparse(char* symbols);
	
	/* add other methods here */
};

#endif
