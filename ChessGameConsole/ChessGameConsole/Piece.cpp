#include "pch.h"
#include "Piece.h"


Piece::Piece(){};

Piece::Piece(bool white, Type type1)	// Constructor
{
	isWhite = white;
	type = type1;
	moveCount = 0;
}

bool Piece::IsWhite()
{
	return isWhite;
}

Type Piece::GetType() {
	return type;
}

void Piece::SetType(Type type2) {
	type = type2;
}

void Piece::SetWhite(bool white) {
	isWhite = white;
}

int Piece::GetMoveCount(){
	return moveCount;
}

void Piece::SetMoveCount(int movecount) {
	moveCount = movecount;
}

Piece::~Piece()	// Deconstructor
{
}
