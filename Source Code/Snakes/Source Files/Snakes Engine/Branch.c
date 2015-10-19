#include "GameEngine.h"

// Creates an instance of Branch Structure
Branch *BR_Create(COORD Start,COORD End,Orientation Orient){
	Branch *Br=(Branch *)malloc(sizeof(Branch));
	int i;

	Br->Start=Start;
	Br->End=End;
	Br->Orient=Orient;
	Br->Len = (Orient==Horizontal) ? (End.X - Start.X +1) : (End.Y - Start.Y +1);
	
	//Enter Branch position to GE.GameField & GE.GameFieldObj: 
	if(Orient==Horizontal)
		for(i=Start.X;i<=End.X;i++)
			GE_SetGameField(i,Start.Y,BRANCH,Br);
	else
		for(i=Start.Y;i<=End.Y;i++)
			GE_SetGameField(Start.X,i,BRANCH,Br);
	return Br;
}

// Draws Specified Branch on the Console win
void BR_PreDraw(Branch *Br){
	int i;
	if(Br->Orient==Horizontal)//If Branch Is Horizontal:
		for(i=Br->Start.X;i<=Br->End.X;i++)
			GE_SDraw(BRANCH_CH,i,Br->Start.Y);
	else//If Branch Is Vertical:
		for(i=Br->Start.Y;i<=Br->End.Y;i++)
			GE_SDraw(BRANCH_CH,Br->Start.X,i);
}