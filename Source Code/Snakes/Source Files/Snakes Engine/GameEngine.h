#include "stdio.h"
#include "stdlib.h"
#include "Conio.h"
#include "windows.h"
#include "string.h"
#include "process.h"
#include "time.h"
#include "limits.h"

#include "SE_Types.h"
#include "AI.h"
#include "Branch.h"
#include "Mushroom.h"
#include "OtherF.h"
#include "ScoreBoard.h"
#include "Snake.h"
#include "StatusBoard.h"
#include "MainMenu.h"

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Definitions & Types: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#define CONSOLE_TITLE "S N /-\\ K E S  By H.Doulabi & P.Zare"

#define SNAKEBODY_CH 'N'
#define COM_SNAKEBODY_CH '8'
#define SNAKEHEAD_CH 1
#define SNAKEHEAD_S2CH '*'
#define MUSH_CH 3
#define PMUSH_CH 15
#define FMUSH_CH1 'F'
#define FMUSH_CH2 6
#define HMUSH_CH1 'H'
#define HMUSH_CH2 21
#define BRANCH_CH 186
#define HOR_WALL_CH 196
#define VER_WALL_CH 197
#define WALL_CH 186
#define SPLIT_LINE_CH 16
#define SPLIT_LINE_CH2 17

#define MAX_LEVEL_NUM 10 //Max Level Number of Special mode

#define PMUSH_NUM_PRO (1/6.0f) //Poison Mushroom Number Proportion only in Free Mode
#define MIN_PRIMUSH_NUM 10 //Min Pri Number of Normal Mushs

#define ENTER_KEY 13 //Ascii codes:
#define ESC_KEY 27 

// Special Mode Defines(related to levels):
#define LE_FPS_MIN 10.0f //FPS Base value for first level
#define LE_FPS_MAX 30.0f //FPS Max value that is used in last Level
#define LE_TIME_BASE 25 //Time Base Value for first level
#define LE_TIME_STEP 2 //Time increasing Value for having different Time in each level
#define LE_HEIGHT_MAX 28
#define LE_WIDTH_MAX 49
#define LE_HEIGHT_MIN 12
#define LE_WIDTH_MIN (int)(LE_HEIGHT_MIN * 1.8f)
#define LE_MUSH_MAX_NUM 50
#define LE_MUSH_MIN_NUM ((LE_WIDTH_MIN - 4)*(LE_HEIGHT_MIN - 4)/10)
#define LE_PMUSH_MAX_NUM 20
#define LE_PMUSH_MIN_NUM LE_MUSH_MIN_NUM

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Functions Declaration: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void GE_LaunchManageGame();
void GE_Prepare();
void GE_Close();

Snake *GE_GetWinner();
void GE_GetBestSnakes(int *Num,int Bests[],Bool BetweenAliveSN);
void GE_Draw(char Ch,short X,short Y);
void GE_SDraw(char Ch,short X,short Y);
void GE_UnDraw(short X,short Y);
void GE_SUnDraw(short X,short Y);
void GE_SetGameField(int X,int Y,GameObjs Type,void *P);
Bool GE_CheckConSize();

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Global Variables: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

extern GameEngine GE;
extern int PlayerScoreSum,PlayerKScoreSum;
extern char *SysColors[];
extern int Colors[][6];
