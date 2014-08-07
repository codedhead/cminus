/* created by yanjun, as a little demonstration of c minus language */

char readbuf[256];
int rbuf_length;
int rbuf_pointer;

int _WINAPI_readconsole(char buf[],int cnt)
{
%{	sub esp,4
	lea eax,(-4)[ebp]
	invoke ReadConsole,_YaNjUn_hStdin,8[ebp],12[ebp],eax,NULL
	mov eax,(-4)[ebp]%}
}
void _WINAPI_writeconsole(char buf[],int cnt)
{
%{	sub esp,4
	lea eax,(-4)[ebp]
	invoke WriteConsole,_YaNjUn_hStdout,8[ebp],12[ebp],eax,NULL%}
}

int readInt(void)
{
	char c;	
	int getit,res;
	getit=0;
	res=0;	
	
	while(getit==0)
	{
		if(rbuf_pointer>=rbuf_length)
		{
			rbuf_length=_WINAPI_readconsole(readbuf,255);
			rbuf_pointer=0;
		}

		while(c=readbuf[rbuf_pointer])
		{
			rbuf_pointer=rbuf_pointer+1;
			/* encounter bad character(not digit) */
			if(c<'0'||c>'9')
				break;
			else
			{
				getit=1;
				res=res*10+c-'0';
			}			
		}
	}
	return res;
}

void getchar(void)
{
	_WINAPI_readconsole(readbuf,1);
}
void revBuf(char buf[],int l,int r)
{
	char temp;
	while(l<r)
	{
		temp=buf[l];
		buf[l]=buf[r];
		buf[r]=temp;
		l=l+1;r=r-1;
	}
}

void writeInt(int x)
{
	int cnt;
	char buf[256];
	cnt=0;
	if(x)
	{
		int neg;
		neg=0;
		if(x<0){neg=1;x=-x;}
		while(x)
		{
			buf[cnt]=x%10+'0';
			x=x/10;
			++cnt;
		}
		if(neg) 
		{buf[cnt]='-';++cnt;}
		revBuf(buf,0,cnt-1);
	}
	else
	{
		cnt=1;
		buf[0]='0';
	}
	buf[cnt]=' ';buf[cnt+1]='\0';

	_WINAPI_writeconsole(buf,cnt+1);
}
void writeln(void)
{
	char buf[2];
	buf[0]='\n';
	_WINAPI_writeconsole(buf,1);
}


void quicksort(int data[], int N)
{
	int i, j;
	int v, t;

	if( N <= 1 )
		return;

	v = data[0];
	i = 0;
	j = N;
	for(;;)
	{
		while(data[++i] < v && i < N);
		while(data[--j] > v);
		if( i >= j )
			break;
		t = data[i];
		data[i] = data[j];
		data[j] = t;
	}
	t = data[i-1];
	data[i-1] = data[0];
	data[0] = t;
	quicksort(data, i-1);
	quicksort(data+i, N-i);
}


void main(void)
{
	int i,arr[25];
	rbuf_length=0;
	rbuf_pointer=0;

	i=0;
	for(i=0;i<10;++i)
		arr[i]=readInt();

	quicksort(arr,10);

	for(i=0;i<10;++i)
		writeInt(arr[i]);
	writeln();

	getchar();
	
}