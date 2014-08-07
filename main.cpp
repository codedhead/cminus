#include "_lex.h"
#include "_parser.h"
#include "cminus.h"
#include <cstdio>
#include <string>
using namespace std;

int Param::param_cnt=0;
int Arg::arg_cnt=0;
Arg* Arg::first=0;

CMinus cminus;

int yylex();
Parser parser(yylex);
Lex lex(parser.yylval);

int main(int argc, char* argv[])
{
 	if(argc!=2)
 	{
 		printf("usage: cminus filename\n");
 		return -1;
 	}

	char* buffer=NULL;

	FILE* inf=fopen(argv[1],"r");
	
	if(!inf)
	{
		printf("cannot open the file\n");
		return -1;
	}
	string srcfilename;
	srcfilename=argv[1];
	int fpos,fpos2;
	fpos=srcfilename.find_last_of('\\');
	fpos2=srcfilename.find_last_of('/');
	if(fpos2>fpos) fpos=fpos2;
	if(fpos!=-1) srcfilename.erase(0,fpos+1);
	fpos=srcfilename.find_last_of('.');
	if(fpos!=-1) srcfilename.erase(fpos,srcfilename.length()-fpos);
	srcfilename.append(".asm");

	fseek(inf,0,SEEK_END);
	unsigned long filesize=ftell(inf);
	fseek(inf,0,SEEK_SET);

	unsigned long MAXBUFFER=filesize+20;
	buffer=new char[MAXBUFFER];

	int cnt=fread(buffer,sizeof(char),MAXBUFFER,inf);
	fclose(inf);
	buffer[cnt]='\0';

	cminus.setFileName(srcfilename.c_str());
	

	lex.scanNewCode(buffer);
	parser.yyparse(buffer);
	cminus.generateASM();

	delete [] buffer;
}
int yylex()
{
	return lex.yylex();
}