#include "GameEngine.h"

// Creates an instance of Snake Structure
Snake *SN_Create(String Name,int Num,COORD Position,int BodyLen,COORD Body[],Bool IsCom,Direction Direction){
	Snake *Sn=(Snake *)malloc(sizeof(Snake));
	int i;

	Sn->Name=(String)malloc(MAX_SN_NAME_LEN + 1);
	strcpy(Sn->Name,Name);
	Sn->Num=Num;
	Sn->Shape=1;
	Sn->ShapeCounter=0;
	Sn->Position = Position;
	Sn->Score=0;
	Sn->KillScore=0;
	Sn->KillerOf=0;
	Sn->Target=-1;
	Sn->IsDead=False;
	Sn->IsCom=IsCom;
	Sn->Direction=Direction;
	Sn->BodyLen=BodyLen;
	Sn->BodyRealLen=(BodyLen / SN_BODYLEN_STEP + 1) * SN_BODYLEN_STEP;
	Sn->Body=(COORD *)malloc(Sn->BodyRealLen * sizeof(COORD));
	for(i=0;i<BodyLen;i++)
		Sn->Body[i]=Body[i];
	
	//Enter Snake Head & Body position to GE.GameField & GE.GameFieldObj: 
	GE_SetGameField(Position.X,Position.Y,SNAKEHEAD,Sn);
	for(i=0;i<BodyLen;i++)
		GE_SetGameField(Sn->Body[i].X,Sn->Body[i].Y,SNAKEBODY,Sn);
	
	return Sn;
}

// Draws Specified Snake on the Console win
void SN_PreDraw(Snake *Sn){
	int i , BodyCh=(Sn->IsCom==True) ? COM_SNAKEBODY_CH : SNAKEBODY_CH;

	SN_DrawHead(Sn);//Draw Snake Head
	if(GE.GameMode!=League) SetFColor(Colors[GE.ColorNum-1][5],Colors[GE.ColorNum-1][4]);
	for(i=0; i < Sn->BodyLen ; i++)//Draw Snake Tail:
		GE_SDraw(BodyCh,Sn->Body[i].X,Sn->Body[i].Y);
	SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);
}

// Draws Head of Specified Snake on the Console Win
void SN_DrawHead(Snake *Sn){
	char HeadCh;
	short HX,HY;

	HX=Sn->Position.X;
	HY=Sn->Position.Y;
	if(GE.Scale==1){
		if(Sn->Shape) HeadCh=Sn->Num+'0'; else HeadCh=SNAKEHEAD_CH;
		GE_Draw(HeadCh,HX,HY);
	}else{ //Scale 2:
		if(Sn->Shape) HeadCh=Sn->Num+'0'; else HeadCh=SNAKEHEAD_S2CH;
		switch(Sn->Direction){
		case Up:
			if(Sn->Shape){
				GE_Draw('\\', 2*HX, 2*HY);
				GE_Draw('/', 2*HX + 1, 2*HY);
			}else{
				GE_Draw('/', 2*HX, 2*HY);
				GE_Draw('\\', 2*HX + 1, 2*HY);
			}
			GE_Draw(HeadCh, 2*HX, 2*HY + 1);
			GE_Draw(HeadCh, 2*HX + 1, 2*HY + 1);
			break;
		case Down:
			if(Sn->Shape){
				GE_Draw('/', 2*HX, 2*HY + 1);
				GE_Draw('\\', 2*HX + 1, 2*HY + 1);
			}else{
				GE_Draw('\\', 2*HX, 2*HY + 1);
				GE_Draw('/', 2*HX + 1, 2*HY + 1);
			}
			GE_Draw(HeadCh, 2*HX, 2*HY);
			GE_Draw(HeadCh, 2*HX + 1, 2*HY);
			break;
		case Right:
			if(Sn->Shape){
				GE_Draw('/', 2*HX + 1, 2*HY);
				GE_Draw('\\', 2*HX + 1, 2*HY + 1);
			}else{
				GE_Draw('\\', 2*HX + 1, 2*HY);
				GE_Draw('/', 2*HX + 1, 2*HY + 1);
			}
			GE_Draw(HeadCh, 2*HX, 2*HY);
			GE_Draw(HeadCh, 2*HX, 2*HY + 1);
			break;
		case Left:
			if(Sn->Shape){
				GE_Draw('\\', 2*HX, 2*HY);
				GE_Draw('/', 2*HX, 2*HY + 1);
			}else{
				GE_Draw('/', 2*HX, 2*HY);
				GE_Draw('\\', 2*HX, 2*HY + 1);
			}
			GE_Draw(HeadCh, 2*HX + 1, 2*HY);
			GE_Draw(HeadCh, 2*HX + 1, 2*HY + 1);
			break;
		}
	}
}

// Removes Specified Snake From Console Win
void SN_UnDraw(Snake *Sn){
	int i;
	GE_SUnDraw(Sn->Position.X,Sn->Position.Y);
	for(i=0;i<Sn->BodyLen;i++)
		GE_SUnDraw(Sn->Body[i].X,Sn->Body[i].Y);
}

// Moves Specified Snake
void SN_Move(Snake *Sn,MoveType MT,int NewX,int NewY){
	int i,BodyL;
	int tmp;
	COORD Tail;
	COORD *SnBody;
	Snake *NewSn;
	Direction SnDir;

	if(MT==MiniAndMove){ //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Mini and Move: &&&&&&&&&&&&&&&&
		BodyL=Sn->BodyLen;
		for(i=0;i<SNBODY_MINI_NUM && BodyL-1 - i>=SNBODY_MIN_LEN;i++){
			GE_SUnDraw(Sn->Body[BodyL-1-i].X,Sn->Body[BodyL-1-i].Y);
			GE_SetGameField(Sn->Body[BodyL-1-i].X,Sn->Body[BodyL-1-i].Y,EMPTY,0);
		}
		Sn->BodyLen-=i;
	}//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&    Move:   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

	Tail=Sn->Body[Sn->BodyLen-1];
	if(MT!=EatAndMove){
		GE_SUnDraw(Tail.X,Tail.Y);//Remove end of Snake Tail from Console win and GameField:
		GE_SetGameField(Tail.X,Tail.Y,EMPTY,0);
	}


	for(i=Sn->BodyLen-1; i>=1;i--)//Move all parts of Snake Body:
		Sn->Body[i]=Sn->Body[i-1];
	Sn->Body[0]=Sn->Position;
	GE_SetGameField(Sn->Position.X,Sn->Position.Y,SNAKEBODY,Sn);
	if(GE.GameMode!=League) SetFColor(Colors[GE.ColorNum-1][5],Colors[GE.ColorNum-1][4]);
	GE_SDraw((Sn->IsCom==True) ? COM_SNAKEBODY_CH : SNAKEBODY_CH,Sn->Position.X,Sn->Position.Y);
	SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);

	Sn->Position.X = NewX;//Set New position of snake head and draw it:
	Sn->Position.Y = NewY;
	GE_SetGameField(NewX,NewY,SNAKEHEAD,Sn);
	SN_DrawHead(Sn);

	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Eat And Move &&&&&&&&&&&&&&&&&&&&& 
	if(MT==EatAndMove){
		if(Sn->BodyLen==Sn->BodyRealLen){
			Sn->BodyRealLen+=SN_BODYLEN_STEP;
			Sn->Body=(COORD*)realloc((void*)Sn->Body,sizeof(COORD) * Sn->BodyRealLen);
		}
		Sn->Body[Sn->BodyLen]=Tail;
		Sn->BodyLen++;
	}else if(MT==HalfAndMove && Sn->BodyLen>=3){//&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Half And Move &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
		BodyL=Sn->BodyLen;// Create New Snake:
		SnBody=(COORD*)malloc(sizeof(COORD) * (BodyL-1)/2);//make Body of new Snake:
		for(i=0;i<(BodyL-1)/2;i++){
			SnBody[i]=Sn->Body[BodyL-2-i];
		}
		SnDir=GetDirection(Sn->Body[BodyL-2],Sn->Body[BodyL-1]);//Set Direction of New Snake:
		NewSn=SN_Create((GE.SnakesNum==2) ? "Com(1)":"Com(2)",GE.SnakesNum+1,Sn->Body[BodyL-1],(BodyL-1)/2,SnBody,True,SnDir);
		GE.Snakes[GE.SnakesNum]=NewSn;
		GE.SnakesNum++;
		GE.AliveSnakesNum++;
		SN_DrawHead(NewSn);//Body of New Snake have been already drawn,so only draw Head of New Snake

		Sn->BodyLen=BodyL/2;//reduce Current Snake Body Len
	}//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	
	if((tmp=(int)GE.FPS / GE.AliveSnakesNum / 2) < 2) tmp=2;//Calc & Changes Snake Head Shape:
	Sn->ShapeCounter = (++Sn->ShapeCounter) % tmp;
	if(Sn->ShapeCounter==0) Sn->Shape =!(Sn->Shape);
}

// Adds specified Score to Snake Score
void SN_AddScore(Snake *Sn,int Score){
	Sn->Score+=Score;
	SB_SNUnDrawStatus(Sn,SCORE);
	SB_SNDrawStatus(Sn,SCORE);
	SB_SNUnDrawStatus(Sn,TOTALSCORE);
	SB_SNDrawStatus(Sn,TOTALSCORE);
	SB_DrawBest();
}
