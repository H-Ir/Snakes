#include "GameEngine.h"

String MM_GetPlayerName(int PlayerNum);

// Shows Main Menu of game
int  MM_ShowMainMenu(){
	int c,res=0;

	system("cls");//first clear all screen
	system(SysColors[2]);//set back color of screen to black with white font
	SetCursorShape(True,100);

	CenterAlignedPrint("|||||||||",0,GE.ConSize.Width,0,1);
	CenterAlignedPrint("|||||||||||||||||||",0,GE.ConSize.Width,1,1);
	SetFColor(Colors[1][0],Colors[1][4]);
	CenterAlignedPrint("///////////// S N /-\\ K S \\\\\\\\\\\\\\\\\\\\\\\\\\",0,GE.ConSize.Width,2,1);
	SetFColor(Colors[1][3],Colors[1][4]);
	CenterAlignedPrint("|||||||||||||||||||",0,GE.ConSize.Width,3,1);
	CenterAlignedPrint("|||||||||",0,GE.ConSize.Width,4,1);
	CenterAlignedPrint("Developers: Hojat Irvani & Parnian Zare",0,GE.ConSize.Width,6,1);
	CenterAlignedPrint(">>>>>>>>> Main Menu <<<<<<<<<",0,GE.ConSize.Width,10,1);
	CenterAlignedPrint("Menu Options:",0,GE.ConSize.Width-16,13,1);
	CenterAlignedPrint("1) League Mode",0,GE.ConSize.Width-15,15,1);
	CenterAlignedPrint("2) Free Mode",0,GE.ConSize.Width-17,16,1);
	SetFColor(Colors[1][1],Colors[1][4]);
	CenterAlignedPrint("3) Special Mode(Career)",0,GE.ConSize.Width-6,17,1);
	SetFColor(Colors[1][2],Colors[1][4]);
	CenterAlignedPrint("4) Color Setting",0,GE.ConSize.Width-13,19,1);
	SetFColor(Colors[1][3],Colors[1][4]);
	CenterAlignedPrint("5) Exit",0,GE.ConSize.Width-22,21,1);
	CenterAlignedPrint("Please Enter Your Selected Mode Number(1 to 5 ?)... ",0,GE.ConSize.Width,24,1);
	ClearKBBuffer();//clear buffer
	while(!((c=getch())>='1' && c<='5'));
	putchar(c);
	Sleep(750);
	system("cls");
	if(c=='5'){
		CPrint("\n\nExiting Game...");Sleep(1500);
		return 0;//Exit whole game
	}else{
		if(c=='1'){
			CPrint("\n\nLeague Mode:\nGame starts in 2 Sec...");
			Sleep(2000);	
		}else if(c=='2')
			CPrint("\n\nFree Mode:");
		else if(c=='3')
			CPrint("\n\nSpecial Mode:");
		return c-'0';
	}
}

// Shows Free Mode Config menu Screen
void MM_ShowConfigMenu(GameMode GameM,short *Width,short *Height,int *Scale,float *FPS,int *SnakesNum,
				   int *PlayersNum,int *MushNum,int *PMushNum,Difficulty *Diffic,Bool *HaveWall,String PlayerNames[]){
	int c,MaxW,MaxH,i,MaxMN;
	char Line[11];
	if(GameM==Special){
		*Width=LE_WIDTH_MAX; *Height=LE_HEIGHT_MAX;
		*MushNum=LE_MUSH_MAX_NUM; *PMushNum=LE_PMUSH_MIN_NUM;
		*FPS=LE_FPS_MIN;
		PlayerNames[0]=MM_GetPlayerName(0);
	}else{
		printf("\n\n1) Get Scale(1 or 2): ");
		ClearKBBuffer();
		while((c=getch())!='1' && c!='2');
		putchar(c);
		*Scale=c-'0';

		printf("\n\n\n2) Get FPS(2 to 300): ");
		ClearKBBuffer();
		*FPS=0;
		GetLine(Line,10);
		sscanf(Line,"%f",FPS);
		while(!(*FPS >=2 && *FPS<=300)){
			printf("\n    Entered Number is not valid.Please get another FPS value: ");
			GetLine(Line,3);
			sscanf(Line,"%f",FPS);
		}

		printf("\n\n3) Get Number of Snakes(2 to 4) and Number of Players(1 or 2)(e.g 3 1): ");
		*SnakesNum = *PlayersNum = 0;
		GetLine(Line,10);
		sscanf(Line,"%d %d",SnakesNum,PlayersNum);
		while(!(*SnakesNum>=2 && *SnakesNum<=4 && *PlayersNum>=1 && *PlayersNum<=2)){
			printf("\n    Entered numbers are not valid.Please Get other numbers: ");
			GetLine(Line,10);
			sscanf(Line,"%d %d",SnakesNum,PlayersNum);
		}

		for(i=1; i <= *PlayersNum ; i++)
			PlayerNames[i-1] = MM_GetPlayerName(i);

		printf("\n\n4) Do you want to have wall around game field?(y/n) ");
		ClearKBBuffer();
		while((c=tolower(getch()))!='n' && c!='y');
		putchar(c);
		if(c=='n') *HaveWall=False; else *HaveWall=True;

		MaxW=150 / *Scale; MaxH=76  / *Scale;
		printf("\n\n\n5) Width and Height of Game Field:");
		printf("\n\n       -Min possible dimension is 12x12");
		printf("\n       -Max possible dimension is %dx%d",MaxW,MaxH);
		printf("\n       -Max possible dimension for 1024x768 resolution is %dx%d",
			  ((*HaveWall) ? 98 / *Scale : 96 / *Scale),((*HaveWall) ? 56 / *Scale : 54 / *Scale));
		printf("\n\n   Now Get Width and Height of game field(e.g 30 20): ");
		ClearKBBuffer();
		*Width = 0; *Height = 0;
		GetLine(Line,10);
		sscanf(Line,"%hd %hd",Width,Height);
		while(!(*Width >=12 && *Height >=12 && *Width <= MaxW && *Height <= MaxH)){
			printf("\n    Entered dimension isn't valid.Please get other values: ");
			GetLine(Line,10);
			sscanf(Line,"%hd %hd",Width,Height);
		}

		*MushNum = *PMushNum = 0;
		printf("\n\n6) Do you want to have Mushrooms in the game field?(y/n) ");
		ClearKBBuffer();
		while((c=tolower(getch()))!='n' && c!='y');
		putchar(c);
		if(c == 'y'){
			*MushNum = *PMushNum = -2;
			MaxMN = (*Width - ((*HaveWall) ? 4:2)) * (*Height - ((*HaveWall) ? 4:2)) * 3 / 4;
			if(MaxMN < MIN_PRIMUSH_NUM) 
				MaxMN=MIN_PRIMUSH_NUM;
			printf("\n\n       -Min number of Normal and Poison Mushrooms are %d & 0",MIN_PRIMUSH_NUM);
			printf("\n       -Sum of two numbers must be less than %d",MaxMN);
			printf("\n       -If you want to have proper number of Poison Mushrooms you can \n       get -1 as number of Poison Mushrooms");
			printf("\n\n   Now Get Number of Normal and Poison \n   Mushrooms(e.g 40 10 or 60 -1 or 20 0): ");
			GetLine(Line,10);
			sscanf(Line,"%d %d",MushNum,PMushNum);
			if(*PMushNum == -1)
				*PMushNum =(int) (*MushNum * PMUSH_NUM_PRO);
			while(!(*MushNum>=MIN_PRIMUSH_NUM && *PMushNum >= 0 && *MushNum + *PMushNum <= MaxMN)){
				printf("\n     Entered numbers are not valid.Please Get other numbers: ");
				GetLine(Line,10);
				sscanf(Line,"%d %d",MushNum,PMushNum);
				if(*PMushNum == -1)
					*PMushNum =(int) (*MushNum * PMUSH_NUM_PRO);
			}
			
		}
	}

	printf("\n\nDifficulty:");
	printf("\n\n\t   1)Funny    2)Easy    3)Normal    4)Hard    5)VeryHard");
	printf("\n\n\tPlease get difficulty number(1 to 5): ");
	ClearKBBuffer();
	while((c=getch())<'1' || c>'5');
	putchar(c);
	*Diffic=(Difficulty)(5-(c-'0'));

	CPrint("\n\nGame starts in 2 Sec...");
	Sleep(2000);
}

// Shows Color Menu to user
void MM_ShowColorMenu(){
	int Num=MAX_COLOR_NUM - 1,c,CurColor;
	
	system("cls");
	CPrint("\n\tColor Setting:");
	printf("\n\nIn order to find Your favorite color please try possible colors.");
	printf("\nSo,Please press any number you want from 1 to "); printf("%d",Num); printf(" Several times to find it.");
	CPrint("\n\n  Press ESC to Cancel and return to Main Menu  \n  Press Enter to apply new Color  \n  Press 'D' to set Default Color");
	CPrint("\n\n\nNow press your ideal color number(1 to "); printf("%d",Num); CPrint(")? ");
	
	ClearKBBuffer();//Clear Keyboard Buffer
	CurColor=GE.ColorNum;
	while((c=getch())!=ENTER_KEY){
		if(c==ESC_KEY){
			GE.ColorNum=CurColor;
			SetConColors();
			break;
		}else if(tolower(c)=='d')
			GE.ColorNum=1;
		else if(c>='1' && c<='0' + Num)
			GE.ColorNum=c-'0';
		else
			continue;
		SetConColors();
		putchar(c); putchar(' ');
	}
	CPrint("\n\n Your Selected Color Number is : ");printf("%d...",GE.ColorNum);
	Sleep(2000);//Wait for User to see result
}

// Gets a Name from user for Specified Player and returns it
static String MM_GetPlayerName(int PlayerNum){
	String PName=(String)malloc(sizeof(char)*(MAX_SN_NAME_LEN + 1));
	CPrint("\n\nEnter Name Of Player");if(PlayerNum) printf(" %d (Max %d char): ",PlayerNum,MAX_SN_NAME_LEN); else printf("(Max %d char): ",MAX_SN_NAME_LEN);
	ClearKBBuffer();//Clear Keyboard Buffer
	do
		GetLine(PName,MAX_SN_NAME_LEN);
	while(strlen(PName)<=0);
	return PName;
}
