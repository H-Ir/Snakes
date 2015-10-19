typedef char *String;

typedef enum _bool{False,
				   True} Bool;
typedef enum _GameObjs{SNAKEBODY=-20,
					   SNAKEHEAD=-15,
					   BRANCH=-10,
					   PMUSH=-5,
					   EMPTY=0,
					   MUSH=5,
					   FMUSH=15,
					   HMUSH=20} GameObjs;
typedef enum _GameMode{League=1,
					   Free,
					   Special} GameMode;
typedef enum _GameStatus{Running,
						 Quitted,
						 GoNextLevel,
						 GameEnded,
						 GameOver} GameStatus;
typedef enum _Orientation{Horizontal,
						  Vertical} Orientation;
typedef enum _Direction{Up,
						Down,
						Left,
						Right} Direction;
typedef enum _MoveType{Move,
					   EatAndMove,
					   MiniAndMove,
					   HalfAndMove} MoveType;
typedef enum _StatusType{SCORE,
						 KILLSCORE,
						 TOTALSCORE,
						 KILLEROF,
						 STATUS} StatusType;
typedef enum _MushType{Normal,
					   Poison,
					   Half,
					   Fast} MushType;
typedef enum _Difficulty{VeryHard,
						 Hard,
						 Norm,
						 Easy,
						 Funny} Difficulty;
						

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Structures: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

typedef struct _Size{
	int Width;
	int Height;
} Size;

typedef struct _Mushroom{
	MushType Type;
	COORD Position;
	int Shape;
	int Index;
} Mushroom;

typedef struct _Branch{
	Orientation Orient;
	int Len;
	COORD Start,End;
} Branch;

typedef struct _Snake{
	String Name;
	int Num;//Snake Index
	int Shape,ShapeCounter;
	int KillerOf;
	Direction Direction;
	COORD Position;
	int Target;//Target Snake that this snake wants to kill it
	Bool IsCom;
	Bool IsDead;
	int Score,KillScore;
	int BodyLen,BodyRealLen;
	COORD *Body;
} Snake;


typedef struct _GameEngine{
	short Width,Height;
	int Scale;
	float FPS; 
	int ColorNum;//Back & Fore Color index
	GameMode GameMode;
	HANDLE ConHandle;
	HANDLE hScreenMutex;// A mutex for avoiding sync use of console win.
	Size ConSize;
	COORD Offset; //Offset of Game field
	COORD SBOffset;//Status Board Offset
	COORD Center; //Center of the Screen
	unsigned long FrameCounter;
	GameStatus GameStatus;//0-> Game is runnig,1->Quit(goto main menu),2->NextLevel(only in Special mode,show score board & Go next level),3->Game End(in league and Free mode,Show Score board & Go Main menu)
	Bool Paused;
	unsigned int HighSpeed_RTime; // High Speed(FPS) Remaning Time(or remainig Frame)
	Bool HaveWall;
	Bool CanEndGame;
	int Inning;
	int Level;
	Difficulty Diffic;
	int Time;
	int SnakesNum,AliveSnakesNum,BranchsNum;
	int MushNum,PMushNum,PriMushNum;//Normal Mushroom Primary number 
	GameObjs **GameField;
	void ***GameFieldObj;
	Snake *Snakes[4];
	Branch **Branchs;
	Mushroom *FMush;
	Mushroom *HMush;
	Mushroom **Mushs;
} GameEngine;
