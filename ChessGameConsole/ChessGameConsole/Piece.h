#pragma once
#include "PieceType.h"
class Piece
{
public:
	bool isWhite;

	int moveCount;

	bool IsWhite();

	Type GetType();

	void SetType(Type);

	void SetWhite(bool);

	int GetMoveCount();

	void SetMoveCount(int);

	Type type;

	Piece();
	Piece(bool, Type);	// Constructor
	~Piece();	// Destructor
};

