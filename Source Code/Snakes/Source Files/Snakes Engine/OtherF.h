#define CPRINT_DELAY 20

#define RED 12 // Colors Number:
#define BLUE 9
#define GREEN 10
#define YELLOW 14
#define BLACK 0
#define WHITE 15
#define GRAY 8
#define LBLUE 11
#define PINK 13

void GotoXY(short X, short Y);
void SetConsoleProperties();
void SetFColor(int FColor,int BColor);
void SetCursorShape(Bool Visible,int Size);
void SetConColors();
int  Rand(int min,int max);
int  DigitNum(int Num);
String ToStr(int Num,char res[]);
void CPrint(String S);
void RightAlignedPrint(String S,short RightX,short Y,int PrintType);
void CenterAlignedPrint(String S,short StartX,short XSpace,short Y,int PrintType);
void GetLine(String Line,int Len);
void ClearKBBuffer();
Direction GetDirection(COORD Co1,COORD Co2);
void DrawVirtualWalls(int X,int Y,int Width,int Height,int Ch,int Shape,unsigned int Interval);