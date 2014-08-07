%{
#include "_lex.h"
#include "_tokens.h"
#include "cminus.h"
extern Lex lex;
extern CMinus cminus;

#include "quadruple.h"

%}

%union { 
	ival;
	str;
	asmblock;
	sym_info;
	params_info;
	param;
	operand;
	while_label;
	for_label;
	do_label;
	args_info;
	arg;
}

%token<str> DEC_ID ID
%token DEC_NUM NUM CHARACTER
%token LEFT_OP RIGHT_OP INC_OP DEC_OP PTR_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME
%token INT VOID CHAR
%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token<asmblock> ASM_BLOCK

%start program

%type<ival> specifier single_var_declaration dec_array_index type_specifier
%type<sym_info> single_var_declaration single_var_declaration_s declarator
%type<params_info> params
%type<param> param_list

%type<operand> ref_array_index factor unary_expression term additive_expression relational_expression 
			   logical_and_expression logical_or_expression conditional_expression expression 

%type<while_label> while_clause
%type<for_label> for_head for_condition
%type<do_label> do do_statement

%type<arg> arg_list
%type<args_info> args

%%

program
	: declaration_list
	;

declaration_list
	: declaration_list declaration
	| declaration
	;

declaration
	: var_declaration
	| fun_declaration
	;

fun_declaration
	: single_var_declaration compound_stmt {cminus.leaveFunc($1);cminus.leaveScope();func_def_showup=false;}
	;

var_declaration
	: single_var_declaration ';'		{func_def_showup=false;cminus.declare($1);}
	| single_var_declaration_s declarator_list ';'	{cminus.declare($1);}
	;

single_var_declaration
	: type_specifier declarator			{func_def_showup=true;$2->type=$1;$$=$2;}
	;

declarator_list
	: declarator						{$1->type=$<sym_info>0->type;cminus.declare($1);}
	| declarator_list ',' declarator	{$3->type=$<sym_info>0->type;cminus.declare($3);}
	;

dec_array_index
	: '[' ']'					{$$=-1;}
	| '[' DEC_NUM ']'			{$$=$2;}
	;

declarator
	: DEC_ID					{$$=new SymbolInfo;strcpy($$->name,$1);$$->kind=KIND_VARIABLE;}
	| DEC_ID dec_array_index 	{$$=new SymbolInfo;strcpy($$->name,$1);$$->kind=KIND_ARRAY;$$->arr_size=$2;/*pointer*/}
	| DEC_ID '(' params ')'		{$$=new SymbolInfo;strcpy($$->name,$1);$$->kind=KIND_FUNCTION;$$->params_info=$3;}
	;

type_specifier
	: INT						{$$=INT;}
	| CHAR						{$$=CHAR;}
	| VOID						{$$=VOID;}
	;

params
	: type_specifier			{$$=new ParamsInfo;if($1!=VOID)puts("missing param name.");/*else*/ $$->param_cnt=0;$$->first=NULL;}
	| param_list				{$$=new ParamsInfo(Param::param_cnt,$1);}
	;

param_list
	: single_var_declaration				{func_def_showup=false;Param::param_cnt=0;$$=new Param($1);}
	| single_var_declaration_s param_list	{$$=new Param($1,$2);}
	;

single_var_declaration_s
	: single_var_declaration ','			{func_def_showup=false;$$=$1;}
	;

left_brace
	: '{'		{cminus.enterScope();if(func_def_showup==true){func_def_info=$<sym_info>0;func_def_showup=false;cminus.enterFunc(func_def_info);}}
	;

compound_stmt
	: left_brace local_declarations statement_list '}'	
	;

local_declarations
	: 
	| local_declarations var_declaration
	;

statement_list
	: { if(a!='\x7d') lex.changstate();cminus.afterDeclaration(); }
	| statement_list statement
	;

statement
	: expression_stmt
	| compound_stmt					{cminus.leaveScope();}
	| selection_stmt
	| iteration_stmt				{iteration_stack.pop();}
	| jump_stmt
	| ASM_BLOCK						{cminus.addASMBlock($1);}
	;

expression_stmt
	: expression ';'					{/*do nothing*/;}
	| ';'
	;

if_expression
	: IF '(' expression ')'		{$$=cminus.nextlabel();cminus.addJz($3,$$);}
	;

if_clause
	:  if_expression statement	{$$=$1;}
	;

if_clause_else
	: if_clause ELSE					{$$=cminus.nextlabel();cminus.addJmp($$);cminus.placelabel($1);}
	;

selection_stmt
	: if_clause						{cminus.placelabel($1);}
	| if_clause_else statement		{cminus.placelabel($1);}
	;

while
	: WHILE							{$$=cminus.nextlabel();cminus.placelabel($$);}
	;

while_clause
	: while '(' expression ')'	{$$.start=$1;$$.end=cminus.nextlabel();cminus.addJz($3,$$.end);iteration_stack.push(IterationLabel(WHILE,$$));}
	;

for_start
	: FOR '(' expression_stmt		{$$=cminus.nextlabel();cminus.placelabel($$);}
	;

for_condition
	: for_start ';' 	{$$.condition=-1;$$.increment=cminus.nextlabel();$$.body=cminus.nextlabel();$$.end=cminus.nextlabel();cminus.addJmp($$.body);cminus.placelabel($$.increment);}
	| for_start expression	';'	{$$.condition=$1;$$.increment=cminus.nextlabel();$$.body=cminus.nextlabel();$$.end=cminus.nextlabel();cminus.addJz($2,$$.end);cminus.addJmp($$.body);cminus.placelabel($$.increment);}
	;

for_head
	: for_condition ')'				{$$=$1;if($$.condition!=-1)cminus.addJmp($$.condition);cminus.placelabel($1.body);iteration_stack.push(IterationLabel(FOR,$$));}
	| for_condition expression ')'	{$$=$1;if($$.condition!=-1)cminus.addJmp($$.condition);cminus.placelabel($1.body);iteration_stack.push(IterationLabel(FOR,$$));}
	;

do
	: DO				{$$.start=cminus.nextlabel();$$.condition=cminus.nextlabel();$$.end=cminus.nextlabel();cminus.placelabel($$.start);iteration_stack.push(IterationLabel(DO,$$));}
	;

do_statement
	: do statement		{$$=$1;cminus.placelabel($$.condition);}
	;

iteration_stmt
	: while_clause statement		{cminus.addJmp($1.start);cminus.placelabel($1.end);}
	| do_statement while '(' expression ')' ';'	{cminus.addJnz($4,$1.start);cminus.placelabel($1.end);}
	| for_head statement	{cminus.addJmp($1.increment);cminus.placelabel($1.end);}
	;

jump_stmt
	: CONTINUE ';'					{if(!iteration_stack.empty())con}
	| BREAK ';'						{if(!iteration_stack.empty())break}
	| RETURN ';'					{cminus.addReturn(func_def_info);}
	| RETURN expression ';'			{cminus.addReturn(func_def_info,$2);}
	;

expression
	: factor '=' expression								{$$=cminus.addQuad(OP_ASS,$1,$3);}
	| conditional_expression							{$$=$1;}
	;

conditional_expression
	: logical_or_expression													{$$=$1;}
	| logical_or_expression '?' expression ':' conditional_expression       {$$=cminus.addConditional($1,$3,$5);}
	;

logical_or_expression
	: logical_and_expression								{$$=$1;}
	| logical_or_expression OR_OP logical_and_expression     {$$=cminus.addQuad(OP_LOR,$1,$3);}
	;

logical_and_expression
	: relational_expression									{$$=$1;}
	| logical_and_expression AND_OP relational_expression	{$$=cminus.addQuad(OP_LAND,$1,$3);}
	;

relational_expression
	: relational_expression '<' additive_expression		{$$=cminus.addQuad(OP_L,$1,$3);}
	| relational_expression '>' additive_expression		{$$=cminus.addQuad(OP_G,$1,$3);}
	| relational_expression LE_OP additive_expression		{$$=cminus.addQuad(OP_LE,$1,$3);}
	| relational_expression GE_OP additive_expression		{$$=cminus.addQuad(OP_GE,$1,$3);}
	| relational_expression EQ_OP additive_expression		{$$=cminus.addQuad(OP_EQ,$1,$3);}
	| relational_expression NE_OP additive_expression		{$$=cminus.addQuad(OP_NE,$1,$3);}
	| additive_expression								{$$=$1;}
	;

additive_expression
	: additive_expression '+' term	{$$=cminus.addQuad(OP_ADD,$1,$3);}
	| additive_expression '-' term	{$$=cminus.addQuad(OP_SUB,$1,$3);}
	| term							{$$=$1;}
	;

term
	: term '*' unary_expression			{$$=cminus.addQuad(OP_MUL,$1,$3);}
	| term '/' unary_expression			{$$=cminus.addQuad(OP_DIV,$1,$3);}
	| term '%' unary_expression			{$$=cminus.addQuad(OP_MOD,$1,$3);}
	| unary_expression					{$$=$1;}
	;

unary_op
	: '+'							{$$='+';}
	| '-'							{$$='-';}
	| '!'							{$$='!';}
	;

unary_expression
	: factor						{$$=$1;}
	| INC_OP unary_expression		{$$=cminus.addPreInc($2);}
	| DEC_OP unary_expression		{$$=cminus.addPreDec($2);}
	| unary_op unary_expression		{$$=cminus.addUnary($1,$2);}
	;

args
	:							{$$=new ArgsInfo;}
	| arg_list					{$$=new ArgsInfo(Arg::arg_cnt,Arg::first,$1);}
	;

arg_list
	: expression				{Arg::arg_cnt=0;$$=new Arg($1);Arg::first=$$;}
	| arg_list ',' expression	{$$=new Arg($3,$1);$1->next=$$;}
	;

factor
	: '(' expression ')'		{$$=$2;}
	| ID						{$$=cminus.getSymOperand($1);}
	| NUM						{$$.type=OPERAND_NUM;$$.num=$1;$$.lvalue=false;}
	| CHARACTER					{$$.type=OPERAND_CHA;$$.num=$1;$$.lvalue=false;}
	| ID '[' expression ']'		{$$=cminus.addArrayRef($1,$3);}
	| ID '(' args ')'			{$$=cminus.addCall($1,$3);}
	;