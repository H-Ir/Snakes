#include "GameEngine.h"

char FMush_Ch[]={FMUSH_CH1,FMUSH_CH2};
char HMush_Ch[]={HMUSH_CH1,HMUSH_CH2};

// Creates an instance of Mushroom Structure
Mushroom *MU_Create(MushType Type,COORD Position,int Index){
	Mushroom *Mu=(Mushroom *)malloc(sizeof(Mushroom));
	GameObjs GFT;//Type of mushroom that we need for GE.GameField
	Mu->Index=Index;
	Mu->Position=Position;
	Mu->Type=Type;
	switch(Type){
	case Normal:
		Mu->Shape=MUSH_CH;
		GFT=MUSH;
		break;
	case Poison:
		Mu->Shape=PMUSH_CH;
		GFT=PMUSH;
		break;
	case Fast:
		Mu->Shape=FMUSH_CH1;
		GFT=FMUSH;
		break;
	case Half:
		Mu->Shape=HMUSH_CH1;
		GFT=HMUSH;
		break;
	}

	//Enter Mushroom position to GE.GameField & GE.GameFieldObj: 
	GE_SetGameField(Position.X,Position.Y,GFT,Mu);
	return Mu;
}

// Draws Specified Mushroom on the Console win
void MU_Draw(Mushroom *Mu){
	switch(Mu->Type){//first Set Color of Mushrooms::
	case Poison:
		SetFColor(Colors[GE.ColorNum-1][0],Colors[GE.ColorNum-1][4]);
		break;
	case Fast:
		SetFColor(Colors[GE.ColorNum-1][1],Colors[GE.ColorNum-1][4]);
		break;
	case Half:
		SetFColor(Colors[GE.ColorNum-1][2],Colors[GE.ColorNum-1][4]);
		break;
	}
		
	GE_SDraw(Mu->Shape, Mu->Position.X, Mu->Position.Y);
	
	SetFColor(Colors[GE.ColorNum-1][3],Colors[GE.ColorNum-1][4]);//now return Font Color to its Default value

	switch(Mu->Type){//Switch FMush and HMush Shapes(switch between two dedicated char):
	case Fast:
		if(Mu->Shape==FMUSH_CH1)
			Mu->Shape=FMUSH_CH2;
		else
			Mu->Shape=FMUSH_CH1;
		break;
	case Half:
		if(Mu->Shape==HMUSH_CH1)
			Mu->Shape=HMUSH_CH2;
		else
			Mu->Shape=HMUSH_CH1;
		break;
	default:
		break;
	}
}
