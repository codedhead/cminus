#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include "climits.h"
#include <string.h>

enum _SymbolKind{KIND_VARIABLE,KIND_ARRAY,KIND_FUNCTION};

struct ParamsInfo;
struct SymbolInfo
{
	SymbolInfo(){memset(this,0,sizeof(SymbolInfo));}
	char name[C_INDENTIFIER_LENGTH];
	int type;
	int kind;//var,arr,func
	struct  
	{
		bool global;
		int offset;
	}address;

	bool long_life_time;
	bool pointer;
	union {
		int arr_size;
		int arr_offset;
		ParamsInfo* params_info;
	};
};

struct Param
{
	static int param_cnt;
	Param(SymbolInfo* sym):syminfo(sym),next(0){++param_cnt;}
	Param(SymbolInfo* sym,Param* p):syminfo(sym),next(p){++param_cnt;}
	SymbolInfo* syminfo;
	Param* next;
};


struct ParamsInfo
{
	ParamsInfo(int pc,Param* p):param_cnt(pc),first(p){}
	ParamsInfo():param_cnt(0),first(0){}
	int param_cnt;
	Param* first;
	void clear()
	{
		Param* p,*q;
		for(p=first;p;p=q)
		{
			q=p->next;
			delete p;
		}
		first=0;
	}
};

struct WhileLabel
{
	int start;
	int end;
};
struct ForLabel
{
	int condition;
	int increment;
	int body;
	int end;
};
struct DoLabel
{
	int start;
	int condition;
	int end;
};

#endif