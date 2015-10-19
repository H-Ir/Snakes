#include "GameEngine.h"
#include <stdarg.h>
#include <crtdbg.h>

#define ATT_APEAK_DIR_BCOE 100 // Base coefficient of Apeak Dir for choosing Probability
#define ATT_CONV_DIR_BCOE (100-ATT_APEAK_DIR_BCOE)//Base Coefficient of Converse Dir for choosing Probability

// Scores of different possible ways of moving(Phase 1):
#define PH1_FIR_SCR 100000//Score of Best Way for to reach the most Mushs and Scores(First Way)(in Phase 1)
#define PH1_SEC_SCR 95000// Scores of Second , Third and Fourth best ways for to reach the most Mushs and Scores(in Phase 1):
#define PH1_THI_SCR 90000
#define PH1_FOU_SCR 85000

#define PMUSH_SCR (-25000) 
#define FMUSH_SCR 1000000
#define HMUSH_SCR 2000000
#define MUSH_SCR_COE 10
#define PMUSH_SCR_COE -0

#define PH2_SCR 5000
#define TAR_SN_HEAD_SCR 100000

// Scores of different possible ways of moving(Phase 3):
#define PH3_FIR_SCR 100000 //Score of Best Way that is chosen by probability & Random Numbers that Can be one of Apeak or Converse or Same Directions( in Attack phase(phase 3) )
#define PH3_SEC_SCR 99900  //Score of Way that not chosen in probability( in Attack phase(phase 3) )
#define PH3_FAR_APEAK_SCR 85000 //Score of Far Apeak Direction( in Attack phase(phase 3) )
#define PH3_LAST_DIR_SCR 75000 //when we haven't wall is Score of Same Direction and we have wall maybe score of Same Dir or Converse Dir( in Attack phase(phase 3) )

#define ILLEGAL_DIR_SCR (INT_MIN/2) //Score of Illegal Direction.Illegal Direction is Dir that contains body[0] of Sn
#define OBSTACLE_SCR (INT_MIN/8) //Score of Points that contains an Obstacle.it means that contains Snake Head or Snake Body or Branch
#define ENEMY_SN_BODY_HEAD_SCR (INT_MIN / 4)

#define TRAP_T1_RECLEVEL 8 //Max Recursive Level for Trap Checking of Type 1
#define TRAP_T2_RECLEVEL 15 //Max Recursive Level for Trap Checking of Type 2
#define TAR_SN_MAXDIS 4//Max Distance of Target Snake.if all snakes of game have more distance with our Sn then we don't consider them for Trap checking

#define SQ_SN_DIS_COE 2 //Square Traps: Coe of Snakes Max Distance to Sn
#define SQ_WALL_DIS 4 //Square Traps: Walls Max Distance to Sn

#define Orient(Direct) (((Direct) <= 1) ? Vertical:Horizontal)

typedef struct _Trap_Parameters{
	COORD *Sn;// Coord of all Sn Body 
	COORD *TarSn; // Coord of all Target Sn Body
	Bool TarSnIsDead;
	Bool IsBackTrack;
	int SnL,TarSnL; //len of two previous arrays
	signed char **GField; //simulated GameField 
	short Width,Height;
	int Reclevel;
	int T1RecMaxLevel,T2RecMaxLevel;
	int IllegalDir,TarSn_IllegalDir;
} Trap_Param;


void CheckTraps(Snake *Sn,int DirS[],int Phase,int IllegalDir);
int Rec_TrapsScore(Trap_Param *TP);
int Rec_TrapsScoreT2(Trap_Param *TP);
void CalcMushScr(Snake* Sn,Mushroom* Mu,float DirMushScr[4][2]);
COORD GetFirstObstacle(short StartX,short StartY,Direction MoveDir,Trap_Param *TP,int *ObsCode);
int MinDisofObjs(Snake* Sn,int Obj2);
int GetNearSnakes(COORD P,Snake *OurSn,int MaxDis,Snake *Res[],int SnakesDis[]);

int MNum(int,int (*)(int,int),...);
int MNumA(int[],int,int (*)(int,int),int*);

int IsLess(int a,int b);
int IsMore(int a,int b);

int Dir_Dis(Direction Dir,COORD P1,COORD P2);
int HorDis(COORD p1,COORD p2);
int VerDis(COORD p1,COORD p2);
int Dis(COORD p1,COORD p2);

void GetNearWays(COORD Offset,COORD Dest,Direction *VerDir,Direction *HorDir);
void GetNearWays_SS(Snake *Sn,Snake *TarSn,Direction *VerDir,Direction *HorDir);
void GetNearWays_SC(Snake *Sn,COORD Dest,Direction *VerDir,Direction *HorDir);

Direction AdverseDir(Direction Dir);
int ApeakMoveDir(Snake *Sn,Snake *TarSn,Direction Res[]);
Direction *GetApeak(Direction Dir,Direction[]);

GameObjs GetObjTypeOfLoc(COORD p,Direction Dir);
void GetNewPos(short X,short Y,int Dir,short *NX,short *NY);

void ReportVirStat(Trap_Param *TP);
void ReportVirStat1(Trap_Param *TP);
void ReportGameField(Trap_Param *TP);

int DirDiff[4][2]={
				{0,-1},{0,1}, //Up , Down
				{-1,0},{1,0} //Left , Right
			   };


// Main Function of AI.Only this Function is not static and can be seen from outside.Chooses Best Possible Direction for Specified Com Snake.
void SetComSnakeDirection(Snake *Sn){
	int DirS[]={0,0,0,0};// Scores of all Directions that Current Snake can Move
	int Phase;
	int i,j;
	int found;
	
	float DirMushScr[][2]={{0,0},{0,1},{0,2},{0,3}}; //[][0] -> Score , [][1] -> Direction
	float t;
	int FinalMushDirS[4]={PH1_FIR_SCR,PH1_SEC_SCR,
						  PH1_THI_SCR,PH1_FOU_SCR};

	Direction HorDir,VerDir;

	Snake *TarSn;
	Direction IllegalDir,ApeakDirs[2],ConvDir,SameDir;// in this line & all other lines of this func, 'Same' is related to TarSn Directions and Orientations
	int ApeakDirDis,ConvDirDis,SameDirDis;//Distances between heads of two snakes when Sn go by Apeak, Converse & Same Directions
	Direction BetterDir;
	int BetterDirDis;
	float ApeakProb_Coe; //Coefficient of Apeak Dir for choosing Probability
	int tmp[4],tmp1;

	IllegalDir=GetDirection(Sn->Position,Sn->Body[0]);
	DirS[IllegalDir] = ILLEGAL_DIR_SCR;

	if(GE.MushNum || GE.FMush || GE.HMush){
		//################################################### Phase 1: Eating Mushrooms #################################################################
		Phase=1;

		if(GE.FMush && GE.Diffic < Easy){
			CalcMushScr(Sn,GE.FMush,DirMushScr);
		}
		if(GE.HMush){
			CalcMushScr(Sn,GE.HMush,DirMushScr);
		}
		for(i=0;i<(GE.MushNum + GE.PMushNum)*(5-GE.Diffic)/5;i++){
			CalcMushScr(Sn,GE.Mushs[i],DirMushScr);
		}

		for(i=0;i<4;i++){// using bubble Sort to make Descending Order for Mush Scores of Directions:
			for(j=0;j<4-i-1;j++){
				if(DirMushScr[j][0] < DirMushScr[j+1][0]){
					t=DirMushScr[j][0];
					DirMushScr[j][0]=DirMushScr[j+1][0];
					DirMushScr[j+1][0]=t;
					
					t=DirMushScr[j][1];
					DirMushScr[j][1]=DirMushScr[j+1][1];
					DirMushScr[j+1][1]=t;
				}
			}
		}

		j=0;
		for(i=0;i<4;i++){
			if(i>0 && DirMushScr[i][0]!=DirMushScr[i-1][0]) 
				j++;
			DirS[(int)DirMushScr[i][1]] += FinalMushDirS[j];
		}
	}else if(GE.GameMode==Special && GE.AliveSnakesNum>2 && !(GE.AliveSnakesNum==4 && Sn->Num==4)){
		//####################################################### Phase 2: Merging #######################################################################
		Phase=2;
		for(i=1;i<GE.SnakesNum;i++){// First Set Target Snake for merging
			if(GE.Snakes[i]!=Sn && GE.Snakes[i]->IsDead==False){
				TarSn=GE.Snakes[i];
				break;
			}
		}
		GetNearWays_SS(Sn,TarSn,&VerDir,&HorDir);
		if(VerDir==-2){//from 2 vertical Dir distance is equal to reach Target:
			DirS[Up]+=PH2_SCR;
			DirS[Down]+=PH2_SCR;
		}else if(VerDir != -1){
			DirS[VerDir]+=PH2_SCR;
		}
		if(HorDir==-2){//from 2 Horizontal Dir distance is equal to reach Target:
			DirS[Left]+=PH2_SCR;
			DirS[Right]+=PH2_SCR;
		}else if(HorDir != -1){
			DirS[HorDir]+=PH2_SCR;
		}
	}else{
		//####################################################### Phase 3: Attack #######################################################################
		Phase=3;
		if(GE.GameMode==League || GE.GameMode==Special)// Choose a Target(One of Alive Snakes):
			TarSn=GE.Snakes[0];
		else {
			Sn->IsDead=True;
			GE_GetBestSnakes(&tmp1,tmp,True);
			Sn->IsDead=False;
			found=0;
			for(i=0;i<tmp1;i++)//find Sn Target in new best Snakes.if not found then change target of this Sn:
				if(Sn->Target==tmp[i]){
					found=1;
					break;
				}
			if(!found) Sn->Target=tmp[Rand(0,tmp1)];//Now Change Target to one of New best Snakes by Random
			TarSn=GE.Snakes[Sn->Target];
		}
		
		ApeakDirDis=ApeakMoveDir(Sn,TarSn,ApeakDirs);// Get Best Apeak Dir and Distance
		
		SameDir=TarSn->Direction;
		SameDirDis=Dir_Dis(SameDir,Sn->Position,TarSn->Position);
		
		ConvDir=AdverseDir(SameDir); //Get Converse Dir of Target Snake
		ConvDirDis=Dir_Dis(ConvDir,Sn->Position,TarSn->Position);
		
		//when HaveWall be True then one of Converse or Same Direction of TarSn will choose for higher importance after Apeak Dir
		//when it is False then ConvDir will be selected:
		if(ConvDirDis==-1){
			BetterDir=SameDir;
			BetterDirDis=SameDirDis;
			ApeakProb_Coe=5;
		}else{
			BetterDir=ConvDir;
			BetterDirDis=ConvDirDis;

			// Calc Probability of choosing Apeak Dir or Converse Dir:
			ApeakProb_Coe = ATT_APEAK_DIR_BCOE + 
				(ApeakDirDis - BetterDirDis) / 
				(float)(ApeakDirDis + BetterDirDis) * 
				((ApeakDirDis >= BetterDirDis) ? ATT_CONV_DIR_BCOE : ATT_APEAK_DIR_BCOE);

		}

		tmp1=Rand(0,101);//returns an int between 0 to 100
		
		// Now Set Scores of 4 Directions:
		if(tmp1<=ApeakProb_Coe){// by this Rand int Apeak Dir was chosen:
			DirS[(int)ApeakDirs[0]] += PH3_FIR_SCR;
			if(ApeakDirs[1] != -1) DirS[(int)ApeakDirs[1]] += PH3_FIR_SCR;// if Second Apeak Dir equals First One so Set PH3_FIR_SCR to it
			DirS[(int)BetterDir] += PH3_SEC_SCR;
		}else{// by this Rand int Converse Dir was chosen:
			DirS[(int)BetterDir] += PH3_FIR_SCR;
			DirS[(int)ApeakDirs[0]] += PH3_SEC_SCR;
			if(ApeakDirs[1] != -1) DirS[(int)ApeakDirs[1]] += PH3_SEC_SCR;
		}
		if(ApeakDirs[1] == -1) 
			DirS[(int)AdverseDir(ApeakDirs[0])] += PH3_FAR_APEAK_SCR;
		DirS[(int)AdverseDir(BetterDir)] += PH3_LAST_DIR_SCR;
	}

	CheckTraps(Sn,DirS,Phase,IllegalDir);

	Sn->Direction = (Direction)(MNumA(DirS,4,IsMore,&tmp1));
}

// Checks Traps of Future moves and Reduces scores of DirS[] as much as dangerousness of that Direction
static void CheckTraps(Snake *Sn,int DirS[],int Phase,int IllegalDir){
	int i,j,z;
	int TrapsS[]={0,0,0,0} ;//Traps Score of 4 Direction
	Trap_Param TP;
	Snake *TarSn=0;
	Snake *NearSnakes[3];
	int SnakesDis[3];
	int NearSnNum,MinDis;
	int Found;
	short NX,NY;
	int Res;

	int BoundObjs[4][3];//[][0] for Objs Code,[][1] for Number of Objs Occurrence,[][2] for min Dis of Objs to Sn 
	int BoundObjsCount;
	Direction ApeakDirs[2];
	COORD FirObs[4];//first obstacle
	int ObsCodes[4];//first obstacle Codes
	COORD SnNPos;//SN New Position
	Direction Dir;
	int tmp;
	Snake* Tmp;
	short tx,ty;

	//############################################## Simulation For Recursive Traps Checking: #######################################################

	if(GE.Diffic!=Funny){// if Game Difficulty is higher than Funny so do simulation:
		TP.Width=GE.Width;
		TP.Height=GE.Height;

		TP.GField=(signed char**)malloc(sizeof(signed char*) * GE.Width);//Allocating space for Game Field:
		for(i=0;i<GE.Width;i++){
			TP.GField[i]=(signed char*)malloc(sizeof(signed char) * GE.Height);
			for(j=0;j<GE.Height;j++)//initialize all elements to 0:
				TP.GField[i][j]=0;
		}
		if(GE.HaveWall){ //If HaveWall is True then change value of all COORD that Walls exists to 1:
			for(i=0;i<GE.Width;i++){
				TP.GField[i][0]=1;
				TP.GField[i][GE.Height-1]=1;
			}
			for(i=1;i<GE.Height-1;i++){
				TP.GField[0][i]=1;
				TP.GField[GE.Width-1][i]=1;
			}
		}
		
		TP.T1RecMaxLevel = TRAP_T1_RECLEVEL - GE.Diffic * 2;
		if(GE.Diffic==Easy)
			TP.T2RecMaxLevel = TP.T1RecMaxLevel+1;
		else if(GE.Diffic==Norm)
			TP.T2RecMaxLevel = TP.T1RecMaxLevel+3;
		else
			TP.T2RecMaxLevel = TRAP_T1_RECLEVEL - GE.Diffic*5;

		TP.IsBackTrack=False;
		TP.TarSn=0;

		if(GE.GameMode==League){
			if(GetNearSnakes(Sn->Position,Sn,TAR_SN_MAXDIS,NearSnakes,SnakesDis))// if Player is Near then set Target to it else we have no target:
				TarSn=GE.Snakes[0];
		}else{
			NearSnNum=GetNearSnakes(Sn->Position,Sn,TAR_SN_MAXDIS,NearSnakes,SnakesDis);
			if(NearSnNum>0){//if we have near Snakes:
				Found=-1;
				for(i=0;i<NearSnNum;i++){//find Snake(s) that their target is me:
					if(NearSnakes[i]->Target == Sn->Num-1){
						if(Found == -1){
							Found=i;
							MinDis=SnakesDis[i];
						}else if(MinDis > SnakesDis[i]){
							Found=i;
							MinDis=SnakesDis[i];
						}
					}
				}
				if(Found == -1){
					Found=0; MinDis=SnakesDis[0];
					for(i=1;i<NearSnNum;i++){//target is Nearest Snake to me:
						if(MinDis > SnakesDis[i]){
							Found=i;
							MinDis=SnakesDis[i];
						}
					}
					TarSn=NearSnakes[Found];
				}else{
					TarSn=NearSnakes[Found];
				}
			}
		}
		if(TarSn){
			TP.IsBackTrack=True;
			TP.TarSnIsDead=False;
			if(TarSn->IsCom==False || Phase!=2)
				TP.TarSnL=TarSn->BodyLen+1;
			else
				TP.TarSnL=TarSn->BodyLen;
			TP.TarSn=(COORD*)malloc(sizeof(COORD) * (TP.TarSnL));
			//if Target is Com and phase is 2 then we can collide Head of Target Snake so its position value must be 0 :
			if(TarSn->IsCom==False || Phase!=2){
				TP.TarSn[0]=TarSn->Position;
				TP.GField[TP.TarSn[0].X][TP.TarSn[0].Y] = 1;
				for(i=1;i<=TarSn->BodyLen;i++){
					TP.TarSn[i]=TarSn->Body[i-1];
					TP.GField[TP.TarSn[i].X][TP.TarSn[i].Y] = 1;
				}
			}else{
				for(i=0;i<TarSn->BodyLen;i++){
					TP.TarSn[i]=TarSn->Body[i];
					TP.GField[TP.TarSn[i].X][TP.TarSn[i].Y] = 1;
				}
			}

			TP.TarSn_IllegalDir = GetDirection(TP.TarSn[0],TP.TarSn[1]);
		}
		
		TP.SnL=Sn->BodyLen+1;//Allocate memory for TP.Sn and Set positions of Sn Body parts to it:
		TP.Sn=(COORD*)malloc(sizeof(COORD) * (TP.SnL));
		TP.Sn[1]=Sn->Position;

		TP.GField[Sn->Position.X][Sn->Position.Y]=1;//Set All Positions of Sn Head & Bodies value to 1:
		for(i=0;i < Sn->BodyLen - 1;i++){
			TP.GField[Sn->Body[i].X][Sn->Body[i].Y]=1;
			TP.Sn[i+2]=Sn->Body[i];
		}

		for(i=0;i<4;i++){
			if(i != IllegalDir){
				GetNewPos(Sn->Position.X,Sn->Position.Y,i,&NX,&NY);
				if(GE.GameField[NX][NY] <= SNAKEHEAD && ((Snake*)GE.GameFieldObj[NX][NY])!=Sn){// if new place that snakes wants to go there is an Enemy Snake Head or Body(not Sn head or body):
					if(Phase==2 && ((Snake*)GE.GameFieldObj[NX][NY])->IsCom && GE.GameField[NX][NY]==SNAKEHEAD) 
						TrapsS[i] = TAR_SN_HEAD_SCR;
					else if(GE.Diffic<=Hard) {
						TrapsS[i] = ENEMY_SN_BODY_HEAD_SCR; 
					}else
						TrapsS[i] = OBSTACLE_SCR; 
				}else if(GE.GameField[NX][NY] <= BRANCH){// if new place that snakes wants to go that is an Branch or there is parts of me:
					TrapsS[i] = OBSTACLE_SCR;
				}else{ // otherwise check next places and ways recursively if Game Difficulty is not Funny:
					TP.IllegalDir=AdverseDir((Direction)i);
					TP.Sn[0].X=NX; TP.Sn[0].Y=NY;
					TP.GField[NX][NY]=1;
					TP.Reclevel=1;
					if(TP.TarSn){
						
						Res = -1 * Rec_TrapsScore(&TP);
						
						TP.GField[NX][NY]=0;// Return Head of Sn to its previous Location for back tracking
						TP.Sn[0] = TP.Sn[1];
					}else{
						Res = -1 * Rec_TrapsScoreT2(&TP);

						for(j=0 ; j<TP.SnL ; j++)//Returns TP.GField and other Variables to Previous State before calling of Recursive Func for backtracking:
							TP.GField[TP.Sn[j].X][TP.Sn[j].Y]=0;
						TP.Sn[1]=Sn->Position;
						TP.GField[Sn->Position.X][Sn->Position.Y]=1;
						for(j=0;j < Sn->BodyLen - 1;j++){
							TP.GField[Sn->Body[j].X][Sn->Body[j].Y]=1;
							TP.Sn[j+2]=Sn->Body[j];
						}
					}
					if(Res == -PH3_FIR_SCR) Res = OBSTACLE_SCR;
					TrapsS[i]=Res;
				}
			}
		}
	}else{// if Game Difficulty is Funny only check next move of 3 possible ways:
		for(i=0;i<4;i++)
			if(i != IllegalDir){
				GetNewPos(Sn->Position.X,Sn->Position.Y,i,&NX,&NY);
				if(GE.GameField[NX][NY]==SNAKEHEAD && Phase==2 && ((Snake*)GE.GameFieldObj[NX][NY])!=Sn && ((Snake*)GE.GameFieldObj[NX][NY])->IsCom)// if new place that snakes wants to go that is an Obstacle:
					TrapsS[i] = TAR_SN_HEAD_SCR;
				else if(GE.GameField[NX][NY]<=BRANCH)
					TrapsS[i] = OBSTACLE_SCR;
			}
	}

	//################################################ Square Traps Checking: ########################################################
	
	if(GE.Diffic <= Hard){//if Difficulty be more than Normal then Check Squar Traps:
		if(TarSn){//First Remove TarSn from GField because maybe in its next move will die: 
			TP.GField[TarSn->Position.X][TarSn->Position.Y]=0;
			for(i=0; i<TarSn->BodyLen ; i++){
				TP.GField[TarSn->Body[i].X][TarSn->Body[i].Y]=0;
			}
		}
		for(i=0 ; i < GE.SnakesNum ; i++){// first add All Alive Snakes to GField and move them once in GField(if can Move otherwise don't add them):
			if(GE.Snakes[i]->IsDead==False){
				Tmp=GE.Snakes[i];
				if(Tmp!=Sn){
					GetNewPos(Tmp->Position.X,Tmp->Position.Y,Tmp->Direction,&NX,&NY);
					j=0;
					// if new place that snakes wants to go is an Obstacle found other way if not then don't add it to Game Field:
					while(GE.GameField[NX][NY] <= BRANCH && j<4){
						if(j!=Tmp->Direction)
							GetNewPos(Tmp->Position.X,Tmp->Position.Y,j,&NX,&NY);
						j++;
					}
					Found=0;
					if(GE.GameField[NX][NY] > BRANCH){
						TP.GField[NX][NY]=Tmp->Num;
						Found=1;// Shows that Current Snake in its next move won't die
					}
				}else
					Found=1;
				if(Found)
					TP.GField[Tmp->Position.X][Tmp->Position.Y]=Tmp->Num;
					for(j=0 ; j < Tmp->BodyLen-1 ; j++){
						TP.GField[Tmp->Body[j].X][Tmp->Body[j].Y]=Tmp->Num;
				}
			}
		}

		if(GE.HaveWall){
			for(i=0;i<GE.Width;i++){
				TP.GField[i][0]=-1;
				TP.GField[i][GE.Height-1]=-3;
			}
			for(i=1;i<GE.Height-1;i++){
				TP.GField[0][i]=-4;
				TP.GField[GE.Width-1][i]=-2;
			}
		}
		
		for(i=0;i<4;i++){
			if(i != IllegalDir){
				GetNewPos(Sn->Position.X,Sn->Position.Y,i,&NX,&NY);// now place Head of Sn for next move:
				if(GE.GameField[NX][NY] > BRANCH){// if new place that snakes wants to go is not an Obstacle:
					TP.GField[NX][NY]=Sn->Num;
					TP.IllegalDir=AdverseDir((Direction)i);
					GetApeak((Direction)i,ApeakDirs);
					SnNPos.X=NX; SnNPos.Y=NY;

					//Get First Obstacle position & Type when moving in 3 possible Dirs that Sn can go:
					FirObs[0]=GetFirstObstacle(NX,NY,ApeakDirs[0],&TP,&ObsCodes[0]);
					FirObs[1]=GetFirstObstacle(NX,NY,ApeakDirs[1],&TP,&ObsCodes[1]);
					FirObs[2]=GetFirstObstacle(NX,NY,(Direction)i,&TP,&ObsCodes[2]);
					tx=NX; ty=NY;//Get Last Body of Sn position when moving in illegal Dir:
					do{
						GetNewPos(tx,ty,TP.IllegalDir,&tx,&ty);
					}while(TP.GField[tx][ty] == Sn->Num && (tx!=NX || ty!=NY));
					GetNewPos(tx,ty,(Direction)i,&tx,&ty);
					FirObs[3].X=tx;
					FirObs[3].Y=ty;
					ObsCodes[3]=Sn->Num;

					if(//if left & Right Objects and UP & Down Objects are not same so we are in a square trap:
					 (FirObs[0].X != FirObs[1].X || FirObs[0].Y != FirObs[1].Y ) &&
					 (GE.HaveWall || (FirObs[2].X != FirObs[3].X || FirObs[2].Y != FirObs[3].Y) ) &&
					 (ObsCodes[0]>0 || ObsCodes[1]>0) //obstacle 1 & 2 must not be game around walls:
					){
						BoundObjsCount=0;//first Index always belongs to Sn
						tmp=1;
						for(j=0;j<3;j++)//Reset Values:
							BoundObjs[j][1]=0;
						BoundObjs[0][0]=Sn->Num;
						for(j=0;j<3;j++){
							if(j<=1) Dir=ApeakDirs[j]; else Dir=(Direction)i;
							if((ObsCodes[j]>0 && Dir_Dis(Dir,SnNPos,FirObs[j]) <= GE.Snakes[ObsCodes[j]-1]->BodyLen / SQ_SN_DIS_COE) ||
							   (ObsCodes[j]<0 && (Dir==i || Dir_Dis(Dir,SnNPos,FirObs[j]) <= SQ_WALL_DIS))){//if Distances of Founded Obstacles is proper them save & consider them:
								   BoundObjsCount++;
								   if(ObsCodes[j]==Sn->Num){//index 0 is for Sn:
									   BoundObjs[0][1]++;
								   }else{//other indexes have not any arrangement.if number of it exists then increase its occurrence otherwise add it to array:
									   Found=0;
									   for(z=1;z<tmp;z++){
										   if(BoundObjs[z][0]==ObsCodes[j]){
											   Found=1;
											   BoundObjs[z][1]++;
											   break;
										   }
									   }
									   if(!Found){
										   BoundObjs[tmp][0]=ObsCodes[j];//Set its Code
										   BoundObjs[tmp][1]=1;
										   BoundObjs[tmp][2]=MinDisofObjs(Sn,ObsCodes[j]);//Min Dis of Sn to Object(Snake or Wall)
										   tmp++;
									   }
								   }
							}
						}
						if(BoundObjsCount==3){
							Res=0;
							switch(BoundObjs[0][1]){// if any of 3 Bounded Obstacles aren't Sn: 
							case 0:
								if(BoundObjs[1][1]==3)
									if(BoundObjs[1][2]==1) 
										Res=60;
									else if(BoundObjs[1][2]<=4)
										Res=45;
									else// more than 4:
										Res=40;
								else if(BoundObjs[1][1]==2 || BoundObjs[2][1]==2){//one of objs have occurred 2 and another once:
									if(BoundObjs[1][2]<=2 || BoundObjs[2][2]<=2)
										if((BoundObjs[1][0]<0 && BoundObjs[1][2]==1) || (BoundObjs[2][0]<0 && BoundObjs[2][2]==1))
											Res=40;
										else if(BoundObjs[1][0]>0 && BoundObjs[2][0]>0)
											Res=40;
								}else{// every obj have occurred once:
									if(BoundObjs[1][2]<=2 || BoundObjs[2][2]<=2 || BoundObjs[3][2]<=2)
										Res=33;
								}
								break;
							case 1:
								if(BoundObjs[1][1]==2){// if we have only one Obj(Snake) around Sn:
									if(BoundObjs[1][2]==1)
										Res=55;
									else if(BoundObjs[1][2]<=4)
										Res=32;
								}else{//if we have two Obj around Sn:
									if(BoundObjs[1][0]>0 && BoundObjs[2][0]>0){
										if(BoundObjs[1][2]==1 && BoundObjs[2][2]==1)
											Res=40;
									}else if(BoundObjs[1][0]<0 && BoundObjs[2][0]<0){
										if(BoundObjs[1][2]==1 && BoundObjs[2][2]==1)
											Res=70;
										else if(BoundObjs[1][2]<=3 && BoundObjs[2][2]<=3)
											Res=35;
									}else{
										if(BoundObjs[1][2]==1 && BoundObjs[2][2]==1)
											Res=50;
										else if(BoundObjs[1][2]<=4 && BoundObjs[2][2]<=4)
											Res=35;
									}
								}
								break;
							case 2:
								if(BoundObjs[1][0]<0){//be Wall:
									if( BoundObjs[1][2]==1 ) Res=60;
								}else{//be Snake:
									if( BoundObjs[1][2]==1 )
										Res=50;
									else if( BoundObjs[1][2]<=4)
										Res=30;
								}
								break;
							case 3:
								Res=80;
							}

							Res=(Res * PH3_FIR_SCR) / 100 * -1;
							// if Trap Score of Recursive checking be less than of Trap Score of Square Checking so do anything
							// otherwise Use Score of this Part(Square Check -> Res):
							if(Res < TrapsS[i]) TrapsS[i]=Res; // Save Max Trap Score for next Trap Checking algorithms
						}
					}

					TP.GField[NX][NY]=0;
				}
			}
		}
	}
	
	if(GE.Diffic!=Funny){
		for(i=0;i<GE.Width;i++) //release all allocated memories:
			free(TP.GField[i]);
		free(TP.Sn);
		if(TP.TarSn) free(TP.TarSn);
		free(TP.GField);
	}
	
	////################################################ PMushs existing Checking: ########################################################

	if(GE.Diffic != Funny){
		for(i=0;i<4;i++){
			if(i!=IllegalDir){
				GetNewPos(Sn->Position.X,Sn->Position.Y,i,&NX,&NY);
				if(GE.GameField[NX][NY]==PMUSH && TrapsS[i] > PMUSH_SCR){
					TrapsS[i]=PMUSH_SCR;
				}
			}
		}
	}
	for(i=0;i<4;i++){//Now Add Traps Score to DirS:
		DirS[i] += TrapsS[i];
	}
}

// Recursive Func for Traps Checking and specifying Scores of each move for Move Type 1 and 2.
// Move Type1: Game field contains Sn,TarSn & Walls but MoveType2 only contains Sn and walls
static int Rec_TrapsScore(Trap_Param *TP){
	short TarNX,TarNY;
	short NX,NY;
	short SnEmptyWays[3],TarEmptyWays[3];
	short SnEmptyNum=0,TarEmptyNum = -1;//it must be -1 because by this num we can find out that whether in this RecLevel we removed TarSn from GameField or not(it was dead?)(when TarSn hasn't any empty way we remove it) if removed so after backtracking alive it and return it to GField 
	COORD SnTail,TarTail;
	short i,j;
	int MinS,Scr,Aver=0;
	char DangerConCount=0;

	if(TP->Reclevel <= TP->T1RecMaxLevel && TP->TarSnIsDead==False){
		TarEmptyNum=0;
		for(i=0;i<4;i++){//First Count Empty Ways of Target Snake for its next move:
			if(i != TP->TarSn_IllegalDir){
				GetNewPos(TP->TarSn[0].X,TP->TarSn[0].Y,i,&TarNX,&TarNY);
				if(TP->GField[TarNX][TarNY] == 0)
					TarEmptyWays[TarEmptyNum++]=i;
			}
		}
		// because TarSn haven't any empty way so it will die in next move so we remove it from game field after backtracking we alive it again:
		if(TarEmptyNum==0){
			TP->TarSnIsDead=True;
			for(i=0 ; i < TP->TarSnL ; i++)
				TP->GField[TP->TarSn[i].X][TP->TarSn[i].Y]=0;
		}
	}
	if(TP->Reclevel <= TP->T1RecMaxLevel && TP->TarSnIsDead==False){
		i=TP->TarSnL-1;//Move TarSn body without its Head to Next position:
		TarTail=TP->TarSn[i];
		TP->GField[TarTail.X][TarTail.Y] = 0;
		for(;i>0;i--)
			TP->TarSn[i]=TP->TarSn[i-1];
		
		i=TP->SnL-1;//Now Move Sn body without its Head to Next position:
		SnTail=TP->Sn[i];
		TP->GField[SnTail.X][SnTail.Y] = 0;
		for(;i>0;i--)
			TP->Sn[i]=TP->Sn[i-1];

		for(i=0;i<TarEmptyNum && DangerConCount < 2;i++){
			GetNewPos(TP->TarSn[1].X,TP->TarSn[1].Y,TarEmptyWays[i],&TarNX,&TarNY);//Get New Position of TarSn Head then move TarSn to it:
			TP->GField[TarNX][TarNY]=1;
			TP->TarSn[0].X = TarNX; TP->TarSn[0].Y = TarNY;
			TP->TarSn_IllegalDir=AdverseDir((Direction)TarEmptyWays[i]);
			
			SnEmptyNum=0;
			for(j=0;j<4;j++){//Now Count and Save Empty Ways of Sn for its next move:
				if(j != TP->IllegalDir){
					GetNewPos(TP->Sn[0].X,TP->Sn[0].Y,j,&NX,&NY);
					if(TP->GField[NX][NY] == 0)
						SnEmptyWays[SnEmptyNum++]=j;
				}
			}

			//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			if(SnEmptyNum == 0){// it is so dangerous so consider its point -100000 :
				Aver += PH3_FIR_SCR;
				DangerConCount++; //Dangerous Condition Counter must be increased
			}else if(SnEmptyNum <= 2){//if be 3 so we dont need to check every ways recursively beacause it is not dangerous so consider its point to 0:
				MinS = INT_MAX;
				for(j=0 ; j<SnEmptyNum && MinS > 0; j++){
					GetNewPos(TP->Sn[0].X,TP->Sn[0].Y,SnEmptyWays[j],&NX,&NY);//Get New Position of Sn Head then move Sn to it:
					TP->GField[NX][NY]=1;
					TP->Sn[0].X=NX; TP->Sn[0].Y=NY;
					TP->IllegalDir=AdverseDir((Direction)SnEmptyWays[j]);

					TP->Reclevel++; //increase Recursive Level
					Scr = Rec_TrapsScore(TP);
					TP->Reclevel--; //Reduce Recursive Level for Backtracking
					if(Scr < MinS) MinS = Scr;

					TP->GField[NX][NY]=0;// Return Head of Sn to its previous Location for back tracking
					TP->Sn[0]=TP->Sn[1];
				}
				Aver += MinS;
				if(MinS == PH3_FIR_SCR) DangerConCount++; //Dangerous Condition Counter must be increased if 3 possible ways contain obstacle
			}
			//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

			TP->GField[TarNX][TarNY]=0;// Return Head of TarSn to its previous Location for back tracking
			TP->TarSn[0]=TP->TarSn[1];
		}

		for(i=0 ; i< TP->SnL-1 ; i++)// Return Body of Sn to its previous Location for back tracking:
			TP->Sn[i]=TP->Sn[i+1];
		TP->Sn[i]=SnTail;
		TP->GField[SnTail.X][SnTail.Y] = 1;

		for(i=0 ; i < TP->TarSnL-1 ; i++)// Return Body of TarSn to its previous Location for back tracking:
			TP->TarSn[i]=TP->TarSn[i+1];
		TP->TarSn[i]=TarTail;
		TP->GField[TarTail.X][TarTail.Y] = 1;

		switch(DangerConCount){
		case 0:
			return Aver/TarEmptyNum;
			break;
		case 1:
			if(TarEmptyNum == 1)
				return PH3_FIR_SCR;
			else // 2 and 3:
				return max(
						   (int)((1.0F - 0.05F * TarEmptyNum) * PH3_FIR_SCR),
						   Aver/TarEmptyNum
						  );
			break;
		default:// 2 and 3 :
			return PH3_FIR_SCR;
		}
	}else{
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		Scr = Rec_TrapsScoreT2(TP);
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		if(TarEmptyNum==0){// if we have removed TarSn from GameField beacause it was dead so return it to gameField for BackTrack:
			TP->TarSnIsDead=False;
			for(i=0 ; i < TP->TarSnL ; i++)
				TP->GField[TP->TarSn[i].X][TP->TarSn[i].Y]=1;
		}

		return Scr;
	}
}

// Recursive Func for Traps Checking and specifying Scores of each move only for Move Type 2.in this move type Game Field Contains only Walls and Sn
static int Rec_TrapsScoreT2(Trap_Param *TP){
	int i,EmptyWay=-1,Scr;
	short NX,NY;
	COORD Tail;

	if(TP->T2RecMaxLevel == TP->Reclevel){
		return 0;
	}else{
		for(i=0;i<4;i++){
			if(i != TP->IllegalDir){
				GetNewPos(TP->Sn[0].X,TP->Sn[0].Y,i,&NX,&NY);
				if(TP->GField[NX][NY] == 0){
					if(EmptyWay == -1)//if we have more than 1 empty way then return 0 else contain empty way for Recursive:
						EmptyWay=i;
					else
						return 0;
				}
			}
		}
		
		if(EmptyWay == -1) //if we have no empty way then return this way is dangerous (return -PH3_FIR_SCR):
			return PH3_FIR_SCR;
		else{// we have only one empty way so check it recursivly:
			i=TP->SnL-1;//First Move Sn to new position:
			Tail=TP->Sn[i];
			TP->GField[Tail.X][Tail.Y] = 0;
			for(;i>0;i--)
				TP->Sn[i]=TP->Sn[i-1];
			GetNewPos(TP->Sn[0].X,TP->Sn[0].Y,EmptyWay,&NX,&NY);
			TP->GField[NX][NY]=1;
			TP->Sn[0].X = NX; TP->Sn[0].Y = NY;
			TP->IllegalDir=AdverseDir((Direction)EmptyWay);
			TP->Reclevel++;
			
			//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			Scr = (Rec_TrapsScoreT2(TP) + PH3_FIR_SCR)/2;
			//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

			if(TP->IsBackTrack){
				TP->Reclevel--;
				TP->GField[NX][NY]=0;
				for(i=0 ; i < TP->SnL-1 ; i++)
					TP->Sn[i]=TP->Sn[i+1];
				TP->Sn[i]=Tail;
				TP->GField[Tail.X][Tail.Y]=1;
			}
			
			return Scr;
		}
	}
}

static void CalcMushScr(Snake* Sn,Mushroom* Mu,float DirMushScr[4][2]){
	int WPH = GE.Width + GE.Height; // Width Plus Height
	int Scr;
	Direction VerDir,HorDir;
	int AbsDis;//Absolute Distance
	GetNearWays_SC(Sn,Mu->Position,&VerDir,&HorDir);
	switch(Mu->Type){
	case Normal:
		AbsDis=Dis(Sn->Position,Mu->Position);
		Scr=WPH * MUSH_SCR_COE / (float)AbsDis;
		break;
	case Poison:
		AbsDis=Dis(Sn->Position,Mu->Position);
		Scr=WPH * PMUSH_SCR_COE / (float)AbsDis;
		break;
	case Fast:
		Scr=FMUSH_SCR;
		break;
	case Half:
		Scr=HMUSH_SCR;
	}
	
	if(VerDir==-2){
		DirMushScr[Up][0] += Scr;
		DirMushScr[Down][0] += Scr;
	}else if(VerDir != -1){
		DirMushScr[VerDir][0] += Scr;
	}
	if(HorDir==-2){
		DirMushScr[Left][0] += Scr;
		DirMushScr[Right][0] += Scr;
	}else if(HorDir != -1){
		DirMushScr[HorDir][0] += Scr;
	}
}

// Returns First Obstacle position and type when we start moving from 'StartX' , 'StartY' COORD with 'MoveDir' Direction.'ObsCode' returns Obstacle Code.
static COORD GetFirstObstacle(short StartX,short StartY,Direction MoveDir,Trap_Param *TP,int *ObsCode){
	short NX,NY;
	COORD Res;

	NX=StartX;
	NY=StartY;
	do{
		GetNewPos(NX,NY,MoveDir,&NX,&NY);
	}while(TP->GField[NX][NY] == 0);
	Res.X=NX;
	Res.Y=NY;
	*ObsCode=TP->GField[NX][NY];
	return Res;
}

// Returns Min Distance between Sn and specified Object(Obj2 that is code of it in GFiled Simulation)
static int MinDisofObjs(Snake* Sn,int Obj2){
	int min=INT_MAX;
	Snake *Sn2;
	COORD tmp;
	int D;
	int i,j;

	if(Obj2<0){//it is a Wall
		if(	Obj2 == -1 || Obj2 == -4){// set COORD of Wall if be wall 1 or 4:
			tmp.X=0;tmp.Y=0;
		}else{// set COORD of Wall if be wall 2 or 3:
			tmp.X=GE.Width-1;tmp.Y=GE.Height-1;
		}

		//Calc Dis of Sn Head and Wall:
		if(Obj2 == -1 || Obj2 == -3)//if wall be Horizontal:
			D=VerDis(Sn->Position,tmp);
		else//if wall be Vertical:
			D=HorDis(Sn->Position,tmp);
		if(min > D) min=D;

		for(i=0 ; i<Sn->BodyLen ; i++){//Now Calc Dis of Sn Body to Wall:
			if(Obj2 == -1 || Obj2 == -3)
				D=VerDis(Sn->Body[i],tmp);
			else
				D=HorDis(Sn->Body[i],tmp);
			if(min > D) min=D;
		}
	}else{//if Obj2 be a Snake:
		Sn2=GE.Snakes[Obj2-1];

		for(i=0;i<Sn->BodyLen;i++){// First Calc min dis of Sn2 Head to all parts of Sn Body: 
			D=Dis(Sn->Body[i],Sn2->Position);
			if(min > D) min=D;
		}

		for(i=0;i<Sn2->BodyLen;i++){//Calc min dis of Sn Head to all parts of Sn2 Body:
			D=Dis(Sn2->Body[i],Sn->Position);
			if(min > D) min=D;
		}

		D=Dis(Sn2->Position,Sn->Position);//Calc min dis of Sn2 Head to Sn Head:
		if(min > D) min=D;

		for(i=0;i<Sn->BodyLen;i++){//Now Calc min dis of Sn2 Body Parts to parts of Sn Body:
			for(j=0;j<Sn2->BodyLen;j++){
				D=Dis(Sn->Body[i],Sn2->Body[j]);
				if(min > D) min=D;
			}
		}
	}
	return min;
}

// Returns Number of Near Snakes to 'P' Coordinate that all parts of that snakes(Head and Bodies) have less than 'MaxDis' Distance from it and returns 
// themselves by 'Res' Param.You Must Get Sn to 'OurSn' param, because we don't want to return it as result.'SnakesDis' Byref Param returns Distances
// of Snakes that exist in 'Res' Param. 
static int GetNearSnakes(COORD P,Snake *OurSn,int MaxDis,Snake *Res[],int SnakesDis[]){
	int i,h,j=0,MinDis,tmp;

	for(i=0;i<GE.SnakesNum;i++){
		if(GE.Snakes[i]->IsDead==False && GE.Snakes[i] != OurSn){
			MinDis=Dis(P,GE.Snakes[i]->Position);
			for(h=0;h<GE.Snakes[i]->BodyLen;h++){
				tmp=Dis(P,GE.Snakes[i]->Body[h]);
				if(tmp<MinDis) MinDis=tmp;
			}
			if(MinDis<=MaxDis){
				Res[j]=GE.Snakes[i];
				SnakesDis[j]=MinDis;
				j++;
			}
		}
	}
	return j;
}

// Returns Distance between two points for going from P1 to P2 By 'Dir' Direction.Returns -1 if we can't go from 'Dir' Direction because we have Wall around 
// game field(when HaveWall is True)
static int Dir_Dis(Direction Dir,COORD P1,COORD P2){
	switch(Dir){
	case Left:
		if(P2.X <= P1.X)
			return P1.X - P2.X;
		else if(GE.HaveWall)
			return -1;
		else
			return P1.X + GE.Width - P2.X;
		break;
	case Right:
		if(P1.X <= P2.X)
			return P2.X - P1.X;
		else if(GE.HaveWall)
			return -1;
		else
			return P2.X + GE.Width - P1.X;
		break;
	case Up:
		if(P2.Y <= P1.Y)
			return P1.Y - P2.Y;
		else if(GE.HaveWall)
			return -1;
		else
			return P1.Y + GE.Height - P2.Y;
		break;
	default:
		if(P1.Y <= P2.Y)
			return P2.Y - P1.Y;
		else if(GE.HaveWall)
			return -1;
		else
			return P2.Y + GE.Height - P1.Y;
	}
}

// Returns Horizontal Distance between two points from nearest Way
static int HorDis(COORD p1,COORD p2){
	if(GE.HaveWall)
		return abs(p2.X - p1.X);
	else
		return min(abs(p2.X - p1.X) , abs(p2.X + GE.Width - p1.X));
}

// Returns Vertical Distance between two points from nearest Way
static int VerDis(COORD p1,COORD p2){
	if(GE.HaveWall)
		return abs(p2.Y - p1.Y);
	else
		return min(abs(p2.Y - p1.Y) , abs(p2.Y + GE.Height - p1.Y));
}

// Returns Distance between two points from nearest Way
static int Dis(COORD p1,COORD p2){
	return VerDis(p1,p2) + HorDis(p1,p2);
}

/* Returns Min or Max number between parameters.if you want max value pass Max function and if you want Min value pass Min function to 'cmp' Arguman
pass Parameters(Values) number to PNum */
static int MNum(int PNum,int (*cmp)(int a,int b),...){
	int res,n;
	va_list va_marker;

	va_start(va_marker,PNum);
	res=va_arg(va_marker,int);
	while(--PNum)
		 if(cmp((n=va_arg(va_marker,int)) , res)) res = n; //Read Variable arguments from stack and Set Max value
	va_end(va_marker);//reset argument reader pointer to NULL
	return res;
}

/* Returns Index of Min or Max number of specified int Array.if you want max value pass Max function and if you want Min value pass Min function to 'cmp' arguman.
pass Array Len number to 'len' argument */
static int MNumA(int nums[],int len,int (*cmp)(int a,int b),int *Max){
	int res,i;
	
	*Max=nums[0];
	res=0;
	for(i=1;i<len;i++)
		if(cmp(nums[i],*Max)){
			*Max=nums[i];
			res=i;
		}
	return res;
}

static int IsLess(int a,int b){
	return (a<b);
}

static int IsMore(int a,int b){
	return (a>b);
}

// Returns two values: first, Nearest Vertical way(-2 | -1 | UP | Down) via 'VerDir' argument and second, Nearest Horizontal way(-2 | -1 | LEFT | RIGHT)
// via 'HorDir' argument between two given COORDs(Offset & Dest)
// if 'Offset' & 'Dest' Points have equal columns, 'HorDir' will be -1 and if have equal Rows then 'VerDir' will be -1
// if in Horizontal way if distance be equal from each direction(left & Right) then 'HorDir' will be -2 and this is right for 'VerDir'
static void GetNearWays(COORD Offset,COORD Dest,Direction *VerDir,Direction *HorDir){
	if(Offset.X == Dest.X)
		*HorDir =(Direction) -1;
	else if(Dest.X > Offset.X)
		if(GE.HaveWall || Dest.X - Offset.X < Offset.X + GE.Width - Dest.X)
			*HorDir = Right;
		else if(Dest.X - Offset.X == Offset.X + GE.Width - Dest.X)
			*HorDir =(Direction) -2;
		else
			*HorDir = Left;
	else
		if(GE.HaveWall || Offset.X - Dest.X < Dest.X + GE.Width - Offset.X)
			*HorDir = Left;
		else if(Offset.X - Dest.X == Dest.X + GE.Width - Offset.X)
			*HorDir =(Direction) -2;
		else
			*HorDir = Right;

	if(Offset.Y == Dest.Y)
		*VerDir =(Direction) -1;
	else if(Dest.Y > Offset.Y)
		if(GE.HaveWall || Dest.Y - Offset.Y < Offset.Y + GE.Height - Dest.Y)
			*VerDir = Down;
		else if(Dest.Y - Offset.Y == Offset.Y + GE.Height - Dest.Y)
			*VerDir =(Direction) -2;
		else
			*VerDir = Up;
	else
		if(GE.HaveWall || Offset.Y - Dest.Y < Dest.Y + GE.Height - Offset.Y)
			*VerDir = Up;
		else if(Offset.Y - Dest.Y == Dest.Y + GE.Height - Offset.Y)
			*VerDir =(Direction) -2;
		else
			*VerDir = Down;
}

// this func is same GetNearWays func but Offset & Dest of it, are  Head Position of specified Snakes(Sn > Offset & TarSn > Dest)
static void GetNearWays_SS(Snake *Sn,Snake *TarSn,Direction *VerDir,Direction *HorDir){
	GetNearWays(Sn->Position,TarSn->Position,VerDir,HorDir);
}

// this func is same GetNearWays func but Offset of it, is  Head Position of specified Snake
static void GetNearWays_SC(Snake *Sn,COORD Dest,Direction *VerDir,Direction *HorDir){
	GetNearWays(Sn->Position,Dest,VerDir,HorDir);
}

// Returns Adverse Direction of specified direction
Direction AdverseDir(Direction Dir){
	if(Dir<=1)
		return (Direction)!Dir;
	else
		return (Direction)(2+!(Dir-2));
}

// Returns Apeak Move Direction(s).'Res' parameter must have space for two Direction.first element of it is nearest Apeak Direction and second maybe the other
// Apeak Direction or be -1(when two Dir have the same Distances from Target Snake it has two Directions otherwise second is -1)
static int ApeakMoveDir(Snake *Sn,Snake *TarSn,Direction Res[]){
	Direction Apeaks[2];
	int DDis1,DDis2;
	
	GetApeak(TarSn->Direction,Apeaks);
	DDis1=Dir_Dis(Apeaks[0],Sn->Position,TarSn->Position);//Get Distances of Heads of two Snakes by two Apeak Directions:
	DDis2=Dir_Dis(Apeaks[1],Sn->Position,TarSn->Position);
	
	Res[1]=(Direction)-1;
	if(DDis1==-1){ // -1 represents 'Havewall' is True and we can go from only one ways of Apeak:
		Res[0]=Apeaks[1];
		return DDis2;
	}else if(DDis2==-1){
		Res[0]=Apeaks[0];
		return DDis1;
	}else if(DDis1<DDis2){ 
		Res[0]=Apeaks[0];
		return DDis1;
	}else if(DDis1>DDis2){
		Res[0]=Apeaks[1];
		return DDis2;
	}else{
		Res[0]=Apeaks[0];
		Res[1]=Apeaks[1];
		return DDis1;
	}
}

// Returns Apeak Directions of 'Dir' Direction(for example if you get left or right it returns Up and Down in an array)
static Direction *GetApeak(Direction Dir,Direction Res[]){
	if(Dir<=1){
		Res[0]=(Direction)2;
		Res[1]=(Direction)3;
	}else{
		Res[0]=(Direction)0;
		Res[1]=(Direction)1;
	}
	return Res;
}

// Returns Object Type of new Location that is result of moving from P COORD through 'Dir' Direction.
GameObjs GetObjTypeOfLoc(COORD p,Direction Dir){
	short NX,NY;
	GetNewPos(p.X,p.Y,Dir,&NX,&NY);
	return GE.GameField[NX][NY];
}

// Returns New X and New Y that are results of moving from X , Y COORD through 'Dir' Direction.
static void GetNewPos(short X,short Y,int Dir,short *NX,short *NY){
	X += DirDiff[Dir][0];
	Y += DirDiff[Dir][1];
	if(X==GE.Width)
		X = 0;
	else if(X<0)
		X = GE.Width-1;
	if(Y==GE.Height)
		Y = 0;
	else if(Y<0)
		Y = GE.Height-1;
	*NX = X;
	*NY = Y;
}

static void ReportVirStat(Trap_Param *TP){
	int i,j;
	_RPT3(_CRT_WARN,"RecLevel: %d , T1 & T2 MaxLevel: %d %d\n",TP->Reclevel,TP->T1RecMaxLevel,TP->T2RecMaxLevel);
	_RPT0(_CRT_WARN,"Game Field Status:\n");
	for(i=0 ; i < TP->Height ; i++){
		for(j=0 ; j < TP->Width ; j++){
			_RPT1(_CRT_WARN,"%c",(TP->GField[j][i]) ? '8':'*');
		}
		_RPT0(_CRT_WARN,"\n");
	}
	_RPT0(_CRT_WARN,"\nSn: \n");
	for(i=0 ; i < TP->SnL ; i++){
		_RPT2(_CRT_WARN,"(%d,%d) ",TP->Sn[i].X,TP->Sn[i].Y);
		if(TP->GField[TP->Sn[i].X][TP->Sn[i].Y] == 0)
			_RPT0(_CRT_WARN,"Diff ");
		_RPT0(_CRT_WARN,"\n");
	}

	_RPT0(_CRT_WARN,"\nTarSn: \n");
	if(TP->TarSn && !TP->TarSnIsDead){
		for(i=0 ; i < TP->TarSnL ; i++){
			_RPT2(_CRT_WARN,"(%d,%d) ",TP->TarSn[i].X,TP->TarSn[i].Y);
			if(TP->GField[TP->TarSn[i].X][TP->TarSn[i].Y] == 0)
				_RPT0(_CRT_WARN,"Diff ");
			_RPT0(_CRT_WARN,"\n");
		}
	}
	_RPT0(_CRT_WARN,"####################################################################################################\n");
	_RPT0(_CRT_WARN,"\n");
}

static void ReportVirStat1(Trap_Param *TP){
	int i,j,DrawGF=0;
	for(i=0 ; i < TP->SnL ; i++){
		if(TP->GField[TP->Sn[i].X][TP->Sn[i].Y] == 0){
			_RPT2(_CRT_WARN,"Sn: (%d,%d) Diff\n",TP->Sn[i].X,TP->Sn[i].Y);
			DrawGF=1;
		}
	}
	if(TP->TarSn && !TP->TarSnIsDead){
		for(i=0 ; i < TP->TarSnL ; i++){
			if(TP->GField[TP->TarSn[i].X][TP->TarSn[i].Y] == 0){
				_RPT2(_CRT_WARN,"TarSn: (%d,%d) Diff\n",TP->TarSn[i].X,TP->TarSn[i].Y);
				DrawGF=1;
			}
		}
	}
	if(DrawGF){
		_RPT3(_CRT_WARN,"\nRecLevel: %d , T1 & T2 MaxLevel: %d %d\n",TP->Reclevel,TP->T1RecMaxLevel,TP->T2RecMaxLevel);
		_RPT0(_CRT_WARN,"Game Field Status:\n");
		for(i=0 ; i < TP->Height ; i++){
			for(j=0 ; j < TP->Width ; j++){
				_RPT1(_CRT_WARN,"%c",(TP->GField[j][i]) ? '8':'*');
			}
			_RPT0(_CRT_WARN,"\n");
		}
		for(i=0 ; i < TP->SnL ; i++)
			_RPT2(_CRT_WARN,"(%d,%d) ",TP->Sn[i].X,TP->Sn[i].Y);
		_RPT0(_CRT_WARN,"\n#######################################################################################\n");
	}
}

void ReportGameField(){
	int i,j;
	_RPT0(_CRT_WARN,"Game Field Status:\n");
	for(i=0 ; i < GE.Height ; i++){
		for(j=0 ; j < GE.Width ; j++){
			if(GE.GameField[j][i]==SNAKEHEAD)
				_RPT1(_CRT_WARN,"%d",((Snake*)GE.GameFieldObj[j][i])->Num);
			else if(GE.GameField[j][i]==SNAKEBODY)
				_RPT1(_CRT_WARN,"%c",(((Snake*)GE.GameFieldObj[j][i])->IsCom) ? '8':'N');
			else
				_RPT0(_CRT_WARN,(GE.GameField[j][i]==BRANCH) ? "B":"*");
		}
		_RPT0(_CRT_WARN,"\n");
	}
	_RPT0(_CRT_WARN,"####################################################################################################\n");
	_RPT0(_CRT_WARN,"\n");
}
