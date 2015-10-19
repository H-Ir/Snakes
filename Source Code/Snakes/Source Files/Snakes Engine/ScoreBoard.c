#include "GameEngine.h"

#define SCB_TITLE_LN 7 //Score board Title Line Number
#define SCB_ROUND_INF_LN 2 //Score Board Round information Line Number
#define SCB_HEADER_LN 8 //Score Board Header Line Number
#define SCB_LIST_LN 15 //Score Board List Line Number
#define SCB_FINAL_LN 6 //Score Board Final Scores List Line Number
#define SCB_PRESSANYKEY_LN 9 //Score Board Press any Key Line Number
#define SCB_FREEM_LN (SCB_TITLE_LN + SCB_ROUND_INF_LN + SCB_HEADER_LN + SCB_LIST_LN + SCB_PRESSANYKEY_LN + 1) //whole Score Board Line Number in Free Mode
#define SCB_SPECIALM_LN (SCB_FREEM_LN - 1 + SCB_FINAL_LN) //whole Score Board Line Number in Special Mode
#define SCB_NAME_LEN (MAX_SN_NAME_LEN + 3) // Score Board Name column width
#define SCB_SCORES_LEN 15 // Score Board Score,Kill Score,Total Score,Highest Score and 'Killer Of' columns width
#define SCB_STATUS_LEN 10 // Score Board Status column width
#define SCB_FINAL_ITEMS_LEN 20 //Score Board Final List Items Width
#define SCB_FINAL_ITEMS_DIS 2  //Score Board Final List Items distances

// Draws and Shows Score Board after each Level and returns 1 if game is in 
int SB_ShowScoreBoard(){
	int i,j;
	int x,y,cx,cy,XO,YO,XO2;
	int len;
	char tmp[100];
	Snake *winner,*Sn;
	int Bests[4];
	int SCB_Len;
	int BNum;
	int t,ty; //tmp

	if((i=GE.Offset.Y-1)<0) i=0;// first clear screen by effect:
	for( ; i < GE.Offset.Y + GE.Height * GE.Scale + ((GE.HaveWall) ? 0:2) ; i++){//Clear Game Field:
		GotoXY(GE.Offset.X , i);
		for(j=0 ; j < GE.Width * GE.Scale + ((GE.HaveWall) ? 0:2); j++)
			putchar(' ');
		Sleep(10);
	}
	for(i=0;i<GE.ConSize.Height;i++){//Clear Status Board:
		GotoXY(GE.SBOffset.X - 2,i);
		for(j=0;j<SB_MAX_LEN + 4;j++)
			putchar(' ');
		Sleep(10);
	}
	
	system(SysColors[2]);//Set black color to Screen
	GE_CheckConSize();
	winner = GE_GetWinner();
	cx = GE.ConSize.Width / 2;
	cy = GE.ConSize.Height / 2;
	if(GE.GameMode==League){// Special Mode when the game ends and League Mode :
ShowTheEnd:
		len=0;
		if(GE.GameMode==League){
			strcpy(tmp,"Game Winner: ");
			len = strlen(winner->Name);
		}else
			strcpy(tmp,"The End");
		len += strlen(tmp) ;
		x = cx - len / 2; 
		y = cy;
		if(x<0) x=0;

		for(i=0 ; i<y-4 ; i++){// Draw Horizontal Lines
			DrawVirtualWalls(0,i,GE.ConSize.Width,GE.ConSize.Height - i*2,HOR_WALL_CH,1,0);
			Sleep(1);
		}
		DrawVirtualWalls(0,i,GE.ConSize.Width,GE.ConSize.Height - i*2,WALL_CH,1,10);
		
		GotoXY(x,y);
		CPrint(tmp);
		if(GE.GameMode==League) CPrint(winner->Name);

		for(i=0 ; i<y-4 ; i++){
			DrawVirtualWalls(0,i,GE.ConSize.Width,GE.ConSize.Height - i*2,' ',1,0);
			Sleep(1);
		}
		// Draw "XHD" 1000 time on rnd positions of screen when game ends
		if(GE.GameMode==Special)
			strcpy(tmp,"   >>> Developer & Owner: XHD Forever.All Rights Reserved <<<   ");
		else
			strcpy(tmp,winner->Name);
	    for(i=0; i<2000 ; i++){
			x=Rand(0,GE.ConSize.Width - strlen(tmp));
			do
				y=Rand(0,GE.ConSize.Height);
			while(y >= cy-4 && y <= cy+4);
			GotoXY(x,y);
			printf(tmp);
			Sleep(1);
		}

		ClearKBBuffer();
		getch();// wait for user till see all score board info
		return 0;
	}else{// Special and Free Mode:
		GE_GetBestSnakes(&BNum,Bests,False);// Get Best Snakes
		SCB_Len = SCB_NAME_LEN + SCB_SCORES_LEN * 5 + SCB_STATUS_LEN + 2;

		// first Set X offset and Y offset for drawing ScoreBoard:
		if((XO = (GE.ConSize.Width - SCB_Len) / 2) < 0) XO=0;
		if(GE.GameMode==Free)
			YO = (GE.ConSize.Height - SCB_FREEM_LN) / 2;
		else// Special Mode:
			YO = (GE.ConSize.Height - SCB_SPECIALM_LN) / 2;
		if(YO<0) YO=0;

		x=XO; y=YO + SCB_TITLE_LN;

		SetFColor(Colors[1][0],Colors[1][4]);
		GotoXY(x,y);//Draw Level Number:
		CPrint("Level:   "); 
		if(GE.GameMode==Special) printf("%d/%d",GE.Level,MAX_LEVEL_NUM); else CPrint("None");
		SetFColor(Colors[1][3],Colors[1][4]);

		y += SCB_ROUND_INF_LN;

		DrawVirtualWalls(XO,y,SCB_Len,SCB_LIST_LN + SCB_HEADER_LN + 1,WALL_CH,0,10);//Draw 4 Lines around Score Board List:
		CenterAlignedPrint("Score Board",0,GE.ConSize.Width,y+1,1);// Score Board Header
		CenterAlignedPrint("///////////////",0,GE.ConSize.Width,y+2,1);
		
		//Now Print List Columns Name:
		x=XO + 1;
		CenterAlignedPrint("Name",x,SCB_NAME_LEN,y+5,1);
		x += SCB_NAME_LEN; CenterAlignedPrint("Score",x,SCB_SCORES_LEN,y+5,1);
		x += SCB_SCORES_LEN; CenterAlignedPrint("Kill Score",x,SCB_SCORES_LEN,y+5,1);
		x += SCB_SCORES_LEN; CenterAlignedPrint("Total Score",x,SCB_SCORES_LEN,y+5,1);
		x += SCB_SCORES_LEN; CenterAlignedPrint("Highest Score",x,SCB_SCORES_LEN,y+5,1);
		x += SCB_SCORES_LEN; CenterAlignedPrint("Killer Of",x,SCB_SCORES_LEN,y+5,1);
		x += SCB_SCORES_LEN; CenterAlignedPrint("Status",x,SCB_STATUS_LEN,y+5,1);

		GotoXY(XO+1,y+6);
		for(i=0 ; i < SCB_Len - 2 ; i++)
			putchar(HOR_WALL_CH);

		y += SCB_HEADER_LN;

		for(i=0 ; i < GE.SnakesNum ; i++){// Draw Snakes Scores List and their status:
			x = XO + 1;
			Sn=GE.Snakes[i];
			CenterAlignedPrint(Sn->Name,x,SCB_NAME_LEN,y+i*2,1);//Draw Name
			x += SCB_NAME_LEN; CenterAlignedPrint(ToStr(Sn->Score,tmp),x,SCB_SCORES_LEN,y+i*2,1);//Draw Score
			x += SCB_SCORES_LEN; CenterAlignedPrint(ToStr(Sn->KillScore,tmp),x,SCB_SCORES_LEN,y+i*2,1);//Draw Kill Score
			SetFColor(Colors[1][0],Colors[1][4]);
			x += SCB_SCORES_LEN; CenterAlignedPrint(ToStr(Sn->Score + Sn->KillScore,tmp),x,SCB_SCORES_LEN,y+i*2,1);//Draw Total Score
			SetFColor(Colors[1][3],Colors[1][4]);
			x += SCB_SCORES_LEN;
			for(j=0 ; j < BNum; j++){
				if(Bests[j]==i){
					CenterAlignedPrint("X",x,SCB_SCORES_LEN,y+i*2,1);// Draw Highest Score
					break;
				}
			}
			x += SCB_SCORES_LEN; CenterAlignedPrint(ToStr(Sn->KillerOf,tmp),x,SCB_SCORES_LEN,y+i*2,1);// Draw Status
			x += SCB_SCORES_LEN; CenterAlignedPrint(((Sn->IsDead) ? "Dead":"Alive"),x,SCB_STATUS_LEN,y+i*2,1);// Draw Status
		}

		ty = YO + SCB_TITLE_LN + SCB_ROUND_INF_LN;
		DrawVirtualWalls(0,ty,XO,6,WALL_CH,1,10);//Draw Hands of Robot:
		DrawVirtualWalls(0,ty+1,XO+1,4,' ',2,10);
		DrawVirtualWalls(XO+SCB_Len-9,0,9,ty,WALL_CH,2,10);
		DrawVirtualWalls(XO+SCB_Len-8,0,7,ty+1,' ',1,10);

		y += SCB_LIST_LN + 1;

		if(GE.GameMode==Special){
			
			PlayerScoreSum += GE.Snakes[0]->Score;//Increase Player Final Scores:
			PlayerKScoreSum += GE.Snakes[0]->KillScore;

			SCB_Len = SCB_FINAL_ITEMS_LEN * 3 + SCB_FINAL_ITEMS_DIS * 2 + 6;// Calc Width of Final List with its Around Lines:
			if((XO2 = (GE.ConSize.Width - SCB_Len) / 2) < 0) XO2=0;
			
			t = SCB_FINAL_LN + SCB_PRESSANYKEY_LN - 3;
			DrawVirtualWalls(XO2 , y-1 , SCB_Len , t , WALL_CH , 0,10);//Draw 4 Lines around Score Board Final List
			t += y-1;
			DrawVirtualWalls(cx-26 , t , 53 , min(GE.ConSize.Height - t,10) , WALL_CH , 2,10); // Draw 2 foot for scoreboard:
			DrawVirtualWalls(cx-32 , t , 64 , min(GE.ConSize.Height - t,10) , WALL_CH , 2,10); 
			CenterAlignedPrint("Final Scores" , 0 , GE.ConSize.Width , y , 1);// Final Scores List Header
			CenterAlignedPrint("////////////////" , 0 , GE.ConSize.Width , y+1 , 1);

			y += SCB_FINAL_LN - 1;
			x = XO2 + 3;// Print Final Scores:
			GotoXY(x,y); CPrint("Score:");
			GotoXY(x + SCB_FINAL_ITEMS_LEN + SCB_FINAL_ITEMS_DIS , y); CPrint("Kill Score:");
			GotoXY(x + 2 * SCB_FINAL_ITEMS_LEN + SCB_FINAL_ITEMS_DIS * 2 , y); CPrint("Total Score:");
			SetFColor(Colors[1][0],Colors[1][4]);
			RightAlignedPrint(ToStr(PlayerScoreSum,tmp) , (x + SCB_FINAL_ITEMS_LEN - 1) , y , 1);
			RightAlignedPrint(ToStr(PlayerKScoreSum,tmp) , (x + 2 * SCB_FINAL_ITEMS_LEN + SCB_FINAL_ITEMS_DIS - 1) , y , 1);
			RightAlignedPrint(ToStr(PlayerScoreSum + PlayerKScoreSum,tmp) , (x + 3 * SCB_FINAL_ITEMS_LEN + 2 * SCB_FINAL_ITEMS_DIS - 1) , y , 1);
			SetFColor(Colors[1][3],Colors[1][4]);
		}else{
			DrawVirtualWalls(cx-26 , y-1 , 53 , min(GE.ConSize.Height - y + 1,19) , WALL_CH , 2,10); // Draw 2 foot for scoreboard:
			DrawVirtualWalls(cx-32 , y-1 , 64 , min(GE.ConSize.Height - y + 1,19) , WALL_CH , 2,10); 
		}
		
		y += SCB_PRESSANYKEY_LN;
		
		SetFColor(Colors[1][2],Colors[1][4]);
		if(GE.GameStatus == GoNextLevel)
			strcpy(tmp,"Press Any Key to Go Next Level...");
		else if(GE.GameMode == Free){
			strcpy(tmp,"Press any other key for going to Main Menu...");
			CenterAlignedPrint("Press R if you want to play again",0,GE.ConSize.Width,y++,1);
		}else
			strcpy(tmp,"Press any key to go Main Menu...");
		CenterAlignedPrint(tmp,0,GE.ConSize.Width,y,1);
		SetFColor(Colors[1][3],Colors[1][4]);

		y = YO;

		if(GE.GameMode==Special)
			if(winner->Num==1){// if Player is winner:
				strcpy(tmp,"0 You Won");
				SetFColor(Colors[1][2],Colors[1][4]);
			}else{
				strcpy(tmp,"0 Game Over");
				SetFColor(Colors[1][0],Colors[1][4]);
			}
		else{// Free Mode
			strcpy(tmp,"0 Game Winner: "); 
			strcat(tmp,winner->Name);
			SetFColor(Colors[1][2],Colors[1][4]);
		}
		//Now Print the Title of Score Board:
		DrawVirtualWalls(cx - 6 , y + 5 , 13 , SCB_TITLE_LN - 5 + SCB_ROUND_INF_LN , WALL_CH , 2,10);//Draw Around Walls of Title:
		DrawVirtualWalls(cx - 20 , y , 41 , 5 , WALL_CH , 1,10);
		strcat(tmp," 0");
		CenterAlignedPrint(tmp , 0 , GE.ConSize.Width , y+2 , 1);//Now Print Title text
		SetFColor(Colors[1][3],Colors[1][4]);

		ClearKBBuffer();
		t=getch();// wait for user till see all score board info
		if(GE.GameStatus==GameEnded && GE.Level==MAX_LEVEL_NUM){
			system("cls");
			goto ShowTheEnd;
		}

		if(tolower(t)=='r') return 1; else return 0; //if R have pressed then return 1 for playing again
	}
}
