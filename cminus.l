/* credit info
ANSI C grammar, Lex specification
In 1985, Jeff Lee published this Lex specification together with a Yacc grammar for the April 30, 1985 ANSI C draft.  Tom Stockfisch reposted both to net.sources in 1987; that original, as mentioned in the answer to question 17.25 of the comp.lang.c FAQ, can be ftp'ed from ftp.uu.net, file usenet/net.sources/ansi.c.grammar.Z. 
I intend to keep this version as close to the current C Standard grammar as possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

/* modified by yanjun */

D			[0-9]
OC			[0-7]
L			[a-zA-Z_]
H			[a-fA-F0-9]

%%
"%\{"			{ copyAsmBlock(); return (ASM_BLOCK); }
"/*"			{ comment(); return 0;} /* d */
"break"			{ count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); status=C_DECLARATION; return(CHAR); }
"continue"		{ count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"else"			{ count(); return(ELSE); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"int"			{ count(); status=C_DECLARATION; return(INT); }
"return"		{ count(); return(RETURN); }
"switch"		{ count(); return(SWITCH); }
"void"			{ count(); status=C_DECLARATION; return(VOID); }
"while"			{ count(); return(WHILE); }


'\\?.'			{count(); if(yytext[1]=='\\') yylval.ival=handleTransChar(yytext+2);else yylval.ival=yytext[1];return CHARACTER;}

{L}({L}|{D})*	{ count(); strcpy(yylval.str,yytext);if(status==C_DECLARATION) return DEC_ID;else return(ID); }

0[xX]{H}+		{ count(); yylval.ival=hex2dec(yytext);if(status==C_DECLARATION) return DEC_NUM;else return(NUM); }
0{OC}+			{ count(); yylval.ival=oct2dec(yytext);if(status==C_DECLARATION) return DEC_NUM;else return(NUM); }
{D}+			{ count(); yylval.ival=atoi(yytext);if(status==C_DECLARATION) return DEC_NUM;else return(NUM); }

">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"				{ count(); return(';'); }
("{"|"<%")		{ count(); status=C_DECLARATION;/*hack*/return('\x7b'); }
("}"|"%>")		{ count(); status=C_DECLARATION;/*hack*/return('\x7d'); }
","				{ count(); return(','); }
":"				{ count(); return(':'); }
"="				{ count(); return('='); }
"("				{ count(); return('('); }
")"				{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."				{ count(); return('.'); }
"&"				{ count(); return('&'); }
"!"				{ count(); return('!'); }
"~"				{ count(); return('~'); }
"-"				{ count(); return('-'); }
"+"				{ count(); return('+'); }
"*"				{ count(); return('*'); }
"/"				{ count(); return('/'); }
"%"				{ count(); return('%'); }
"<"				{ count(); return('<'); }
">"				{ count(); return('>'); }
"^"				{ count(); return('^'); }
"|"				{ count(); return('|'); }
"?"				{ count(); return('?'); }

[ \t\v\n\f]		{ count(); return 0;}
.				{ /* ignore bad characters */  return 0;}
