#include "GameEngine.h"

// Sets Console Dimentions, Back & Fore Colors, Title and Clears all screen
void SetConsoleProperties(){
	SMALL_RECT WindowSize;
	COORD Cord={190,80};

	system("cls");
	SetConsoleScreenBufferSize(GE.ConHandle , Cord);//Set Console Buffer Size
	
	WindowSize.Left=WindowSize.Top=0;//Set Console Window Dimensions:
	WindowSize.Right=44;
	WindowSize.Bottom=31;
	SetConsoleWindowInfo(GE.ConHandle, 0, &WindowSize);

	SetConColors();
	SetCursorShape(False,25);
}

void SetFColor(int FColor,int BColor){
	SetConsoleTextAttribute(GE.ConHandle, FColor ^ (BColor<<4));
}

void SetCursorShape(Bool Visible,int Size){
	CONSOLE_CURSOR_INFO CI;
	CI.bVisible=Visible;
	CI.dwSize=Size;
	SetConsoleCursorInfo(GE.ConHandle,&CI);
}

// Set Console Background & Foreground color
void SetConColors(){
	system(SysColors[GE.ColorNum]);
}

// changes Cursor position to (X,Y)th char of console
void GotoXY(short X, short Y){
	COORD Coord;
	Coord.X = X; Coord.Y = Y;
	SetConsoleCursorPosition(GE.ConHandle,Coord);
}

// Generate random numbers in [range_min, range_max)
int Rand( int min, int max ){
	return (int)((double)rand() / (RAND_MAX+1) * (max - min) + min);
}

// Returns digits Number of entry Num including negative values.for example 234 -> 3 , -234 -> 4 
int DigitNum(int Num){
	int n=0;
	if(Num<0) n++;
	do
		n++;
	while((Num/=10));
	return n;
}

// Returns equivalent String of specified int number
String ToStr(int Num,char res[]){
	int len=DigitNum(Num),i=len-1;
	if(Num<0){
		res[0]='-';
		Num = -Num;
	}	
	do
		res[i--]= Num % 10 + '0';
	while((Num/=10));
	res[len]='\0';
	return res;
}

// Prints Specified String on the Con Win classically
void CPrint(String S){
	int c,i=0;
	for(;(c=S[i]);i++){
		putchar(c); 
		Sleep(CPRINT_DELAY);
	}
}

/* Prints specified string Right Aligned in Yth Line.last char of the string will printed in RightX position.
for Normal Print 'PrintType' param must be 0 otherwise prints slowly */
void RightAlignedPrint(String S,short RightX,short Y,int PrintType){
	GotoXY(RightX-strlen(S)+1,Y);
	if(PrintType) CPrint(S); else printf(S);
}

/* Prints specified string Center Aligned in Yth Line.
XSpace must be >= the String len.All chars of the String will be printed center of StartX and StartX + XSPace.
for Normal Print 'PrintType' param must be 0 otherwise prints slowly */
void CenterAlignedPrint(String S,short StartX,short XSpace,short Y,int PrintType){
	GotoXY(StartX + (XSpace - strlen(S)) / 2 , Y);
	if(PrintType) CPrint(S); else printf(S);
}

void GetLine(String Line,int Len){
	int c,i=0;
	while((c=getchar())!='\n')
		if(i<Len) 
			Line[i++]=c;
	Line[i]='\0';
}

// Clears Keyboard Buffer
void ClearKBBuffer(){
	while(kbhit()) getch();//clear buffer
}

// Returns needed Direction for going from Coord1 to Coord2
Direction GetDirection(COORD Co1,COORD Co2){
	int dX,dY;

	dX=Co2.X - Co1.X;
	dY=Co2.Y - Co1.Y; 
	if(dX==-1)
		return Left;
	else if(dX==1)
		return Right;
	else if(dY==-1)
		return Up;
	else
		return Down;
}

// Draws specified Walls.if shape be 0 draws a rectangle if be 1 draws two horizontal parallel lines if 2 draws two vertical lines otherwise draws nothing
void DrawVirtualWalls(int X,int Y,int Width,int Height,int Ch,int Shape,unsigned int Interval){
	int i;
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	if(Shape<=1)
		for(i = X ; i < X + Width ; i++){//Horizontal Walls:
			GotoXY(i,Y);//Up Wall:
			putchar(Ch);
			GotoXY(i,Y + Height - 1); //Down Wall:
			putchar(Ch);
			if(Interval) Sleep(Interval);
		}
	if(Shape==0 || Shape==2)
		for(i = Y ; i < Y + Height ; i++){//Horizontal Walls:
			GotoXY(X,i);//Left Wall:
			putchar(Ch);
			GotoXY(X + Width - 1,i); //Right Wall:
			putchar(Ch);
			if(Interval) Sleep(Interval);
		}
	ReleaseMutex(GE.hScreenMutex);
}