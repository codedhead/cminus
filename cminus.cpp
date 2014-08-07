#include "cminus.h"
#include "symtable.h"
#include "env.h"
#include "_lex.h"
#include <sstream>
#include <fstream>
using namespace std;

enum _REGS{EAX,EBX,ECX,EDX,ESP,EBP};

extern Lex lex;

char _relops[][10]={"setl","setg","setle","setge","sete","setne"};
char _regs[][10]={"eax","ebx","ecx","edx","esp","ebp"};
char _byteregs[][10]={"al","bl","cl","dl"};	

#define START_LABEL_NAME "start"
#define MAIN_FUNC_NAME "main"
#define PROGRAM_END_LABEL "_YaNjUn_ProgramEndLabel"
#define OPEN_CONSOLE_FUNC "_YaNjUn_OpenConsole"


#define ALLOC_SIZE(tp) (tp==INT?4:(tp==CHAR?1:0))
#define PARAM_ALLOC_SIZE(tp) (4)
//#define TYPE_SIZE(tp) (tp==INT?4:(tp==CHAR?1:0))
#define TYPE_COMPATIBLE(t1,t2) (t1==t2||(t1==INT&&t2==CHAR)||(t2==INT&&t1==CHAR))

void CMinus::cminuserror(char* s)
{
	printf("ERROR {near line %d} %s\n",lex.linecnt,s);
	failed=true;
}
void CMinus::cminuserror(Operand& opr,char* s)
{
	printf("ERROR {near line %d, ",lex.linecnt);
	switch(opr.type)
	{
	case OPERAND_NUM:
		printf("\'%d\'",opr.num);
		break;
	case OPERAND_CHA:
		printf("\'%c\'",opr.num);
		break;
	case OPERAND_SYM:
	case OPERAND_ARR:
		printf("\'%s\'",opr.syminfo->name);
		break;
	}
	printf("} %s\n",s);
}
void CMinus::cminuserror(int op,char* s)
{
	printf("ERROR {near line %d, %s} %s\n",lex.linecnt,_operators[op],s);	
}
void CMinus::cminuserror(char* opr,char* s)
{
	printf("ERROR {near line %d, %s} %s\n",lex.linecnt,opr,s);	
}
void CMinus::cminuserror(SymbolInfo* syminfo,char* s)
{
	printf("ERROR {near line %d, %s} %s\n",lex.linecnt,syminfo->name,s);	
}

CMinus::CMinus(const char* filename):global_symbol_env(NULL),labelcnt(-1),failed(false)
{
	strcpy(asm_file_name,filename);
	init();	
}
CMinus::CMinus():global_symbol_env(NULL),labelcnt(-1),failed(false)
{
	strcpy(asm_file_name,"cminus.asm");
	init();	
}
void CMinus::setFileName(const char* filename)
{
	strcpy(asm_file_name,filename);
}
void CMinus::init()
{
	top_env=&global_symbol_env;
	dataseg.open("dataseg");
	dataseg<<".data?\n";
	dataseg<<"	_YaNjUn_hStdout dd ?\n";
	dataseg<<"	_YaNjUn_hStdin dd ?\n";

	codeseg.open("codeseg");
	codeseg<<".code\n";
	codeseg<<OPEN_CONSOLE_FUNC<<" proc\n";
	codeseg<<"	mov	_YaNjUn_hStdout,0\n";
	codeseg<<"	mov	_YaNjUn_hStdin,0\n";
	codeseg<<"	invoke GetStdHandle,STD_OUTPUT_HANDLE\n";
	codeseg<<"	cmp eax,INVALID_HANDLE_VALUE\n";
	codeseg<<"	je _OPN_CONSOLE_END\n";
	codeseg<<"	mov _YaNjUn_hStdout,eax\n";
	codeseg<<"	invoke GetStdHandle,STD_INPUT_HANDLE\n";
	codeseg<<"	cmp eax,INVALID_HANDLE_VALUE\n";
	codeseg<<"	je _OPN_CONSOLE_END\n";
	codeseg<<"	mov _YaNjUn_hStdin,eax\n";
	codeseg<<"	mov eax,1-INVALID_HANDLE_VALUE\n";
	codeseg<<"_OPN_CONSOLE_END:\n";
	codeseg<<"	ret\n";
	codeseg<<OPEN_CONSOLE_FUNC<<" endp\n";

	intermediate.open("intermediate code.txt");
}

CMinus::~CMinus()
{
	dataseg.close();
	codeseg.close();
	intermediate.close();
	remove("codeseg");remove("dataseg");
}

void CMinus::generateASM()
{
	if(failed) return;

	_generateASM(&global_symbol_env);

	dataseg.close();
	codeseg.close();
	intermediate.close();

	ifstream idata("dataseg"),icode("codeseg");

	ofstream asmfile(asm_file_name);

	asmfile<<".386\n";
	asmfile<<".model flat, stdcall\n";
	asmfile<<"option casemap :none\n";
	asmfile<<"include inc/console.inc\n";
	asmfile<<"includelib inc/kernel32.lib\n";

	string buffer;
	if(idata.is_open())
	{
		while(!idata.eof())
		{
			getline(idata,buffer);
			asmfile<<buffer<<endl;
		}		
	}
	if(icode.is_open())
	{
		while(!icode.eof())
		{
			getline(icode,buffer);
			asmfile<<buffer<<endl;
		}		
	}
	idata.close();icode.close();asmfile.close();
	
}

void CMinus::_generateASM(Env* env)
{
	int size;
	if(env->block_type==MAIN_BLOCK)
	{			
		codeseg<<START_LABEL_NAME<<":\n";
		//codeseg<<"	push ebp\n";
		codeseg<<"	mov ebp,esp\n";

		size=max(-env->offset,env->inner_need);
		size=(((size+3)>>2)<<2);
		if(size)codeseg<<"	sub esp, "<<size<<endl;

		codeseg<<"	call "<<OPEN_CONSOLE_FUNC<<endl;
		codeseg<<"	cmp eax, INVALID_HANDLE_VALUE\n";
		codeseg<<"	jz "<<PROGRAM_END_LABEL<<endl;
	}
	else if(env->block_type==FUNC_BLOCK)
	{
		codeseg<<env->func_info->name<<" proc\n";
		codeseg<<"	push ebp\n";
		codeseg<<"	mov ebp,esp\n";

		size=max(-env->offset,env->inner_need);
		size=(((size+3)>>2)<<2);
		if(size)codeseg<<"	sub esp, "<<size<<endl;
	}

	for(quad_iterator qit=env->quadruples.qs.begin();qit!=env->quadruples.qs.end();++qit)
	{
		if(qit->op==FAKEOP_ENTERSCOPE)
			_generateASM(qit->scope);
		else generate(qit);
	}		

	if(env->block_type==MAIN_BLOCK)
	{	
		codeseg<<PROGRAM_END_LABEL<<":\n";
		codeseg<<"	invoke ExitProcess,NULL\n";
		codeseg<<"end "<<START_LABEL_NAME<<endl;
	}
	else if(env->block_type==FUNC_BLOCK)
	{
		//codeseg<<"	mov esp,ebp\n";
		//codeseg<<"	pop ebp\n";
		codeseg<<"	leave\n";
		codeseg<<"	ret\n";
		codeseg<<env->func_info->name<<" endp\n";
	}

	if(env!=&global_symbol_env) delete env;
}


//*************************intermediate quads*******************************//	

#define MAKE_A_VOID_OPERAND(op) op.lvalue=false;op.type=OPERAND_VOID;
#define MAKE_A_NONE_OPERAND(op) op.lvalue=false;op.type=OPERAND_NUM;op.num=0;
#define MAKE_A_RVALUE_NUM(op,n) op.lvalue=false;op.type=OPERAND_NUM;op.num=n;
#define MAKE_A_RVALUE_SYM(op,info) op.lvalue=false;op.type=OPERAND_SYM;op.syminfo=info;
#define MAKE_A_LVALUE_SYM(op,info) op.lvalue=true;op.type=OPERAND_SYM;op.syminfo=info;

Operand CMinus::addPreInc(Operand& opr)
{
	if(opr.lvalue)
	{			
		top_env->quadruples.push_back(Quadruple(OP_INC,opr));
		return opr;
	}
	else
	{
		cminuserror(opr,"expect a l-value.");
		Operand none;MAKE_A_NONE_OPERAND(none);
		return none;
	}
}
Operand CMinus::addPreDec(Operand& opr)
{
	if(opr.lvalue)
	{			
		top_env->quadruples.push_back(Quadruple(OP_DEC,opr));
		return opr;
	}
	else
	{
		cminuserror(opr,"expect a l-value.");
		Operand none;MAKE_A_NONE_OPERAND(none);
		return none;
	}
}
Operand CMinus::addUnary(int uop,Operand& opr)
{
	switch(uop)
	{
	case '+':return opr;
	case '-':
		{
			Operand tmp=newTemp();
			top_env->quadruples.push_back(Quadruple(tmp,OP_NEG,opr));
			return tmp;
		}
	case '!':
		{
			Operand tmp=newTemp();
			top_env->quadruples.push_back(Quadruple(tmp,OP_NOT,opr));
			return tmp;
		}
	}		
	Operand noneopr;
	MAKE_A_NONE_OPERAND(noneopr);
	return noneopr;
}
Operand CMinus::addConditional(Operand& cond,Operand& opr1,Operand& opr2)
{
	int lb1=nextlabel(),lb2=nextlabel();
	Operand tmp=newTemp();
	addJz(cond,lb1);
	top_env->quadruples.push_back(Quadruple(tmp,OP_ASS,opr1));
	addJmp(lb2);
	placelabel(lb1);
	top_env->quadruples.push_back(Quadruple(tmp,OP_ASS,opr2));
	placelabel(lb2);
	return tmp;
}
void CMinus::addASMBlock(char* asmblock)
{
	char* newasm=new char[strlen(asmblock)+2];
	strcpy(newasm,asmblock);
	top_env->quadruples.push_back(Quadruple(OP_ASM,newasm));
}
Operand CMinus::addQuad(int op,Operand& arg1,Operand& arg2)
{
	if(arg1.type==OPERAND_VOID)
	{
		cminuserror(arg1,"function has no return value.");
		Operand none;MAKE_A_NONE_OPERAND(none);
		return none;
	}
	else if(arg2.type==OPERAND_VOID)
	{
		cminuserror(arg2,"function has no return value.");
		Operand none;MAKE_A_NONE_OPERAND(none);
		return none;
	}
	else if(arg1.type==OPERAND_SYM&&arg1.syminfo->pointer)
	{
		if((arg2.type==OPERAND_NUM||(arg2.type==OPERAND_SYM&&arg2.syminfo->type==INT&&arg2.syminfo->kind==KIND_VARIABLE))&&(op==OP_ADD||op==OP_SUB))
		{
			Operand eleSize=newTemp();MAKE_A_RVALUE_NUM(eleSize,ALLOC_SIZE(arg1.syminfo->type));				

			Operand tmp=newTemp();tmp.syminfo->kind=KIND_ARRAY;tmp.syminfo->pointer=true;
			top_env->quadruples.push_back(Quadruple(tmp,op,arg1,addQuad(OP_MUL,eleSize,arg2)));
			return tmp;
		}
		else
		{	
			cminuserror(op,"can't perform such arithmetic operation on array variable.");			
			Operand none;MAKE_A_NONE_OPERAND(none);
			return none;
		}
	}
	else if(arg2.type==OPERAND_SYM&&arg2.syminfo->pointer)
	{
		if((arg1.type==OPERAND_NUM||(arg1.type==OPERAND_SYM&&arg1.syminfo->type==INT&&arg1.syminfo->kind==KIND_VARIABLE))&&op==OP_ADD)
		{
			Operand eleSize=newTemp();MAKE_A_RVALUE_NUM(eleSize,ALLOC_SIZE(arg1.syminfo->type));

			Operand tmp=newTemp();tmp.syminfo->kind=KIND_ARRAY;tmp.syminfo->pointer=true;
			top_env->quadruples.push_back(Quadruple(tmp,op,addQuad(OP_MUL,eleSize,arg1),arg2));
			return tmp;
		}
		else
		{	
			cminuserror(op,"can't perform such arithmetic operation on array variable.");			
			Operand none;MAKE_A_NONE_OPERAND(none);
			return none;
		}
	}


	if(op==OP_ASS)
	{
		if(!arg1.lvalue)//.type==OPERAND_NUM)
		{
			cminuserror(arg1,"expect a l-value.");				
		}
		else top_env->quadruples.push_back(Quadruple(arg1,op,arg2));
		return arg1;
	}
	else
	{
		Operand tmp=newTemp();
		top_env->quadruples.push_back(Quadruple(tmp,op,arg1,arg2));
		return tmp;
	}		
}

void CMinus::addReturn(SymbolInfo* func)
{
	if(func->type!=VOID)
	{
		cminuserror("function has return value.");
	}
	else
	{
		Operand vo;MAKE_A_VOID_OPERAND(vo);
		top_env->quadruples.push_back(Quadruple(OP_RET,vo));
	}		
}
void CMinus::addReturn(SymbolInfo* func,Operand& op)
{
	if(func->type==VOID)
	{
		cminuserror("function has no return value.");
	}
	else if(op.type==OPERAND_NUM||op.type==OPERAND_CHA||
		TYPE_COMPATIBLE(op.syminfo->type,func->type))
	{
		top_env->quadruples.push_back(Quadruple(OP_RET,op));
	}
	else
	{
		cminuserror(op,"invalid return value");
	}	
}
void CMinus::addParam(Operand& param)
{
	//参数要匹配，不知是个数，数组传地址
	if(param.type==OPERAND_VOID)
		cminuserror(param,"function has no return value.");
	else
		top_env->quadruples.push_back(Quadruple(OP_PARAM,param));
}

Operand CMinus::addArrayRef(char* str,Operand& index)
{
	Operand arr;arr.syminfo=getSymbol(str);
	if(!arr.syminfo||arr.syminfo->kind!=KIND_ARRAY)
	{
		cminuserror(str,"not array variable or identifier not found.");
		MAKE_A_NONE_OPERAND(arr);
		return arr;
	}
	else
	{
		if(index.type==OPERAND_VOID)
		{
			cminuserror(str,"function has no return value.");
			MAKE_A_NONE_OPERAND(arr);
		}
		else
		{
			Operand arr_index=newTemp(),num;
			MAKE_A_RVALUE_NUM(num,ALLOC_SIZE(arr.syminfo->type));
			top_env->quadruples.push_back(Quadruple(arr_index,OP_MUL,num,index));

			arr.lvalue=true;arr.type=OPERAND_ARR;
			arr.index_offset=arr_index.syminfo->address.offset;
			return arr;
		}
	}				
}

Operand CMinus::addCall(char* str,ArgsInfo* args)
{
	Operand func;func.type=OPERAND_SYM;func.syminfo=getSymbol(str);

	if(!func.syminfo||func.syminfo->kind!=KIND_FUNCTION||func.syminfo->params_info->param_cnt!=args->arg_cnt)
	{
		cminuserror(str,"not function; or identifier not found; or argument count not match.");

		args->clear();
		delete args;

		if(func.syminfo->type==VOID)
		{
			func.type=OPERAND_VOID;
			return func;
		}
		else
		{
			MAKE_A_NONE_OPERAND(func);
			return func;
		}			
	}
	else
	{
		Param* p=func.syminfo->params_info->first;
		Arg* q=args->first;
		for(;p;p=p->next,q=q->next)
		{
			if(q->opr.type==OPERAND_NUM||q->opr.type==OPERAND_CHA)
			{
				if(p->syminfo->kind==KIND_VARIABLE&&(p->syminfo->type==INT||p->syminfo->type==CHAR))
					continue;
				else cminuserror(q->opr,"argument type and param type not compatible.");
			}
			else if(!TYPE_COMPATIBLE(q->opr.syminfo->type,p->syminfo->type))
				cminuserror(q->opr,"argument type and param type not compatible.");
			else if(q->opr.type==OPERAND_ARR&&p->syminfo->kind!=KIND_VARIABLE)
				cminuserror(q->opr,"argument type and param type not compatible.");
			else if(q->opr.type==OPERAND_SYM&&q->opr.syminfo->kind!=p->syminfo->kind)
				cminuserror(q->opr,"argument type and param type not compatible.");
		}

		if(!failed)
		{
			for(q=args->last;q;q=q->prev)
				addParam(q->opr);//,parm_desc(type,varorarr));

			Operand arg;MAKE_A_RVALUE_NUM(arg,(args->arg_cnt<<2));
			top_env->quadruples.push_back(Quadruple(OP_CALL,func,arg));	
		}


		args->clear();
		delete args;					

		if(func.syminfo->type==VOID)
		{
			func.type=OPERAND_VOID;
			return func;
		}
		else
		{
			Operand tmp=newTemp();
			top_env->quadruples.push_back(Quadruple(OP_EAX,tmp));
			return tmp;//store res value
		}	
	}		
}

void CMinus::placelabel(int lb)
{
	// 		Quadruple* qd=top_env->getLastQuad();
	// 		if(qd&&qd->op==OP_LABEL) return;
	// 		else
	{
		Operand _label;MAKE_A_RVALUE_NUM(_label,lb);
		top_env->quadruples.push_back(Quadruple(OP_LABEL,_label));
	}		
}

int CMinus::nextlabel()
{
	// 		Quadruple* qd=top_env->getLastQuad();
	// 		if(qd&&qd->op==OP_LABEL) return labelcnt;
	return ++labelcnt;
}

Operand CMinus::newTemp()
{
	SymbolInfo* info=top_env->temp();
	declareInStack(info);
	Operand res;MAKE_A_RVALUE_SYM(res,info);
	return res;
}

void CMinus::addJmp(int lb)
{
	Operand _label;MAKE_A_RVALUE_NUM(_label,lb);
	top_env->quadruples.push_back(Quadruple(OP_JMP,_label));
}

void CMinus::addJz(Operand& op,int lb)
{
	if(op.type==OPERAND_VOID)
		cminuserror(op,"function has no return value.");
	else
	{
		Operand _label;MAKE_A_RVALUE_NUM(_label,lb);
		top_env->quadruples.push_back(Quadruple(OP_JZ,op,_label));
	}	
}
void CMinus::addJnz(Operand& op,int lb)
{
	if(op.type==OPERAND_VOID)
		cminuserror(op,"function has no return value.");
	else
	{
		Operand _label;MAKE_A_RVALUE_NUM(_label,lb);
		top_env->quadruples.push_back(Quadruple(OP_JNZ,op,_label));
	}	
}

SymbolInfo* CMinus::getSymbol(char* name)
{
	return top_env->get(name);
	//if(!syminfo) {cminuserror("symbol not found.");return;}
}

Operand CMinus::getSymOperand(char* name)
{
	SymbolInfo* info=top_env->get(name);
	Operand res;
	if(!info)
	{
		cminuserror(name,"identifier not found.");
	}
	else if(info->kind==KIND_FUNCTION)
	{
		cminuserror(name,"identifier is a function.");
	}
	else
	{MAKE_A_LVALUE_SYM(res,info);return res;}
	MAKE_A_NONE_OPERAND(res);
	return res;		
}

#define OUTPUT_Operand(x,op) if(op.type==OPERAND_NUM||op.type==OPERAND_CHA) x<<op.num;else x<<op.syminfo->name;

void CMinus::writeQuad(Quadruple& q)
{
	switch(q.op)
	{
	case FAKEOP_ENTERSCOPE:
	case OP_ASM:
		break;
	case OP_INC:
	case OP_DEC:		
		intermediate<<_operators[q.op]<<' ';
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<endl;	
		break;
	case OP_NEG:
	case OP_NOT:
		OUTPUT_Operand(intermediate,q.res);
		intermediate<<" = "<<_operators[q.op]<<' ';
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<endl;	
		break;
		break;
	case OP_RET:			
		intermediate<<"ret ";
		if(q.op1.type!=OPERAND_VOID)
			OUTPUT_Operand(intermediate,q.op1);
		intermediate<<endl;		
		break;
	case OP_ASS:
		OUTPUT_Operand(intermediate,q.res);
		intermediate<<" = ";
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<endl;
		break;
	case OP_PARAM:				
	case OP_CALL:
	case OP_JMP:
	case OP_LABEL:
		intermediate<<_operators[q.op]<<' ';
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<endl;
		break;
	case OP_JZ:
	case OP_JNZ:
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<' '<<_operators[q.op]<<' ';
		OUTPUT_Operand(intermediate,q.op2);
		intermediate<<endl;
		break;
	case OP_EAX:
		intermediate<<"mov ";
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<", eax"<<endl;
		break;
	default:
		OUTPUT_Operand(intermediate,q.res);
		intermediate<<" = ";
		OUTPUT_Operand(intermediate,q.op1);
		intermediate<<_operators[q.op];
		OUTPUT_Operand(intermediate,q.op2);
		intermediate<<endl;
	}

}

//*************************intermediate quads*******************************//	
//****************************generation************************************//
string CMinus::genSymbol(SymbolInfo* syminfo,bool dwordshow)
{		
	stringstream res;
	if(syminfo->address.global)
	{
		if(syminfo->type==INT)
			res<<string("dword ptr ")<<string(syminfo->name);
		else if(syminfo->type==CHAR)
			res<<string("byte ptr ")<<string(syminfo->name);
	}
	else
	{
		if(syminfo->type==CHAR)
			res<<string("byte ptr ");
		else if(syminfo->type==INT&&dwordshow) 
			res<<string("dword ptr ");
		res<<'('<<syminfo->address.offset<<string(")[ebp]");
	}

	return res.str();
}
void CMinus::genArrayRef(Operand& opr)
{
	//mov ecx lea	
	if(opr.syminfo->pointer)
	{
		codeseg<<"	mov ecx, ("<<opr.syminfo->address.offset<<")[ebp]\n";
		codeseg<<"	add ecx, ("<<opr.index_offset<<")[ebp]\n";
	}
	else
	{
		codeseg<<"	lea ecx, "<<genSymbol(opr.syminfo)<<endl;
		codeseg<<"	add ecx, ("<<opr.index_offset<<")[ebp]\n";
	}


	// 		if(opr.syminfo->type==CHAR)
	// 			return "byte ptr [ecx]";		
	// 		else if(opr.syminfo->type==CHAR)
	// 			return "dword ptr [ecx]";
}

void CMinus::twoop(char* op,int reg1,int reg2)
{
	codeseg<<"	"<<op<<" "<<_regs[reg1]<<", "<<_regs[reg2]<<endl;		
}

void CMinus::oneop(char* op,int reg)
{
	codeseg<<"	"<<op<<" "<<_regs[reg]<<endl;		
}

void CMinus::oneop(char* op,char* reg)
{
	codeseg<<"	"<<op<<" "<<reg<<endl;		
}


void CMinus::push(Operand& opr)
{
	if(opr.type==OPERAND_NUM||opr.type==OPERAND_CHA)
		codeseg<<"	push "<<opr.num<<endl;
	else if(opr.type==OPERAND_ARR)
	{
		genArrayRef(opr);

		if(opr.syminfo->type==CHAR)
		{
			// 				codeseg<<"	xor eax, eax\n";
			// 				codeseg<<"	mov al, "<<ar<<endl;
			codeseg<<"	movsx eax, byte ptr [ecx]\n";
			codeseg<<"	push eax\n";
		}
		else if(opr.syminfo->type==INT)
			codeseg<<"	push [ecx]\n";
	}
	else if(opr.type==OPERAND_SYM)
	{
		if(opr.syminfo->kind==KIND_VARIABLE)
		{
			if(opr.syminfo->type==CHAR)
			{
				// 					codeseg<<"	xor eax, eax\n";
				// 					codeseg<<"	mov al, "<<genSymbol(opr.syminfo)<<endl;
				codeseg<<"	movsx eax, "<<genSymbol(opr.syminfo)<<endl;
				codeseg<<"	push eax\n";
			}
			else if(opr.syminfo->type==INT)
				codeseg<<"	push "<<genSymbol(opr.syminfo)<<endl;
		}
		else if(opr.syminfo->kind==KIND_ARRAY)
		{
			if(opr.syminfo->pointer)
			{
				//pointer 一定来自形参
				codeseg<<"	mov eax, ("<<opr.syminfo->address.offset<<")[ebp]\n";
			}
			else
				codeseg<<"	lea eax, "<<genSymbol(opr.syminfo)<<endl;
			codeseg<<"	push eax\n";
		}
	}
}

void CMinus::mov2reg(int reg,Operand& opr)
{
	if(opr.type==OPERAND_NUM||opr.type==OPERAND_CHA)
		codeseg<<"	mov "<<_regs[reg]<<", "<<opr.num<<endl;
	else if(opr.type==OPERAND_SYM)
	{
		if(opr.syminfo->type==CHAR)
			// 			{
			// 				codeseg<<"	xor "<<_regs[reg]<<", "<<_regs[reg]<<endl;
			// 				codeseg<<"	mov "<<_byteregs[reg]<<", byte ptr ";
			// 			}
			codeseg<<"	movsx "<<_regs[reg]<<", "<<genSymbol(opr.syminfo)<<endl;
		else if(opr.syminfo->type==INT)
			codeseg<<"	mov "<<_regs[reg]<<", "<<genSymbol(opr.syminfo)<<endl;	
	}
	else if(opr.type==OPERAND_ARR)
	{
		genArrayRef(opr);

		if(opr.syminfo->type==CHAR)
			// 			{
			// 				codeseg<<"	xor "<<_regs[reg]<<", "<<_regs[reg]<<endl;
			// 				codeseg<<"	mov "<<_byteregs[reg]<<", byte ptr [ecx]";
			// 			}
			codeseg<<"	movsx "<<_regs[reg]<<", byte ptr [ecx]\n";
		else if(opr.syminfo->type==INT)
			codeseg<<"	mov "<<_regs[reg]<<", [ecx]\n";
	}
}
void CMinus::mov2mem(Operand& opr,int reg)
{
	if(opr.type==OPERAND_NUM||opr.type==OPERAND_CHA)
		;//error
	else if(opr.type==OPERAND_SYM)
	{
		if(opr.syminfo->type==CHAR)
			codeseg<<"	mov "<<genSymbol(opr.syminfo)<<", "<<_byteregs[reg]<<endl;				
		else if(opr.syminfo->type==INT)
			codeseg<<"	mov "<<genSymbol(opr.syminfo)<<", "<<_regs[reg]<<endl;				
	}
	else if(opr.type==OPERAND_ARR)
	{
		genArrayRef(opr);

		if(opr.syminfo->type==CHAR)
			codeseg<<"	mov byte ptr [ecx], "<<_byteregs[reg]<<endl;				
		else if(opr.syminfo->type==INT)
			codeseg<<"	mov [ecx], "<<_regs[reg]<<endl;
	}
}
void CMinus::self(char* op,Operand& opr)
{
	if(opr.type==OPERAND_ARR)
	{
		genArrayRef(opr);
		codeseg<<"	"<<op<<' ';
		if(opr.syminfo->type==CHAR)
			codeseg<<"byte ptr [ecx]\n";
		else if(opr.syminfo->type==INT)
			codeseg<<"dword ptr [ecx]\n";
	}
	else if(opr.type==OPERAND_SYM)
	{
		codeseg<<"	"<<op<<' '<<genSymbol(opr.syminfo,true)<<endl;
	}
}
void CMinus::not(Operand& res,Operand& opr)
{		
	if(opr.type==OPERAND_NUM||opr.type==OPERAND_CHA)
	{
		if(opr.num) codeseg<<"	mov "<<genSymbol(res.syminfo,true)<<", 0\n";
		else codeseg<<"	mov "<<genSymbol(res.syminfo,true)<<", 1\n";
	}
	else
	{
		codeseg<<"	xor eax, eax\n";
		if(opr.type==OPERAND_ARR)
		{
			genArrayRef(opr);
			if(opr.syminfo->type==INT)
				codeseg<<"	cmp dword ptr [ecx], 0\n";
			else if(opr.syminfo->type==CHAR)
				codeseg<<"	cmp byte ptr [ecx], 0\n";
		}
		else if(opr.type==OPERAND_SYM)
			codeseg<<"	cmp "<<genSymbol(opr.syminfo,true)<<", 0\n";

		//res is a int temp			
		codeseg<<"	sete al\n";
		codeseg<<"	mov "<<genSymbol(res.syminfo)<<", eax\n";
	}		
}
void CMinus::generate(quad_iterator qit)
{
	Operand helpnum;helpnum.type=OPERAND_NUM;
	//for(quad_iterator qit=top_env->quadruples.qs.begin();qit!=top_env->quadruples.qs.end();++qit)
	{
		switch(qit->op)
		{
		case FAKEOP_ENTERSCOPE:
			break;
		case OP_INC:
			self("inc",qit->op1);
			break;
		case OP_DEC:
			self("dec",qit->op1);
			break;
		case OP_NEG:
			mov2reg(EAX,qit->op1);
			oneop("neg",EAX);
			mov2mem(qit->res,EAX);
			break;
		case OP_NOT:				
			not(qit->res,qit->op1);
			break;
		case OP_ASM:
			codeseg<<qit->asmblock<<endl;
			delete [] qit->asmblock;
			break;
		case OP_RET:
			if(qit->op1.type!=OPERAND_VOID)
				mov2reg(EAX,qit->op1);
			codeseg<<"	leave\n";
			codeseg<<"	ret\n";
			break;
		case OP_ADD:
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			twoop("add",EAX,EBX);
			mov2mem(qit->res,EAX);
			break;
		case OP_SUB:
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			twoop("sub",EAX,EBX);
			mov2mem(qit->res,EAX);
			break;
		case OP_MUL:
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			oneop("mul",EBX);
			mov2mem(qit->res,EAX);
			break;
		case OP_DIV:
			twoop("xor",EDX,EDX);
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			oneop("div",EBX);
			mov2mem(qit->res,EAX);
			break;
		case OP_MOD:
			twoop("xor",EDX,EDX);
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			oneop("div",EBX);
			mov2mem(qit->res,EDX);
			break;
		case OP_L:
		case OP_G:
		case OP_LE:
		case OP_GE:
		case OP_EQ:
		case OP_NE:
			twoop("xor",EDX,EDX);
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			twoop("cmp",EAX,EBX);	
			oneop(_relops[qit->op-OP_L],"dl");
			mov2mem(qit->res,EDX);
			break;
		case OP_LAND:
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			twoop("and",EAX,EBX);
			mov2mem(qit->res,EAX);
			break;
		case OP_LOR:
			mov2reg(EAX,qit->op1);
			mov2reg(EBX,qit->op2);
			twoop("or",EAX,EBX);
			mov2mem(qit->res,EAX);
			break;
		case OP_ASS:
			mov2reg(EAX,qit->op1);
			mov2mem(qit->res,EAX);
			break;
		case OP_PARAM:
			//如果是数组，则传地址
			push(qit->op1);
			break;
		case OP_CALL:
			codeseg<<"	call "<<qit->op1.syminfo->name<<endl;
			codeseg<<"	add esp, "<<qit->op2.num<<endl;
			break;
		case OP_JMP:
			codeseg<<"	jmp ___label"<<qit->op1.num<<endl;
			break;
		case OP_LABEL:
			codeseg<<"___label"<<qit->op1.num<<":\n";
			break;
		case OP_JZ:
			mov2reg(EAX,qit->op1);
			codeseg<<"	cmp eax, 0\n";
			codeseg<<"	jz ___label"<<qit->op2.num<<endl;			
			break;
		case OP_EAX:
			mov2mem(qit->op1,EAX);
			break;
		}
	}

}


//****************************generation************************************//
void CMinus::afterDeclaration()
{
	//top_env->temp_vars_size=-top_env->offset;
}
void CMinus::enterScope()
{
	Env* last_env=top_env;
	top_env=new Env(last_env);

	last_env->quadruples.qs.push_back(Quadruple(FAKEOP_ENTERSCOPE,top_env));

	intermediate<<"{\n";
}

void CMinus::leaveScope()
{
	if(top_env)
	{
		//top_env->buildBlocks();
		//top_env->nextUse();
		// 			if(!failed) generate(); 			
		// 			delete last_env;

		//if(-top_env->offset>top_env->)

		Env* last_env=top_env;			
		top_env=top_env->parent;

		top_env->inner_need=max(top_env->inner_need,max(-last_env->offset,last_env->inner_need));

		//no delete

		intermediate<<"}\n";
	}
	else
		cminuserror("error '}' dont match.");		
}
void CMinus::enterFunc(SymbolInfo* funcinfo)
{		
	//需要记录临时变量需要多上空间
	if(global_symbol_env.put(funcinfo->name,funcinfo))
	{
		top_env->offset=0;
		top_env->put(funcinfo->name,funcinfo);
		funcinfo->long_life_time=true;

		//return address+push ebp=8
		int ofst=8;
		for(Param* param=funcinfo->params_info->first;param;param=param->next)
		{
			param->syminfo->long_life_time=true;
			param->syminfo->address.global=false;
			param->syminfo->address.offset=ofst;

			if(param->syminfo->type==VOID||param->syminfo->kind==KIND_FUNCTION||
				(param->syminfo->kind==KIND_ARRAY&&param->syminfo->arr_size!=-1))
			{
				cminuserror(param->syminfo,"param declaration error, param can't be void type or function type, and array type should not have size.");
				param->syminfo->type=VOID;
				param->syminfo->kind=KIND_VARIABLE;
			}

			if(param->syminfo->kind==KIND_ARRAY)
				param->syminfo->pointer=true;

			if(!top_env->put(param->syminfo->name,param->syminfo))
				cminuserror(param->syminfo,"the param has been declared.");
			else
				ofst+=PARAM_ALLOC_SIZE(param->syminfo->type);
		}

		top_env->func_info=funcinfo;
		if(strcmp(funcinfo->name,MAIN_FUNC_NAME)==0)			
			top_env->block_type=MAIN_BLOCK;
		else
			top_env->block_type=FUNC_BLOCK;		
	}
	else{ cminuserror(funcinfo,"function already declared.");delete funcinfo;}

}
void CMinus::leaveFunc(SymbolInfo* funcinfo)
{
				
}
void CMinus::declare(SymbolInfo* syminfo)
{
	if(top_env==&global_symbol_env)
		declareInSegment(syminfo);
	else declareInStack(syminfo);
}

void CMinus::declareInSegment(SymbolInfo* syminfo)
{
	char dtypes[][8]={" "," db "," dw "," "," dd "};
	syminfo->address.global=true;
	if(top_env->put(syminfo->name,syminfo))
	{
		if(syminfo->kind==KIND_VARIABLE)
			dataseg<<"\t"<<syminfo->name<<dtypes[ALLOC_SIZE(syminfo->type)]<<"?\n";
		else if(syminfo->kind==KIND_ARRAY)
		{
			if(syminfo->arr_size==-1)
			{
				cminuserror(syminfo,"array declaration without giving size.");
				syminfo->arr_size=0;
			}

			dataseg<<"\t"<<syminfo->name<<dtypes[ALLOC_SIZE(syminfo->type)]<<syminfo->arr_size<<" dup(?)\n";
		}
	}
	else{cminuserror(syminfo,"identifier already declared.");delete syminfo;}
}
void CMinus::declareInStack(SymbolInfo* syminfo)
{
	syminfo->address.global=false;

	if(top_env->put(syminfo->name,syminfo))
	{
		if(syminfo->kind==KIND_VARIABLE)
		{
			top_env->offset-=ALLOC_SIZE(syminfo->type);
			syminfo->address.offset=top_env->offset;
		}
		else if(syminfo->kind==KIND_ARRAY)
		{
			if(syminfo->arr_size==-1)
			{
				cminuserror(syminfo,"array declaration without giving size.");
				syminfo->arr_size=0;
			}

			top_env->offset-=ALLOC_SIZE(syminfo->type)*syminfo->arr_size;			
			syminfo->address.offset=top_env->offset;
		}
	}
	else{cminuserror(syminfo,"identifier already declared.");delete syminfo;}
}


