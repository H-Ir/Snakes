#define SNBODY_MINI_NUM 3 //Number of Snake Body that must be remove when it eats PMush
#define MAX_SN_NAME_LEN 15 //Snake Name max len
#define SN_BODYLEN_STEP 200 //Snake BodyRealLen increasing Step
#define SNBODY_MIN_LEN 1 //Minimum Number of Snake Body

#define DEATH_MSG_REAPET_NUM 3 //Number of red flashs that must be shown when an snake was killed.
#define DEATH_MSG_INTERVAL 100 //Interval Between each Red Flash of Death message

Snake *SN_Create(String Name,int Num,COORD Position,int BodyLen,COORD Body[],Bool IsCom,Direction Direction);//Snake Function:
void SN_PreDraw(Snake *Sn);
void SN_DrawHead(Snake *Sn);
void SN_UnDraw(Snake *Sn);
void SN_Move(Snake *Sn,MoveType MT,int NewX,int NewY);
void SN_AddScore(Snake *Sn,int Score);
