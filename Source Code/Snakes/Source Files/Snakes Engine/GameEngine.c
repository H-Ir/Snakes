#include "GameEngine.h"

#define MUSH_SCORE 100 //Score of a normal Mushroom(Base Score of other Scores)
#define HMUSH_SCORE (15*MUSH_SCORE) //HMush Score Coefficient
#define FMUSH_SCORE (5*MUSH_SCORE) //FMush Score coefficient
#define BASE_KILLSCORE (15*MUSH_SCORE) //Base Kill Score in Special Mode
#define S_KILLSCORE_COE 1.5f  
#define F_KILLSCORE_COE (1/100.0f)

#define FMUSH_FPS_INCREASING 15 //FPS Coefficient of FMush.when an snake eat FMush this Coe must be multiply to GE.FPS
#define HIGH_SPEED_TIME 6 //Time in sec that game must Render in High speed(FPS)

//League Mode Standards:
#define LM_WIDTH 80 //Game width in League Mode
#define LM_HEIGHT 25 //Game Height in League Mode
#define LM_FPS 25 //Game FPS in League Mode
#define LM_SNAKEBODY_LEN 15

//Ascii Codes:
#define UP_KEY 72
#define DOWN_KEY 80
#define RIGHT_KEY 77
#define LEFT_KEY 75

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Function Declarations: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void GE_Initialize(GameMode GameM,short Width,short Height,int Scale,float FPS,int SnakesNum,
				   int PlayersNum,int MushNum,int PMushNum,Difficulty Diffic,Bool HaveWall,String PlayerNames[]);
void GE_Render();
void GE_MoveSnakes();
void GE_KillSnake(Snake* Sn);
void GE_GoDeathState(void *ArgList);
int GE_SetPlayersDir();
void GE_NextLevel();
Bool GE_Pause();
void GE_Dispose();
void GE_StartTimer(void *Args);
void GE_PreDraw();
void GE_RedrawColoredObjs();
void GE_DrawLevelNum();
void GE_DropMushs(int MushNum,int PMushNum);
void GE_ManageSpecialMushs();
void GE_DropBranchs(Bool HaveWall);
void GE_RemoveMush(Mushroom *Mu);
void GE_SetOffset();


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Variables: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

GameEngine GE;
int PlayerScoreSum,PlayerKScoreSum;//Player Score & KillScore Sum until current level
char *SysColors[]={"Color CF","Color 0F","Color 1e",
				"Color 70","Color 09","Color F0",
				"Color F9","Color 9F","Color a0","Color 0a"};// First num is Background and other is Foreground.
// Back & Font Colors for SetFColor() Function.Colors: {PMUSH,FMUSH,HMUSH,Foreground,Background} :
int Colors[][6]={{RED,BLUE,GREEN,WHITE,BLACK,PINK},{RED,WHITE,GREEN,YELLOW,0x1,GRAY},{RED,BLUE,GREEN,BLACK,0x7,PINK},
				  {PINK,GRAY,YELLOW,BLUE,BLACK,GREEN},{GREEN,BLUE,RED,BLACK,WHITE,PINK},{GREEN,BLACK,RED,BLUE,WHITE,PINK},
				  {PINK,BLACK,YELLOW,WHITE,BLUE,GRAY},{RED,BLUE,WHITE,BLACK,GREEN,LBLUE},{RED,PINK,YELLOW,GREEN,BLACK,LBLUE}};
static Direction Player1_Dir=Right,Player2_Dir=Left;//Contains Direction of 2 Players that are used only in GE_SetPlayersDir()

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Functions: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Launches
 //Launches Game and manages it until user exits from Game
void GE_LaunchManageGame(){
	GameMode GM;
	short W=0,H=0;//Game Field Width & Height
	int Scale=0,SnakesN=0,PlayersN=0,MushN=0,PMushN=0;
	float FPS=0;
	Difficulty Diffic;
	Bool HWall=False;
	String PNames[2];// Keeps Player Names
	int PlayAgain;

	while((GM=(GameMode)MM_ShowMainMenu())){ //Main loop of game.if MM_ShowMainMenu returned 0(Exit) then close the program.
		if(GM==4)//if user wants to change color of screen:
			MM_ShowColorMenu();
		else{
			switch(GM){//First -> Set Game Engine Parameters for initializing:
			case League:
				W=LM_WIDTH; H=LM_HEIGHT; FPS=LM_FPS; //sets League mode standard values 
				PNames[0]="Player";
				break;
			case Special:
				GE.Level=1;
				MM_ShowConfigMenu(GM,&W,&H,0,&FPS,0,0,&MushN,&PMushN,&Diffic,0,PNames);//Show Config Screen to get preferred options of user
				PlayerScoreSum=PlayerKScoreSum=0;
				break;
			case Free:
				MM_ShowConfigMenu(GM,&W,&H,&Scale,&FPS,&SnakesN,&PlayersN,&MushN,&PMushN,&Diffic,&HWall,PNames);//Show Config Screen to get preferred options of user
			}
			SetConsoleProperties();//Prepares Console win for game rendering like changing color,Win dimensions,Cursor view ...
			GE_Initialize(GM,W,H,Scale,FPS,SnakesN,PlayersN,MushN,PMushN,Diffic,HWall,PNames);//Now Initialize Game Engine(GE)
			do{
				if(GE.GameMode==Special)// Draws Level Number in the middle of the Screen:
					GE_DrawLevelNum();
				GE_PreDraw();//Draws all objects of game on the console Win
				Sleep(1000);
				ClearKBBuffer(); //while user has pressed any key remove them from keyboard buffer because they have been pressed before game start 

				while(GE.GameStatus==Running){
					GE_Render();//Render Game
					Sleep((DWORD)(1000/GE.FPS));//Render Game with Specified FPS
				}

				do 
					Sleep((GE.SnakesNum-1)*1300); 
				while(!GE.CanEndGame);//wait for other threads to be closed

				if(GE.GameStatus==Quitted)
					GE_Dispose();//release all resources & allocated memories & Dispose Game Engine and prepare to go Main Menu
				else if(GE.GameStatus==GoNextLevel){
					SB_ShowScoreBoard();
					GE_NextLevel();
				}else{
					PlayAgain=SB_ShowScoreBoard();
					GE_Dispose();//release all resources & allocated memories & Dispose Game Engine and prepare to go Main Menu
					if(PlayAgain && GE.GameMode == Free){
						GE_Initialize(GM,W,H,Scale,FPS,SnakesN,PlayersN,MushN,PMushN,Diffic,HWall,PNames);//Now Initialize Game Engine(GE)
						SetConColors();
					}
				}
			}while(GE.GameStatus==Running);// when GameStatus is steal 'Running' means GE_NextLevel() have been called and we must go next level
		}
	}
}

//Prepares Game engine for running game.You must call this func only once in your main function!!! call it before launching game.
void GE_Prepare(){
	TIMECAPS tc;

	timeGetDevCaps(&tc, sizeof(TIMECAPS)); //Get min timer resolution supported by this machine
	timeBeginPeriod(max(tc.wPeriodMin, 1)); //Use Multimedia Timer for increasing accuracy of Game FPS
	srand((unsigned)time(NULL)); //Make Rand function to produce different numbers every time app runs.
	
	GE.ConHandle=GetStdHandle(STD_OUTPUT_HANDLE); //Get Console Handle
	GE.ColorNum=1;//default color of console win
	GE.hScreenMutex=CreateMutex(NULL,0,NULL);
	GE.ConSize.Width=-1;
	GE.ConSize.Height=-1;
	GE_CheckConSize();

	SetConsoleTitleA(CONSOLE_TITLE); //Changes Console Title
}

//Prepares Game Engine for closing.You must call it only once in your main function after Launch game.(when user exited from game)
void GE_Close(){
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS)); //Get min timer resolution supported by this machine
	timeEndPeriod(max(tc.wPeriodMin, 1));// when timeBeginPeriod() have been called we must call this func to end it
}

/* 
Initialize the Game Engine before rendering.
Width,Height and FPS in league mode must be LM_WIDTH,LM_HEIGHT,LM_FPS defines.
You can Set SnakesNum and PlayerNum parameters 0 in League and Special Mode.PMushNum can be Null if GameMode is Free or League.
You don't need set BranchsNum value in league mode.it can be 0 to n in Free and Special modes.
change GE.Level before calling this func.
*/
static void GE_Initialize(GameMode GameM,short Width,short Height,int Scale,float FPS,int SnakesNum,
				   int PlayersNum,int MushNum,int PMushNum,Difficulty Diffic,Bool HaveWall,String PlayerNames[]){
	int i,j,BodyL,Min=0;// Min-> min value of X and Y that we can Create Snakes in that positions(because of existing around Walls)
	COORD SnPos,*Sn_BodyPos;

	GE.Width=Width;
	GE.Height=Height;
	GE.FPS=FPS;
	GE.GameMode=GameM;
	GE.GameStatus=Running;
	GE.FMush=0;//Points to nothing
	GE.HMush=0;//Points to nothing
	GE.Inning=0;
	GE.CanEndGame=True;
	GE.Time=-1;
	GE.HighSpeed_RTime=0;
	GE.Paused=False;
	GE.FrameCounter=0;
	GE.Diffic=Diffic;

	GE.GameField=(GameObjs**)malloc(sizeof(GameObjs*) * GE.Width);
	for(i=0;i<GE.Width;i++)
		GE.GameField[i]=(GameObjs*)malloc(sizeof(GameObjs) * GE.Height);
	GE.GameFieldObj=(void***)malloc(sizeof(void**) * GE.Width);
	for(i=0;i<GE.Width;i++)
		GE.GameFieldObj[i]=(void**)malloc(sizeof(void*) * GE.Height);
	for(i=0;i<GE.Width;i++)
		for(j=0;j<GE.Height;j++){
			GE.GameField[i][j]=EMPTY;
			GE.GameFieldObj[i][j]=NULL;//Points to nothing
		}

	switch(GameM){
	case League:
		GE.Scale=1;
		GE.SnakesNum=2;
		GE.MushNum = GE.PriMushNum = GE.PMushNum = GE.BranchsNum = 0;
		GE.HaveWall=False;
		GE.Branchs=0;//Points to nothing
		GE.Mushs=0;//Points to nothing
		GE.Level=1;
		GE.Diffic=VeryHard;
		PlayersNum=1;
		BodyL=LM_SNAKEBODY_LEN;
		
		break;
	case Special:
		GE.Scale=2;
		GE.SnakesNum=2;
		GE.HaveWall=True;
		PlayersNum=1;
		BodyL=SNBODY_MIN_LEN;

		GE_DropBranchs(GE.HaveWall);
		GE_DropMushs(MushNum,PMushNum);
		break;
	case Free:
		GE.Scale=Scale;
		GE.SnakesNum=SnakesNum;
		GE.Level=1;
		GE.HaveWall=HaveWall;
		if(MushNum==0){
			BodyL=min(GE.Width,GE.Height) * 7/8 - 5;
			if(GE.HaveWall) BodyL -= 2;
		}
		else 
			BodyL=SNBODY_MIN_LEN;

		GE_DropBranchs(HaveWall);
		GE_DropMushs(MushNum,PMushNum);
		break;
	}
	
	GE.AliveSnakesNum=GE.SnakesNum;
	
	GE.ConSize.Width = GE.ConSize.Height = 0;
	GE_CheckConSize();//Sets Console window Width, Height and Sets Drawing Offsets

	if(GE.HaveWall) Min=1;
	Sn_BodyPos=(COORD*)malloc(sizeof(COORD)*BodyL);
	
	//Snake 1 Creation:
	SnPos.X=BodyL+Min; //Head Position:
	SnPos.Y=Min;
	for(i=0;i<BodyL;i++){ //Body Position:
		Sn_BodyPos[i].X=BodyL-i-1+Min;
		Sn_BodyPos[i].Y=Min;
	}
	GE.Snakes[0]=SN_Create(PlayerNames[0],1,SnPos,BodyL,Sn_BodyPos,False,Right);
	Player1_Dir=Right;

	//Snake 2 Creation:
	SnPos.X=GE.Width-BodyL-1-Min; //Head Position:
	SnPos.Y=GE.Height-1-Min;
	for(i=0;i<BodyL;i++){ //Body Position:
		Sn_BodyPos[i].X=GE.Width-BodyL+i-Min;
		Sn_BodyPos[i].Y=GE.Height-1-Min;
	}
	if(PlayersNum==1)//if Snake 2 Is not Player:
		GE.Snakes[1]=SN_Create((GE.SnakesNum>2) ? "Com 1":"Com",2,SnPos,BodyL,Sn_BodyPos,True,Left);
	else{//if Snake 2 is Player:
		GE.Snakes[1]=SN_Create(PlayerNames[1],2,SnPos,BodyL,Sn_BodyPos,False,Left);
		Player2_Dir=Left;
	}
	if(SnakesNum>=3){
		//Snake 3 Creation:
		SnPos.X=GE.Width-1-Min;
		SnPos.Y=BodyL+Min; //Head Position:
		for(i=0;i<BodyL;i++){ //Body Position:
			Sn_BodyPos[i].X=GE.Width-1-Min;
			Sn_BodyPos[i].Y=BodyL-i-1+Min;
		}
		GE.Snakes[2]=SN_Create((PlayersNum==1) ? "Com 2":"Com 1",3,SnPos,BodyL,Sn_BodyPos,True,Down);

		if(SnakesNum==4){
			//Snake 4 Creation:
			SnPos.X=Min; //Head Position:
			SnPos.Y=GE.Height-BodyL-1-Min;
			for(i=0;i<BodyL;i++){ //Body Position:
				Sn_BodyPos[i].X=Min;
				Sn_BodyPos[i].Y=GE.Height-BodyL+i-Min;
			}
			GE.Snakes[3]=SN_Create((PlayersNum==1) ? "Com 3":"Com 2",4,SnPos,BodyL,Sn_BodyPos,True,Up);
		}
	}
}

// Renders Game and does any thing need for implementing a frame of game, like Snakes Moving,Devices,Memory,Console Win & Game management
static void GE_Render(){
	if (GE_CheckConSize())//if Console Win  have been changed,Redraw All thing on the console again:
		GE_PreDraw();
	
	if(!GE_SetPlayersDir())//Sets Players Directions
		if(GE_Pause())//if ESC Key was pressed then goto pause(Quit) State:
			GE.GameStatus=Quitted;
	
	if(GE.Snakes[GE.Inning]->IsCom)
		SetComSnakeDirection(GE.Snakes[GE.Inning]);
	
	if(GE.GameStatus==Running){//if game is not quited yet, keep going:
		GE.FrameCounter++;
		GE_MoveSnakes();//Move Snakes

		if(GE.GameMode!=League) GE_ManageSpecialMushs();
		if(GE.FrameCounter % (int)(GE.FPS / 2) == 0){
			if(GE.HMush) MU_Draw(GE.HMush);//Change Special Mushs(F & H Mushs) Shape and Redraw them on the console win:
			if(GE.FMush) MU_Draw(GE.FMush);
		}

		if(GE.GameMode == Special && GE.MushNum == 0 && !GE.FMush && !GE.HMush && GE.Time < 0)//if Game Mode is Special and we haven't any positive Mushs then start level Timer 
			_beginthread(GE_StartTimer,0,NULL);

		if(GE.GameMode!=League) //Draw Real FPS on the Status Board:
			SB_CalcAndDrawFPS();
		
		if(GE.HighSpeed_RTime>0){//Manages High Speed(High FPS) Mode(High Speed Mode Started when Com Snake eats FMush:
			GE.HighSpeed_RTime--;
			if(!GE.HighSpeed_RTime) GE.FPS -= FMUSH_FPS_INCREASING;
		}

		if(GE.AliveSnakesNum == 1 || GE.Time == 0 ||
		  (GE.GameMode==Special && GE.Snakes[0]->IsDead)){//when Game or this level ended:
			if(GE.GameMode<=Free)// League & Free modes or Special mode that is in last level:
				GE.GameStatus=GameEnded;
			else{//Special mode:
				if(GE.Snakes[0]->IsDead)//Player is dead:
					GE.GameStatus=GameOver;
				else if(GE.AliveSnakesNum==1)//Com is dead:
					GE.GameStatus=GoNextLevel;		
				else //Time is over:
					if (GE_GetWinner()->IsCom)
						GE.GameStatus=GameOver;
					else
						GE.GameStatus=GoNextLevel;
				if(GE.Level==MAX_LEVEL_NUM && GE.GameStatus==GoNextLevel) GE.GameStatus=GameEnded;
			}
		}
	}
}

// Moves all Snakes and does any thing need like kill,Mushroom management,...
static void GE_MoveSnakes(){
	int dX=0,dY=0;
	int X,Y,NX,NY;
	int BodyLenDiff;
	int i,Len;
	int t;
	Snake *Sn,*KillerSn;
	Snake *PaSN,*ChSN; //Parent and Child Snakes
	COORD tmp;

	Sn=GE.Snakes[GE.Inning];
	X=Sn->Position.X; Y=Sn->Position.Y;

	switch(Sn->Direction){
	case Up:
		dY=-1; break;
	case Down:
		dY=1; break;
	case Left:
		dX=-1; break;
	case Right:
		dX=1; break;
	}
	
	NX = X + dX;//Calc New Position of Snake Head:
	NY = Y + dY;
	if(NX==GE.Width)
		NX = 0;
	else if(NX<0)
		NX = GE.Width-1;
	if(NY==GE.Height)
		NY = 0;
	else if(NY<0)
		NY = GE.Height-1;

	switch(GE.GameField[NX][NY]){
	case EMPTY:
		SN_Move(Sn,Move,NX,NY);
		break;
	case MUSH:
		GE_RemoveMush((Mushroom*)GE.GameFieldObj[NX][NY]);
		SN_Move(Sn,EatAndMove,NX,NY);
		SN_AddScore(Sn,GE.Level * MUSH_SCORE);
		break;
	case PMUSH:
		GE_RemoveMush((Mushroom*)GE.GameFieldObj[NX][NY]);
		SN_Move(Sn,MiniAndMove,NX,NY);
		SN_AddScore(Sn,SNBODY_MINI_NUM * GE.Level * -MUSH_SCORE);
		break;
	case HMUSH:
		free(GE.HMush);
		GE.HMush=0;
		if(Sn->IsCom){
			SN_Move(Sn,HalfAndMove,NX,NY);
			SB_SetOffset();
			SB_PreDraw();

			//now increase FPS because we have new Snake in the game in order not to change Realness FPS:
			GE.FPS += (GE.FPS - (GE.HighSpeed_RTime)?FMUSH_FPS_INCREASING:0) / GE.AliveSnakesNum;
		}
		else{
			SN_Move(Sn,Move,NX,NY);
			SN_AddScore(Sn,HMUSH_SCORE * GE.Level);
		}
		break;
	case FMUSH:
		free(GE.FMush);
		GE.FMush=0;
		SN_Move(Sn,Move,NX,NY);
		if(Sn->IsCom){
			if(!GE.HighSpeed_RTime) GE.FPS += FMUSH_FPS_INCREASING;
			GE.HighSpeed_RTime +=(int)(GE.FPS * HIGH_SPEED_TIME);
		}
		else{
			SN_AddScore(Sn,FMUSH_SCORE * GE.Level);
		}
		break;
	case SNAKEBODY: case SNAKEHEAD:
		KillerSn=(Snake*)GE.GameFieldObj[NX][NY];
		if(GE.GameMode!=League){ //Calc KillScore of Killer Snake:
			if(KillerSn!=Sn && !(GE.GameMode==Special && Sn->IsCom && KillerSn->IsCom)){
				KillerSn->KillerOf++;
				if((BodyLenDiff=Sn->BodyLen - KillerSn->BodyLen)<0) BodyLenDiff=0;
				BodyLenDiff *= MUSH_SCORE;
				if(GE.GameMode==Free){
					if(Sn->KillScore > 0)
						KillerSn->KillScore += (int)(Sn->KillScore * S_KILLSCORE_COE) + BodyLenDiff;
					else
						KillerSn->KillScore += BASE_KILLSCORE + (int)(GE.Width * GE.Height * 
												F_KILLSCORE_COE * MUSH_SCORE) + BodyLenDiff;
				}else{//Special Mode:
					KillerSn->KillScore += (BASE_KILLSCORE + BodyLenDiff) * GE.Level;
				}
				KillerSn->KillScore = KillerSn->KillScore - KillerSn->KillScore % 100; //Round Kill Score to 100

				SB_SNUnDrawStatus(KillerSn,KILLEROF);//Now ReDraw Kill Score, Total Score, 'Killer Of' and Best Snake(s):
				SB_SNDrawStatus(KillerSn,KILLEROF);
				SB_SNUnDrawStatus(KillerSn,KILLSCORE);
				SB_SNDrawStatus(KillerSn,KILLSCORE);
				SB_SNUnDrawStatus(KillerSn,TOTALSCORE);
				SB_SNDrawStatus(KillerSn,TOTALSCORE);
				SB_DrawBest();
			}
		}

		//in special mode if two com snakes head collide each other when timer have started then merge them to one snake and add their scores:
		if(GE.GameMode==Special && GE.Time >= 0 && KillerSn->IsCom && Sn->IsCom && GE.GameField[NX][NY]==SNAKEHEAD){
			SN_Move(Sn,Move,NX,NY);//first move Sn before Snakes merging
			
			if(Sn->Num < KillerSn->Num){// set Parent and child Snakes: Lower Number(index in GE.Snakes) is Parent and other is child:
				PaSN=Sn; ChSN=KillerSn;
			}else{
				PaSN=KillerSn; ChSN=Sn;
			}

			//Now Merging two Snakes:
			PaSN->Position = PaSN->Body[PaSN->BodyLen-1];//last Body of PaSN will be Head of it 
			if(PaSN->BodyLen==1) 
				PaSN->Direction=AdverseDir(PaSN->Direction);
			else
				PaSN->Direction=GetDirection(PaSN->Body[PaSN->BodyLen-2],PaSN->Body[PaSN->BodyLen-1]);
			SN_DrawHead(PaSN);
			GE_SetGameField(PaSN->Position.X,PaSN->Position.Y,SNAKEHEAD,PaSN);

			PaSN->BodyLen--;//Replace all first bodies of PaSN with last bodies of it because now Sn head is in end of it
			for(i=0 ; i < PaSN->BodyLen / 2 ; i++){
				tmp=PaSN->Body[i];
				PaSN->Body[i]=PaSN->Body[PaSN->BodyLen - 1 - i];
				PaSN->Body[PaSN->BodyLen - 1 - i]=tmp;
			}
			
			PaSN->Body[PaSN->BodyLen] = ChSN->Position;//now Head of Child Snake is Body of Parent Sn:
			SetFColor(Colors[GE.ColorNum-1][5],Colors[GE.ColorNum-1][4]);
			GE_SDraw(COM_SNAKEBODY_CH,ChSN->Position.X,ChSN->Position.Y);
			SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);
			GE_SetGameField(ChSN->Position.X,ChSN->Position.Y,SNAKEBODY,PaSN);

			Len = (PaSN->BodyLen + 1 + ChSN->BodyLen);// Calc New Len of Snake Body;
			PaSN->BodyLen = Len;
			if(PaSN->BodyLen > PaSN->BodyRealLen){
				PaSN->BodyRealLen = ((Len / SN_BODYLEN_STEP) + 1) * SN_BODYLEN_STEP;
				PaSN->Body = (COORD *)realloc(PaSN->Body,Sn->BodyRealLen * sizeof(COORD));//Now ReAllocate memory for Pa Snake Body
			}

			for(i=0 ; i < ChSN->BodyLen ; i++){//all Body of Child Sn will be Body of Parent Sn:
				PaSN->Body[Len - ChSN->BodyLen + i]=ChSN->Body[i]; 
				GE_SetGameField(ChSN->Body[i].X,ChSN->Body[i].Y,SNAKEBODY,PaSN);
			}
			PaSN->Score += ChSN->Score;// add up Scores of two Snakes and assign to Pa Sn:
			PaSN->KillScore += ChSN->KillScore;

			i=ChSN->Num-1;
			free(ChSN->Name);//release allocated memory of child Snake:
			free(ChSN->Body);
			free(ChSN);
			for(; i < GE.SnakesNum - 1 ; i++){ //Reduce Index & Number of Next Snakes(Snakes that have higher Index than Child Sn Index):
				GE.Snakes[i]=GE.Snakes[i+1];
				GE.Snakes[i]->Num--;
			}
			GE.SnakesNum--; //Reduce Snakes Number and remove
			GE.AliveSnakesNum--;

			SB_SetOffset();
			SB_PreDraw();//ReDraw All Status Board
		}
		else{//otherwise kill snake:
			GE_KillSnake(GE.Snakes[GE.Inning]);
		}

		//now decrease FPS in order not to change Realness FPS because we removed a Snake from the game:
		if(GE.GameMode==Special && GE.Inning!=0 && GE.SnakesNum>2) 
			GE.FPS -= (GE.FPS - ((GE.HighSpeed_RTime)?FMUSH_FPS_INCREASING:0)) / (float)(GE.AliveSnakesNum+1);
		break;
	default:// BRANCH:
		GE_KillSnake(GE.Snakes[GE.Inning]);
		//now decrease FPS in order not to change Realness FPS because we removed a Snake from the game:
		if(GE.GameMode==Special && GE.Inning!=0 && GE.SnakesNum>2) 
			GE.FPS -= (GE.FPS - ((GE.HighSpeed_RTime)?FMUSH_FPS_INCREASING:0)) / (float)(GE.AliveSnakesNum+1);
	}
	
	do // Manage Inning of Snakes Moving:
		GE.Inning= ++GE.Inning % GE.SnakesNum;
	while(GE.Snakes[GE.Inning]->IsDead);
}

// Kills Specified Snake and removes it from screen
static void GE_KillSnake(Snake* Sn){
	int i;

	Sn->IsDead=True;
	SN_UnDraw(Sn);
	GE_SetGameField(Sn->Position.X,Sn->Position.Y,EMPTY,0);
	for(i=0;i<Sn->BodyLen;i++)
		GE_SetGameField(Sn->Body[i].X,Sn->Body[i].Y,EMPTY,0);
	GE.AliveSnakesNum--;

	printf("\a");
	_beginthread(GE_GoDeathState,0,(void*)Sn);
}

/* 
When an snake died, this function creates red flashs by a new thread.
This func need 2 argument: 1)dead snake(Snake* type) 2)Current Game Level Num(int type)
*/
static void GE_GoDeathState(void *ArgList){
	Snake *Sn=(Snake *)ArgList;
	int i=0,RepN;
	HANDLE hScreenMutex;

	hScreenMutex=CreateMutex(NULL,0,NULL);
	WaitForSingleObject(hScreenMutex,INFINITE);//Lock this block:

	GE.CanEndGame=False;//while this thread have not been closed dont let game ends or go to next level
	RepN=DEATH_MSG_REAPET_NUM;
	while(i<RepN*4){
		if(i % 2 == 0){
			if(GE.GameMode!=League) SB_SNUnDrawStatus(Sn,STATUS);
			if(i<RepN*2) system(SysColors[0]);
		}else{
			if(GE.GameMode!=League) SB_SNDrawStatus(Sn,STATUS);
			if(i<RepN*2) system(SysColors[GE.ColorNum]);
		}
		Sleep(DEATH_MSG_INTERVAL); i++;
	}

	if(GE.GameMode!=League) GE_RedrawColoredObjs();
	GE.CanEndGame=True;

	ReleaseMutex(hScreenMutex);//Release this block for other threads
}

// Handles Keyboard and Sets Players moving direction.if ESC Key is pressed returns 0 otherwise returns 1 
static int GE_SetPlayersDir(){
	while(kbhit()){
		switch(getch()){
		case 'w': case 'W':
			if(GE.Snakes[0]->Direction > 1) Player1_Dir= Up;
			break;
		case 's': case 'S':
			if(GE.Snakes[0]->Direction > 1) Player1_Dir= Down;
			break;
		case 'a': case 'A':
			if(GE.Snakes[0]->Direction <= 1) Player1_Dir= Left;
			break;
		case 'd': case 'D':
			if(GE.Snakes[0]->Direction <= 1) Player1_Dir= Right;
			break;
		case UP_KEY:
			if(GE.Snakes[1]->IsCom==False && GE.Snakes[1]->Direction > 1) Player2_Dir = Up; 
			break;
		case DOWN_KEY:
			if(GE.Snakes[1]->IsCom==False && GE.Snakes[1]->Direction > 1) Player2_Dir = Down;
			break;
		case LEFT_KEY:
			if(GE.Snakes[1]->IsCom==False && GE.Snakes[1]->Direction <= 1) Player2_Dir = Left;
			break;
		case RIGHT_KEY:
			if(GE.Snakes[1]->IsCom==False && GE.Snakes[1]->Direction <= 1) Player2_Dir = Right;
			break;
		case ESC_KEY: 
			return 0;//ask user "do you want to quit?"
		}
	}
	if(GE.Inning==0) GE.Snakes[0]->Direction=Player1_Dir;
	if(GE.Snakes[1]->IsCom==False && GE.Inning==1) GE.Snakes[1]->Direction=Player2_Dir;
	return 1;
}

// Gets Winner of the Game Round
Snake *GE_GetWinner(){
	int Bests[4],Num,i=0;

	if(GE.GameMode==League)
		if(GE.Snakes[0]->IsDead) return GE.Snakes[1]; else return GE.Snakes[0];
	else{ // Free & Special Modes:
		if(GE.GameMode==Special){
			GE_GetBestSnakes(&Num,Bests,True);//search between alive snakes and get best snakes of them by its scores
			return GE.Snakes[Bests[0]];
		}else{//Free Mode:
			GE_GetBestSnakes(&Num,Bests,False);
			if(Num>1){//alive Snake is winner:
				for(i=0;i<GE.SnakesNum;i++)
					if(!GE.Snakes[i]->IsDead) return GE.Snakes[i];
			}else{//largest score is winner:
				return GE.Snakes[Bests[0]];
			}
		}
	}
}

// Returns index of Best Snakes that have highest total score in a array
void GE_GetBestSnakes(int *Num,int Bests[],Bool BetweenAliveSN){
	int i,BS = INT_MIN,tmp;
	
	*Num=0;
	for(i=0;i<GE.SnakesNum;i++){//calcs best Snakes:		
		if(BetweenAliveSN && GE.Snakes[i]->IsDead) 
			continue;
		tmp=GE.Snakes[i]->Score + GE.Snakes[i]->KillScore;
		if(tmp > BS){
			BS=tmp;
			Bests[0]=i;
			*Num=1;
		}else if(tmp==BS){
			Bests[*Num]=i;
			++*Num;
		}
	}
}

// Prepare Game Engine for next level
static void GE_NextLevel(){
	int W,H,MushN,PMushN;
	float FPS;
	String PName[1];
	Difficulty Diffic;

	PlayerScoreSum += GE.Snakes[0]->Score;
	PlayerKScoreSum += GE.Snakes[0]->KillScore;
	PName[0]=strdup(GE.Snakes[0]->Name);
	Diffic = GE.Diffic;

	GE_Dispose();

	W = LE_WIDTH_MAX - (int)(((float)GE.Level / (MAX_LEVEL_NUM-1)) * (LE_WIDTH_MAX - LE_WIDTH_MIN));
	H = LE_HEIGHT_MAX - (int)(((float)GE.Level / (MAX_LEVEL_NUM-1)) * (LE_HEIGHT_MAX - LE_HEIGHT_MIN));
	MushN = LE_MUSH_MAX_NUM - (int)(((float)GE.Level / (MAX_LEVEL_NUM-1)) * (LE_MUSH_MAX_NUM - LE_MUSH_MIN_NUM));
	PMushN = (int)((LE_PMUSH_MAX_NUM - ((float)GE.Level / (MAX_LEVEL_NUM-1)) * (LE_PMUSH_MAX_NUM - LE_PMUSH_MIN_NUM)) * (6-GE.Diffic)/6);
	FPS = LE_FPS_MIN + (int)(((float)GE.Level / (MAX_LEVEL_NUM-1)) * (LE_FPS_MAX - LE_FPS_MIN) * (8-GE.Diffic)/8);

	GE.Level++;
	SetConColors();// Return User defined Color
	GE_Initialize(Special,W,H,2,FPS,2,1,MushN,PMushN,Diffic,True,PName);
}

// Pauses the Game and writes 'Quit' Question
static Bool GE_Pause(){
	String QMsg="Quit to Main Menu?(Y/N)";
	int MsgL=strlen(QMsg);
	int i,j,SX,c,ColNum;

	while(!GE.CanEndGame) Sleep((DWORD)100);
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	GE.Paused=True;
	ColNum=(GE.ColorNum+1) % MAX_COLOR_NUM;
	system(SysColors[((ColNum)? ColNum:1)]);
	if(GE.GameMode==League)
		SX=GE.Center.X - MsgL/2 - 4;
	else
		SX=GE.Center.X - (SB_MAX_LEN + 6 + MsgL)/2 - 4;
	if(SX<0) SX=0;
	for(i=-3;i<=+3;i++){
		GotoXY(SX,GE.Center.Y+i);
		for(j=-4;j<MsgL+4;j++)
			putchar(' ');
	}
	GotoXY(SX+4,GE.Center.Y);
	SetCursorShape(True,100);
	CPrint(QMsg);
	while((c=tolower(getch()))!='y' && c!='n' && c!=ESC_KEY);
	if(c=='y') printf("Y");
	ReleaseMutex(GE.hScreenMutex);
	if(c=='y'){
		return True;
	}else{
		GE.Paused=False;
		SetConColors();
		SetCursorShape(False,25);
		GE_PreDraw();
		return False;
	}
}

//Releases all resources and allocated memories
static void GE_Dispose(){
	int i;
	for(i=0;i<GE.Width;i++){
		free(GE.GameField[i]);
		free(GE.GameFieldObj[i]);
	}
	free(GE.GameField);free(GE.GameFieldObj);
	for(i=0;i<GE.SnakesNum;i++){
		free(GE.Snakes[i]->Body);
		free(GE.Snakes[i]->Name);
		free(GE.Snakes[i]);
	}
	for(i=0;i<GE.BranchsNum;i++)
		free(GE.Branchs[i]);
	for(i=0;i<GE.MushNum + GE.PMushNum;i++)
		free(GE.Mushs[i]);
	if(GE.Branchs) free(GE.Branchs);
	if(GE.Mushs) free(GE.Mushs);
	if(GE.FMush) free(GE.FMush);
	if(GE.HMush) free(GE.HMush);
}

// Starts Timer of Current level
static void GE_StartTimer(void *Args){
	GE.Time = LE_TIME_BASE + (GE.Level - 1) * LE_TIME_STEP;

	SB_DrawTime();
	while(GE.Time>0 && GE.GameStatus==Running){
		Sleep(1000);
		while(GE.Paused) Sleep(200);//wait until game is paused
		GE.Time--;
		SB_DrawTime();
	}
}

// Draws All Objects on the Console win
static void GE_PreDraw(){
	int i, S=GE.Scale;
	system("cls");//Clear all of Screen
	for(i = 0 ; i < GE.SnakesNum ; i++)//Draws Snakes:
		if(GE.Snakes[i]->IsDead == False) SN_PreDraw(GE.Snakes[i]);
	for(i = 0 ; i < GE.MushNum + GE.PMushNum ; i++)//Draws Mushs:
		MU_Draw(GE.Mushs[i]);
	for(i = 0 ; i < GE.BranchsNum ; i++)//Draws Branchs:
		BR_PreDraw(GE.Branchs[i]);
	if(GE.HMush) MU_Draw(GE.HMush);//Draws Special Mushs:
	if(GE.FMush) MU_Draw(GE.FMush);
	if(GE.GameMode!=League) SB_PreDraw(); // Draws Status Board

	if(GE.HaveWall==False) //Draws 4 Virtual Walls around the Game Field when GE.HaveWall is False(when we haven't real walls):
		DrawVirtualWalls(GE.Offset.X,GE.Offset.Y,GE.Width * GE.Scale + 2,GE.Height * GE.Scale + 2,VER_WALL_CH,0,10);
}

static void GE_RedrawColoredObjs(){
	int i;
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	if(GE.FMush) MU_Draw(GE.FMush);
	if(GE.HMush) MU_Draw(GE.HMush);
	for(i=0;i<GE.PMushNum + GE.MushNum;i++){
		if(GE.Mushs[i]->Type==Poison) MU_Draw(GE.Mushs[i]);
	}
	for(i=0;i<GE.SnakesNum;i++){
		if(GE.Snakes[i]->IsDead==False){
			SN_PreDraw(GE.Snakes[i]);
		}
	}
	ReleaseMutex(GE.hScreenMutex);
}

// Draws a char on specified Coordinate
void GE_Draw(char Ch,short X,short Y){
	WaitForSingleObject(GE.hScreenMutex,INFINITE);
	if(GE.HaveWall)
		GotoXY(GE.Offset.X + X, GE.Offset.Y + Y);
	else//for virtual walls:
		GotoXY(GE.Offset.X + X + 1, GE.Offset.Y + Y + 1);
	putchar(Ch);
	ReleaseMutex(GE.hScreenMutex);
}

// Draws specified char on specified Scaled Coordinate 
void GE_SDraw(char Ch,short X,short Y){
	int S=GE.Scale;
	GE_Draw(Ch,S*X,S*Y);
	if(S==2){
		GE_Draw(Ch,S*X+1,S*Y);
		GE_Draw(Ch,S*X,S*Y+1);
		GE_Draw(Ch,S*X+1,S*Y+1);
	}
}

// Erases a char from specified Coordinate
void GE_UnDraw(short X,short Y){
	GE_Draw(' ',X,Y);
}

// Erases char(s) from specified Scaled Coordinate
void GE_SUnDraw(short X,short Y){
	GE_SDraw(' ',X,Y);
}

// Draws Level Number before Level start in the middle of screen
static void GE_DrawLevelNum(){
	char S[10]="Level " , res[3];
		
	strcat(S,ToStr(GE.Level,res));

	system("cls");
	GE_CheckConSize();
	DrawVirtualWalls(0,GE.Center.Y - 4,GE.ConSize.Width,9,WALL_CH,1,10);
	CenterAlignedPrint(S,0,GE.ConSize.Width,GE.ConSize.Height/2,1);
	Sleep(3500);
}

// Creates and Drops Normal & Poisen Mushrooms on the GameField
static void GE_DropMushs(int MushNum,int PMushNum){
	int i,RndX,RndY,MinX=1,MaxX=GE.Width-2,MinY=1,MaxY=GE.Height-2;
	COORD Pos;
	Bool Droped;

	GE.MushNum = GE.PriMushNum = MushNum;
	GE.PMushNum = PMushNum;
	GE.Mushs =(Mushroom**)malloc(sizeof(Mushroom*) * (MushNum + PMushNum)) ;
	GE_ManageSpecialMushs();// Call this func to reset its counter 

	// Set random position for each Mushroom:
	if(GE.HaveWall){
		MinX++;MinY++;
		MaxX--;MaxY--;
	}
	for(i=0 ; i<MushNum+PMushNum ; i++){
		Droped=False;
		while(!Droped){
			RndX=Rand(MinX,MaxX+1);
			RndY=Rand(MinY,MaxY+1);
			if(GE.GameField[RndX][RndY]==EMPTY){
				Pos.X=RndX;Pos.Y=RndY;
				if(i<MushNum)
					GE.Mushs[i]=MU_Create(Normal,Pos,i);
				else
					GE.Mushs[i]=MU_Create(Poison,Pos,i);
				Droped=True;
			}
		}
	}
}

// Creates and Drops Special Mushs like F & H Mushs to Game Field in specified times
static void GE_ManageSpecialMushs(){
	static int n;
	COORD Pos;
	Bool Droped=False;
	int RndX,RndY;
	
	if(GE.MushNum==GE.PriMushNum) n=4;//Reset n to 4 when new game round started
	if(GE.PriMushNum >= MIN_PRIMUSH_NUM && GE.MushNum>0)
		if((float)GE.MushNum / GE.PriMushNum <= (float)n / 5){
			if(n%2==0 || GE.GameMode==Special)//if n is 2 or 4 (means drop FMush) or n is 1 and 3(means drop HMush but when Game Mode is Special) 
				while(!Droped){//Now Locate an empty place on game field for new mush:
					RndX=Rand(1,(GE.Width-2) + 1);
					RndY=Rand(1,(GE.Height-2) + 1);
					if(GE.GameField[RndX][RndY]==EMPTY){
						Pos.X=RndX;Pos.Y=RndY;
						if(n%2){
							if(GE.HMush){//if previous HMush have not eaten yet remove it from screen:
								GE_SetGameField(GE.HMush->Position.X,GE.HMush->Position.Y,EMPTY,0);
								GE_SUnDraw(GE.HMush->Position.X,GE.HMush->Position.Y);
							}
							GE.HMush=MU_Create(Half,Pos,0);
							MU_Draw(GE.HMush);
							putchar('\a');
						}
						else{
							if(GE.FMush){//if previous FMush have not eaten yet remove it from screen:
								GE_SetGameField(GE.FMush->Position.X,GE.FMush->Position.Y,EMPTY,0);
								GE_SUnDraw(GE.FMush->Position.X,GE.FMush->Position.Y);
							}
							GE.FMush=MU_Create(Fast,Pos,0);
							MU_Draw(GE.FMush);
						}
						Droped=True;
					}
				}
			n--; //Now reduce n
		}
}

// Creates Branchs and Drops them on the Game Field.
static void GE_DropBranchs(Bool HaveWall){
	int i=0,BNum;
	COORD Start,End;

	GE.HaveWall=HaveWall;
	GE.BranchsNum=0;
	BNum=GE.BranchsNum + (GE.HaveWall) ? 4 : 0;
	GE.Branchs = (Branch**)malloc(sizeof(Branch*) * BNum);
	if(HaveWall){//Creates 4 Around Walls:
		Start.X=0; Start.Y=0;
		End.X=GE.Width-1; End.Y=0;
		GE.Branchs[0]=BR_Create(Start,End,Horizontal);
		
		Start.Y=1;
		End.X=0; End.Y=GE.Height-2;
		GE.Branchs[1]=BR_Create(Start,End,Vertical);
		
		Start.X=0; Start.Y=GE.Height-1;
		End.X=GE.Width-1; End.Y=GE.Height-1;
		GE.Branchs[2]=BR_Create(Start,End,Horizontal);
		
		Start.X=GE.Width-1; Start.Y=1;
		End.X=GE.Width-1; End.Y=GE.Height-2;
		GE.Branchs[3]=BR_Create(Start,End,Vertical);
		
		GE.BranchsNum+=4;
		i=4;
	}
	for(;i<BNum;i++){

	}
}

/* 
Sets elements value of GameField & GameFieldObj of GE.
P must be a pointer of your object that currently exists in X,Y position.
*/
void GE_SetGameField(int X,int Y,GameObjs Type,void *P){
	GE.GameField[X][Y]=Type;
	GE.GameFieldObj[X][Y]=P;
}

// Removes specified Mushroom from GE.Mushs & Disposes it(releases its Memory)
static void GE_RemoveMush(Mushroom *Mu){
	int i,MuI=Mu->Index;
	for(i=MuI;i<GE.MushNum + GE.PMushNum - 1;i++){//Remove From GE.Mushs and change index of next items:
		GE.Mushs[i]=GE.Mushs[i+1];
		GE.Mushs[i]->Index--;
	}
	if(Mu->Type==Normal) GE.MushNum--; else GE.PMushNum--;
	free(Mu);//Free memory of Mush
}

// Sets Game Field offset for Drawing
static void GE_SetOffset(){
	GE.Offset.X = (GE.ConSize.Width - GE.Width * GE.Scale - ((GE.GameMode!=League) ? SB_MAX_LEN+6:0)) / 2;
	GE.Offset.Y = (GE.ConSize.Height - GE.Height * GE.Scale) / 2;
	if (GE.Offset.X < 0) GE.Offset.X = 0;
	if (GE.Offset.Y < 0) GE.Offset.Y = 0;
	GE.Center.X=GE.ConSize.Width / 2;
	GE.Center.Y=GE.ConSize.Height / 2;
}

// Checks Console Width and Height and if they had been changed then Changes Drawing Offset
Bool GE_CheckConSize(){
	CONSOLE_SCREEN_BUFFER_INFO cinfo;
	Bool SetOffset=False;

	GetConsoleScreenBufferInfo(GE.ConHandle,&cinfo);//Get Console visible Area Rectangle
	if (cinfo.srWindow.Right - cinfo.srWindow.Left != GE.ConSize.Width ){
		if(GE.ConSize.Width!=-1) SetOffset=True;
		GE.ConSize.Width = cinfo.srWindow.Right - cinfo.srWindow.Left;
	}
	if (cinfo.srWindow.Bottom-cinfo.srWindow.Top != GE.ConSize.Height){
		if(GE.ConSize.Height!=-1) SetOffset=True;
		GE.ConSize.Height = cinfo.srWindow.Bottom - cinfo.srWindow.Top;
	}
	if (SetOffset){
		GE_SetOffset(); 
		if(GE.GameMode!=League) SB_SetOffset();
	}
	return SetOffset;
}