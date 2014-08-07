#include "env.h"
#include "_tokens.h"
#include "symtable.h"
#include "quadruple.h"
#include <hash_map>
#include <string>
#include <fstream>
using namespace std;
using namespace stdext;

#include "cminus.h"
extern CMinus cminus;


//#define TEMP_VARIABLE_NAME "_YaNjUn_%d"
#define TEMP_VARIABLE_NAME "___temp"


void printable_list::push_back(Quadruple& x)
{
	qs.push_back(x);
	cminus.writeQuad(x);
}

typedef list<Quadruple>::iterator quad_iterator;
typedef hash_map<string,SymbolInfo*>::iterator symbolinfo_iterator;


Env::Env(Env* p):parent(p),tmpcnt(0),block_type(INNER_BLOCK),inner_need(0){if(p)offset=p->offset;else offset=0;}

Env::~Env(){if(parent)clear();else globalClear();}

void Env::clear()
{
	for(symbolinfo_iterator it=symbol_table.begin();it!=symbol_table.end();++it)
	{			
		if(!it->second->long_life_time)
			delete it->second;
	}

}

void Env::globalClear()
{
	for(symbolinfo_iterator it=symbol_table.begin();it!=symbol_table.end();++it)
	{			
		if(it->second->kind==KIND_FUNCTION&&it->second->params_info)
		{
			it->second->params_info->clear();
			delete it->second->params_info;
		}
		delete it->second;
	}

}

SymbolInfo* Env::temp()
{
	SymbolInfo* tmp=new SymbolInfo;
	sprintf(tmp->name,"%s%d",TEMP_VARIABLE_NAME,tmpcnt++);
	tmp->type=INT;//always int
	tmp->kind=KIND_VARIABLE;
	return tmp;
}	

bool Env::put(char* s,SymbolInfo* sym)
{
	if(symbol_table.find(string(s))!=symbol_table.end()) return false;

	symbol_table.insert(pair<string,SymbolInfo*>(string(s),sym));
	return true;
}

SymbolInfo* Env::get(char* s)
{
	symbolinfo_iterator it;	
	string key(s);
	for(Env* penv=this;penv!=NULL;penv=penv->parent)
	{
		it=penv->symbol_table.find(key);

		if(it!=penv->symbol_table.end())
			return (it->second);
	}

	return NULL;
}

Quadruple* Env::getLastQuad()
{
	for(Env* penv=this;penv!=NULL;penv=penv->parent)
	{
		if(!penv->quadruples.qs.empty()) return &penv->quadruples.qs.back();
	}

	return NULL;
}
