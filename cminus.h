#ifndef _YANJUN_CMINUS_H_
#define _YANJUN_CMINUS_H_

#include "symtable.h"
#include "env.h"
#include <sstream>
#include <fstream>
using namespace std;

class CMinus
{
	Env global_symbol_env;
	Env* top_env;
	ofstream dataseg,codeseg,intermediate;

	int labelcnt;
	bool failed;

	char asm_file_name[256];

	void init();
	void cminuserror(char* s);
	void cminuserror(Operand& opr,char* s);
	void cminuserror(int op,char* s);
	void cminuserror(char* opr,char* s);
	void cminuserror(SymbolInfo* syminfo,char* s);
	void _generateASM(Env* env);
public:
	CMinus();
	CMinus(const char* filename);
	void setFileName(const char* filename);
	~CMinus();

	void generateASM();
	
//*************************intermediate quads*******************************//
	Operand addPreInc(Operand& opr);
	Operand addPreDec(Operand& opr);
	Operand addUnary(int uop,Operand& opr);
	Operand addConditional(Operand& cond,Operand& opr1,Operand& opr2);
	void addASMBlock(char* asmblock);
	Operand addQuad(int op,Operand& arg1,Operand& arg2);
	void addReturn(SymbolInfo* func);
	void addReturn(SymbolInfo* func,Operand& op);
	void addParam(Operand& param);
	Operand addArrayRef(char* str,Operand& index);
	Operand addCall(char* str,ArgsInfo* args);
	void placelabel(int lb);
	int nextlabel();
	Operand newTemp();
	void addJmp(int lb);
	void addJz(Operand& op,int lb);
	void addJnz(Operand& op,int lb);
	SymbolInfo* getSymbol(char* name);
	Operand getSymOperand(char* name);

	void writeQuad(Quadruple& q);
//*************************~intermediate quads******************************//	
//****************************generation************************************//
	string genSymbol(SymbolInfo* syminfo,bool dwordshow=false);
	void genArrayRef(Operand& opr);
	void twoop(char* op,int reg1,int reg2);
	void oneop(char* op,int reg);
	void oneop(char* op,char* reg);
	void push(Operand& opr);
	void mov2reg(int reg,Operand& opr);
	void mov2mem(Operand& opr,int reg);
	void self(char* op,Operand& opr);
	void not(Operand& res,Operand& opr);
	void generate(quad_iterator qit);
//****************************~generation***********************************//
	void afterDeclaration();
	void enterScope();
	void leaveScope();
	void enterFunc(SymbolInfo* funcinfo);
	void leaveFunc(SymbolInfo* funcinfo);
	void declare(SymbolInfo* syminfo);
	void declareInSegment(SymbolInfo* syminfo);
	void declareInStack(SymbolInfo* syminfo);
};

#endif