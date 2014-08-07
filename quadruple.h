#ifndef _YANJUN_TRIPLE_H_
#define _YANJUN_TRIPLE_H_

#include "symtable.h"

enum _OperandType{OPERAND_CHA,OPERAND_NUM,OPERAND_SYM,OPERAND_ARR,OPERAND_VOID/*when function return void*/};
enum _OperatorType{OP_ADD,OP_SUB,OP_MUL,OP_DIV,OP_MOD,OP_L,OP_G,OP_LE,OP_GE,OP_EQ,OP_NE,OP_ASS,OP_LAND,OP_LOR,OP_NOT,OP_INC,OP_DEC,OP_NEG,OP_PARAM,OP_CALL,OP_LABEL,OP_JMP,OP_JZ,OP_JNZ,OP_RET,OP_EAX,FAKEOP_ENTERSCOPE,OP_ASM};
extern char _operators[][10];

struct Operand
{
	int type;
	union{
		int num;
		SymbolInfo* syminfo;
	};
	bool lvalue;
	int index_offset;
};

struct Arg
{
	static int arg_cnt;
	static Arg* first;

	Arg(Operand op):opr(op),prev(0),next(0){++arg_cnt;}
	Arg(Operand op,Arg* p):opr(op),prev(p),next(0){++arg_cnt;}
	Arg(Operand op,Arg* p,Arg* n):opr(op),prev(p),next(n){++arg_cnt;}
	Operand opr;
	Arg* prev;
	Arg* next;
};

struct ArgsInfo
{
	ArgsInfo(int ac,Arg* f,Arg* l):arg_cnt(ac),first(f),last(l){}
	ArgsInfo():arg_cnt(0),first(0),last(0){}
	int arg_cnt;
	Arg* last;
	Arg* first;
	void clear()
	{
		Arg* p,*q;
		for(p=last;p;p=q)
		{
			q=p->prev;
			delete p;
		}
		last=0;
	}
};

class Env;

struct Quadruple
{
	Quadruple(int o,char* as):op(o),asmblock(as){}
	Quadruple(int o,Env* ev):op(o),scope(ev){}
	Quadruple(int o,Operand& p):op(o),op1(p){}
	Quadruple(int o,Operand& p1,Operand& p2):op(o),op1(p1),op2(p2){}
	Quadruple(Operand& r,int o,Operand& p1):res(r),op(o),op1(p1){}
	Quadruple(Operand& r,int o,Operand& p1,Operand& p2):res(r),op(o),op1(p1),op2(p2){}
	union
	{
		Operand res;
		Env* scope;
		char* asmblock;
	};
	int op;
	Operand op1;
	Operand op2;
};

#endif