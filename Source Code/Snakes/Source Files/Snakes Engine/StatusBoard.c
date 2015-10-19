#include "GameEngine.h"

void SB_Draw(char Ch,short X,short Y);
void SB_SNPreDraw(Snake *Sn);

// Draws a char on specified Coordinate of Status Board
static void SB_Draw(char Ch,short X,short Y){
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	GotoXY(GE.SBOffset.X + X,GE.SBOffset.Y + Y);
	putchar(Ch);
	ReleaseMutex(GE.hScreenMutex);
}

// Draws entire Status Board
void SB_PreDraw(){
	int i,j;
	int XO=GE.SBOffset.X,YO=GE.SBOffset.Y;
	String Mode;

	WaitForSingleObject(GE.hScreenMutex,INFINITE);//Lock Console Screen
	//First Undraw all:
	for(i=0;i<GE.ConSize.Height;i++){
		GotoXY(GE.SBOffset.X-2,i);
		for(j=0;j<SB_MAX_LEN+4;j++)
			putchar(' ');
	}

	//Draw Split Line:
	for(i=0;i<GE.SnakesNum * SB_EACH_SN_LINE_NUM + SB_HEADER_LINE_NUM;i++){
		SB_Draw(SPLIT_LINE_CH,-2,i);
		SB_Draw(SPLIT_LINE_CH2,SB_MAX_LEN + 1,i);
	}

	//Draw SB Header:
	CenterAlignedPrint("Status Board",XO,SB_MAX_LEN,YO,0);//Line 1
	
	GotoXY(XO,YO + SB_MODE_LINE_NUM); printf("Mode:"); //Line 2:
	if(GE.GameMode==Free) Mode="Free"; else Mode="Special";
	RightAlignedPrint(Mode,XO + SB_MAX_LEN - 1,YO + SB_MODE_LINE_NUM,0);
	
	GotoXY(XO,YO + SB_LEVEL_LINE_NUM); printf("Level:"); //Line 3:
	if(GE.GameMode==Special){
		GotoXY(XO + SB_MAX_LEN - DigitNum(GE.Level) - DigitNum(MAX_LEVEL_NUM) - 1,YO + SB_LEVEL_LINE_NUM);
		printf("%d/%d",GE.Level,MAX_LEVEL_NUM);
	}else{
		RightAlignedPrint("None",XO + SB_MAX_LEN - 1,YO + SB_LEVEL_LINE_NUM,0);
	}
	GotoXY(XO,YO + SB_FPS_LINE_NUM); printf("FPS:"); //Line 4:
	GotoXY(XO,YO + SB_TIME_LINE_NUM); printf("Time:"); //Line 5:
	GotoXY(XO,YO + SB_BEST_LINE_NUM); printf("Best:"); //Line 6:
	
	ReleaseMutex(GE.hScreenMutex);//Unlock Console Screen

	SB_CalcAndDrawFPS();
	SB_DrawTime();
	SB_DrawBest();
	
	for(i=0;i<SB_MAX_LEN*3/4;i++) //Line 7 -> Split Line:
		SB_Draw('/',SB_MAX_LEN/8 + i,SB_HEADER_LINE_NUM - 2);

	for(i=0;i<GE.SnakesNum;i++)
		SB_SNPreDraw(GE.Snakes[i]);
}

// Sets Status Board Offset for Drawing
void SB_SetOffset(){
	GE.SBOffset.X = GE.Offset.X + GE.Scale * GE.Width + ((GE.HaveWall) ? 3:5);
	GE.SBOffset.Y = (GE.ConSize.Height - (GE.SnakesNum * SB_EACH_SN_LINE_NUM + SB_HEADER_LINE_NUM)) / 2 + ((GE.HaveWall) ? 0:1);
	if(GE.SBOffset.Y < 0) GE.SBOffset.Y = 0;
}

// Draws Name of Best Snake on the Status Board
void SB_DrawBest(){
	int Num,i,X;
	int bests[4];
	
	GE_GetBestSnakes(&Num,bests,False);
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	X=strlen("Best:");//first Undraw previous Value:
	GotoXY(GE.SBOffset.X + X,GE.SBOffset.Y + SB_BEST_LINE_NUM);
	for(i=0;i<SB_MAX_LEN-X;i++) putchar(' ');

	SetFColor(Colors[GE.ColorNum-1][2],Colors[GE.ColorNum-1][4]);
	if(Num>1){ //if we have only 1 snake as best; draw its name otherwise print only Number of them:
		X=SB_MAX_LEN - 2*Num +1;//Draws Best Snakes Number like -> 2,4
		GotoXY(GE.SBOffset.X + X,GE.SBOffset.Y + SB_BEST_LINE_NUM);
		for(i=0;i<Num;i++){
			putchar('0' + bests[i] + 1);
			if(i != Num-1) putchar(',');
		}
	}else//Draws Best Snake Name:
		RightAlignedPrint(GE.Snakes[bests[0]]->Name,GE.SBOffset.X + SB_MAX_LEN - 1,GE.SBOffset.Y + SB_BEST_LINE_NUM,0);
	SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);

	ReleaseMutex(GE.hScreenMutex);
}

// Draws Remaining Time of the current Level on the Status Board
void SB_DrawTime(){
	int min,sec,i;
	static int ToggleShape=0;

	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	GotoXY(GE.SBOffset.X + SB_MAX_LEN - 5,GE.SBOffset.Y + SB_TIME_LINE_NUM);//Now write new Time
	if(GE.Time<0){
		putchar(' ');
		printf("None");
	}else{
		SetFColor(Colors[GE.ColorNum-1][0],Colors[GE.ColorNum-1][4]);
		min=GE.Time / 60;
		sec=GE.Time % 60;
		
		if(min<10) putchar('0');
		printf("%d:",min);
		if(sec<10) putchar('0');
		printf("%d",sec);

		for(i=0 ; i < 4 ; i++){
			if(i<=1)
				GotoXY(GE.Offset.X + GE.Width * GE.Scale / 2 - 3,GE.Offset.Y+i);//write in the middle of Upper Hor Wall of Game
			else
				GotoXY(GE.Offset.X + GE.Width * GE.Scale / 2 - 3,GE.Offset.Y + GE.Height * GE.Scale - (i - 1));//write in the middle of Lower Hor Wall of Game
			
			if(i%2==ToggleShape){
				if(min<10) putchar('0');
				printf("%d:",min);
				if(sec<10) putchar('0');
				printf("%d",sec);
			}else{
				printf("     ");
			}
		}
		ToggleShape = !ToggleShape;
		SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);
	}
	ReleaseMutex(GE.hScreenMutex);
}

// Calcs and Draws Game Real FPS on the Status Board
void SB_CalcAndDrawFPS(){
	int j;
	static clock_t LastTick;
	static float i=0,FPS=0;
	clock_t TSpan,CurT;

	CurT=clock();//Calc FPS:
	TSpan = CurT - LastTick;
	if (TSpan > 1000){
		FPS = i * (1000.0f / TSpan);
		i = 0;
		LastTick = CurT;

		// Now Draw New FPS on Console win:
		WaitForSingleObject(GE.hScreenMutex,INFINITE);//first Undraw it:
		GotoXY(GE.SBOffset.X + SB_MAX_LEN - 6,GE.SBOffset.Y + SB_FPS_LINE_NUM);
		for(j=0;j<6;j++) putchar(' ');

		GotoXY(GE.SBOffset.X + SB_MAX_LEN - DigitNum((int)FPS)-2,GE.SBOffset.Y + SB_FPS_LINE_NUM);//now write new FPS:
		printf("%.1f",FPS);
		ReleaseMutex(GE.hScreenMutex);
	}
	i++;
}

// Draws Specified Status of specified Snake on the Status Board
void SB_SNDrawStatus(Snake *Sn,StatusType Type){
	String St;
	int Y=SB_HEADER_LINE_NUM + (Sn->Num-1) * SB_EACH_SN_LINE_NUM + 2;
	
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	switch(Type){
	case SCORE:
		GotoXY(GE.SBOffset.X + SB_MAX_LEN - DigitNum(Sn->Score),GE.SBOffset.Y + Y);
		printf("%d",Sn->Score);
		break;
	case KILLSCORE:
		Y++;
		GotoXY(GE.SBOffset.X + SB_MAX_LEN - DigitNum(Sn->KillScore),GE.SBOffset.Y + Y);
		printf("%d",Sn->KillScore);
		break;
	case TOTALSCORE:
		Y+=2;
		GotoXY(GE.SBOffset.X + SB_MAX_LEN - DigitNum(Sn->KillScore + Sn->Score),GE.SBOffset.Y + Y);
		SetFColor(Colors[GE.ColorNum-1][2],Colors[GE.ColorNum-1][4]);
		printf("%d",Sn->KillScore + Sn->Score);
		SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);
		break;
	case KILLEROF:
		Y+=3;
		GotoXY(GE.SBOffset.X + SB_MAX_LEN - DigitNum(Sn->KillerOf),GE.SBOffset.Y + Y);
		printf("%d",Sn->KillerOf);
		break;
	case STATUS:
		Y+=4;
		if(Sn->IsDead) {
			St="Dead";
			SetFColor(Colors[GE.ColorNum-1][0],Colors[GE.ColorNum-1][4]);
		}else 
			St="Alive"; 
		GotoXY(GE.SBOffset.X + SB_MAX_LEN - strlen(St),GE.SBOffset.Y + Y);
		printf(St);
		SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);
		break;
	}
	ReleaseMutex(GE.hScreenMutex);
}

// Removes Specified Status of Specified Snake from the Status Board
void SB_SNUnDrawStatus(Snake *Sn,StatusType Type){
	int i,X,Y=SB_HEADER_LINE_NUM + (Sn->Num-1) * SB_EACH_SN_LINE_NUM + 2;
	
	switch(Type){
	case SCORE:
		X=strlen("Score:");
		break;
	case KILLSCORE:
		Y++;
		X=strlen("KillScore:");
		break;
	case TOTALSCORE:
		Y+=2;
		X=strlen("TotalScore:");
		break;
	case KILLEROF:
		Y+=3;
		X=strlen("Killer Of:");
		break;
	case STATUS:
		Y+=4;
		X=strlen("Status:");
		break;
	}
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	GotoXY(GE.SBOffset.X + X,GE.SBOffset.Y + Y);
	for(i=0;i<SB_MAX_LEN-X;i++)
		putchar(' ');
	ReleaseMutex(GE.hScreenMutex);
}

// Draws all of Status of specified Snake and thing all that it need
static void SB_SNPreDraw(Snake *Sn){
	int L;
	int XO=GE.SBOffset.X,YO=GE.SBOffset.Y;

	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	L=SB_HEADER_LINE_NUM + (Sn->Num - 1) * SB_EACH_SN_LINE_NUM; //Calcs Starting Line of writing
	GotoXY(XO + (SB_MAX_LEN - strlen(Sn->Name)-2) / 2, YO + L++); printf("%d.%s",Sn->Num,Sn->Name);//Line 0 -> Snake name
	L++;
	GotoXY(XO,YO + L++); printf("Score:"); SB_SNDrawStatus(Sn,SCORE);
	GotoXY(XO,YO + L++); printf("KillScore:"); SB_SNDrawStatus(Sn,KILLSCORE);
	GotoXY(XO,YO + L++); printf("TotalScore:"); SB_SNDrawStatus(Sn,TOTALSCORE);
	GotoXY(XO,YO + L++); printf("Killer Of:"); SB_SNDrawStatus(Sn,KILLEROF);
	GotoXY(XO,YO + L++); printf("Status:"); SB_SNDrawStatus(Sn,STATUS);
	ReleaseMutex(GE.hScreenMutex);
}
