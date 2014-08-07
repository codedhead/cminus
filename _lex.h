#ifndef _YANJUN_LEX_H_
#define _YANJUN_LEX_H_

#include "YYSTYPE.h"
#include "yacclimits.h"
#include "ctype.h"

#define C_DECLARATION 111
#define C_STATEMENTS 222

class Lex
{
private:
	char* code;
	int startState;	
	char yytext[YYTEXT_MAX_LENGTH];
	
	YYSTYPE& yylval;
	
	int status;

	
	
	int retType(int st);
	
public:
	int linecnt;
	Lex(YYSTYPE& y):yylval(y){code=0;startState=1;yytext[0]=0;status=C_DECLARATION;linecnt=1;}

//	generally, the yylval comes from the class Parser,
//	if there is no need to use yylval, 
//	comment out the include line of YYSTYPE, the declaration of yylval and the corresponding constructor 
//	and de-comment the following constructor
//
// 	Lex(){startState=1;}
	
	const char* getText(){return yytext;}
	void scanNewCode(char* c){code=c;startState=1;yytext[0]=0;status=C_DECLARATION;linecnt=1;}
	int setStartState(int s){startState=s;}

	int yylex();
	
	/* add other methods here */
	void comment()
	{	
loop:
		while(*code&&*code!='*')
		{
			if(*code=='\n') ++linecnt;
			++code;
		}
		if(*code) ++code;
		if(*code&&*code!='/')
			goto loop;

		if(*code) ++code;
	}

	void changstate()
	{
		status=C_STATEMENTS;
	}

	int oct2dec(char* s)
	{
		int res=0;
		++s;
		while(isdigit(*s)||isalpha(*s))
			res=(res<<3)+(*s++)-'0';
		return res;
	}
	int hex2dec(char* s)
	{
		int res=0;
		++s;
		while(isdigit(*s)||isalpha(*s))
		{
			if(*s>='0'&&*s<='9')
				res=(res<<4)+*s-'0';
			else if(*s>='a'&&*s<='f')
				res=(res<<4)+10+*s-'a';
			else if(*s>='A'&&*s<='F')
				res=(res<<4)+10+*s-'A';
			++s;
		}
		return res;
	}

	int handleTransChar(char* s)
	{
		switch(*s)
		{
		case 'b':
			return '\b';
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case '0':
			if(*(s+1)=='x'||*(s+1)=='X') return hex2dec(s+1);
			else return oct2dec(s);
		case 'x':
		case 'X':
			return hex2dec(s);
		default:
			return *s;
		}
	}
	void copyAsmBlock()
	{
		char* p=yylval.asmblock;
		int i=1;

asmcopyloop:
		while(*code&&*code!='%')
		{
			if(*code=='\n') ++linecnt;
			if(++i<C_ASMBLOCK_LENGTH)
				*p++=*code;
			++code;++i;
		}
		if(*code) ++code;
		if(*code&&*code!='}')
			goto asmcopyloop;

		if(*code) ++code;
		*p=0;
	}
};

#endif
