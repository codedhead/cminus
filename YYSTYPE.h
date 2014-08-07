#ifndef _YYSTYPE_H_
#define _YYSTYPE_H_
#include "symtable.h"
#include "quadruple.h"
#include "climits.h"
union YYSTYPE{
	int ival;
	char str[C_INDENTIFIER_LENGTH];
	char asmblock[C_ASMBLOCK_LENGTH];
	SymbolInfo* sym_info;
	ParamsInfo* params_info;
	Param* param;
	Operand operand;
	WhileLabel while_label;
	ForLabel for_label;
	DoLabel do_label;
	ArgsInfo* args_info;
	Arg* arg;
};
#endif