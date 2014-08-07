	char* startpos;
	int rt,curState;
	
	if(code==NULL||*code=='\0') return 0;
	
	{
		stack<int> traceStack;		

		memset(&yylval,0,sizeof(YYSTYPE));
		curState=startState;
		startpos=code;

		while(*code&&lexTable[curState][*code-1])
		{
			traceStack.push(curState);
			curState=lexTable[curState][*code-1];
			code++;
		}

		if(lexAccept[curState]==0)
		{
			curState=0;
			while(!traceStack.empty()&&lexAccept[curState]==false)
			{curState=traceStack.top();traceStack.pop();code--;}
		}	
	}
	
	if(lexAccept[curState])
	{
		strncpy(yytext,startpos,code-startpos);
		yytext[code-startpos]='\0';	

		rt=retType(curState);
		if(rt==0) 
		{
			curState=startState;
			goto Lrescan;
		}
	}
	else
	{
		printf("finderror: %c(%d)",*code,*code);
		code=startpos+1;
		curState=startState;
		goto Lrescan;
	}

	return rt;
}