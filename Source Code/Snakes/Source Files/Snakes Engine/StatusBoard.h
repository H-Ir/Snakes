#define SB_MAX_LEN 20 //Max Char number of Status Board in width 
#define SB_EACH_SN_LINE_NUM 8 //Number of Lines that each Snake need for showing it's status in Status Board(SB)
#define SB_HEADER_LINE_NUM 10 //Number of Lines that will be writen as header of Status Board like Level number,Level Timer, ...

#define SB_MODE_LINE_NUM 2 //Line number of 'Mode' item of Status Board 
#define SB_LEVEL_LINE_NUM 3 //Line number of 'Level' item of Status Board 
#define SB_FPS_LINE_NUM 4 //Line number of 'FPS' item of Status Board 
#define SB_TIME_LINE_NUM 5 //Line number of 'Time' item of Status Board 
#define SB_BEST_LINE_NUM 6 //Line number of 'Best' item of Status Board 

void SB_Draw(char Ch,short X,short Y);
void SB_SetOffset();
void SB_DrawBest();
void SB_DrawTime();
void SB_CalcAndDrawFPS();
void SB_SNDrawStatus(Snake *Sn,StatusType Type);
void SB_SNUnDrawStatus(Snake *Sn,StatusType Type);
