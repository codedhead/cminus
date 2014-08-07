#ifndef _YANJUN_ENV_H_
#define _YANJUN_ENV_H_

#include "_tokens.h"
#include "symtable.h"
#include "quadruple.h"
#include <hash_map>
#include <string>
#include <fstream>
using namespace std;
using namespace stdext;

enum _CBlock_Type{INNER_BLOCK,FUNC_BLOCK,MAIN_BLOCK};

class printable_list
{
public:
	void push_back(Quadruple& x);
	list<Quadruple> qs;
};

typedef list<Quadruple>::iterator quad_iterator;

typedef hash_map<string,SymbolInfo*>::iterator symbolinfo_iterator;

class Env
{
	friend class CMinus;
	hash_map<string,SymbolInfo*> symbol_table;
	
	Env* parent;

	int tmpcnt;

	printable_list quadruples;
	
	int block_type;
	SymbolInfo* func_info;

	int inner_need;

	int offset;

public:
	

	Env(Env* p);

	~Env();

	void clear();

	void globalClear();

	SymbolInfo* temp();

	bool put(char* s,SymbolInfo* sym);

	SymbolInfo* get(char* s);

	Quadruple* getLastQuad();
};

#endif