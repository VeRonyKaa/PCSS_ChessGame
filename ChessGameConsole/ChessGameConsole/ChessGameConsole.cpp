#include "pch.h"
#include <iostream>
#include "Piece.h"
#include <array>
#include <stdio.h>
#include <math.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

Piece chessPieces[8][8]; // 2D array containing the piece objects, represents their placement on the board

bool checkMate = false; 

bool playersTurn = true;
//bool playerIsWhite = true;	// Stores the colour that the player owns

bool invalidAction = false;

SOCKET serverSocket;

//prototypes
void GenerateBoard();
void DisplayBoard();
void MovePiece(int, int, int, int);
void Validation(int, int, int, int);
bool DetectCollision(int, int, int, int);
void UserInput();
string ConvertToString();
void ConvertToObj(string);
void ConnectToServer();
void SendBoardToServer(SOCKET);
void GracefullyEnd();

int main()
{
	GenerateBoard();
	ConnectToServer();
	

	char buf[4096];
	string userInput;

	while (!checkMate) // Game loop
	{
		DisplayBoard();
		UserInput();

		if (!invalidAction) // Only does this when move is valid
		{
			SendBoardToServer(serverSocket);
			playersTurn = !playersTurn;
			if (playersTurn)
				std::cout << "It's now Red's turn" << endl;
			else
				std::cout << "It's now Green's turn" << endl;
		}
		invalidAction = false;
	}
	GracefullyEnd();
	return 0;
}

void GenerateBoard() {	 //	Places the pieces in the initial position
	for (int x = 0; x < 8; x++) //	Goes through chessPieces array
	{
		for (int y = 0; y < 8; y++)
		{
// Rook 
			if (x == 0 && y == 0) { chessPieces[x][y].SetType(Rook); }
			if (x == 7 && y == 0) { chessPieces[x][y].SetType(Rook); }
			if (x == 0 && y == 7) { chessPieces[x][y].SetType(Rook); }
			if (x == 7 && y == 7) { chessPieces[x][y].SetType(Rook); }


// Knight
			if (x == 1 && y == 0) { chessPieces[x][y].SetType(Knight); }
			if (x == 6 && y == 0) { chessPieces[x][y].SetType(Knight); }
			if (x == 1 && y == 7) { chessPieces[x][y].SetType(Knight); }
			if (x == 6 && y == 7) { chessPieces[x][y].SetType(Knight); }


// Bishop
			if (x == 2 && y == 0) { chessPieces[x][y].SetType(Bishop); }
			if (x == 5 && y == 0) { chessPieces[x][y].SetType(Bishop); }
			if (x == 2 && y == 7) { chessPieces[x][y].SetType(Bishop); }
			if (x == 5 && y == 7) { chessPieces[x][y].SetType(Bishop); }


// Queen
			if (x == 3 && y == 0) { chessPieces[x][y].SetType(Queen); }
			if (x == 3 && y == 7) { chessPieces[x][y].SetType(Queen); }


// King
			if (x == 4 && y == 0) { chessPieces[x][y].SetType(King); }
			if (x == 4 && y == 7) { chessPieces[x][y].SetType(King); }


// Pawn
			if (y == 1 || y == 6) { chessPieces[x][y].SetType(Pawn); }


// Empty spaces
			if (y > 1 && y < 6) { chessPieces[x][y].SetType(Empty); }

// Colour assigning 
			if (y == 0 || y == 1) { chessPieces[x][y].SetWhite(true); }
			if (y == 6 || y == 7) { chessPieces[x][y].SetWhite(false); }



		}
	}
}

void DisplayBoard() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // For colouring letters
	int red = 4;
	int green = 10;
	int white = 15;

	// Letters
	std::cout << "    A  B  C  D  E  F  G  H";
	std::cout << endl << endl;

	for (int y = 7; y > -1; y--)
	{
		std::cout << y + 1 << "   ";

		for (int x = 0; x < 8; x++)
		{
			// RED (white)
			if (chessPieces[x][y].IsWhite() == true)
			{
				SetConsoleTextAttribute(hConsole, red);
			}
			// GREEN (black)
			if (chessPieces[x][y].IsWhite() == false)
			{
				SetConsoleTextAttribute(hConsole, green);
			}
			// WHITE (empty) 
			if (chessPieces[x][y].GetType() == Empty)
			{
				SetConsoleTextAttribute(hConsole, white);
			}

			// Prints out letters based on the type of piece
			switch (chessPieces[x][y].GetType())
			{
			case Pawn:
				std::cout << "P  ";
				break;

			case Rook:
				std::cout << "R  ";
				break;

			case Knight:
				std::cout << "H  ";
				break;

			case Bishop:
				std::cout << "B  ";
				break;

			case Queen:
				std::cout << "Q  ";
				break;

			case King:
				std::cout << "K  ";
				break;

			case Empty:
				std::cout << (char)254 << "  ";
				break;

			default:
				break;
			}
			SetConsoleTextAttribute(hConsole, white);
		}
		std::cout << endl;
	}
}

void MovePiece(int chosenX, int chosenY, int moveX, int moveY) { // Moves piece to chosen location by exchanging their attributes
	bool chosenWhite = chessPieces[chosenX][chosenY].IsWhite();
	int moveCount = chessPieces[chosenX][chosenY].GetMoveCount();
	Type chosenType = chessPieces[chosenX][chosenY].GetType();
	chessPieces[chosenX][chosenY].SetType(Empty);
	chessPieces[moveX][moveY].SetType(chosenType);
	chessPieces[moveX][moveY].SetWhite(chosenWhite);
	chessPieces[moveX][moveY].SetMoveCount(moveCount);
}

void Validation(int pieceX, int pieceY, int moveX, int moveY) { // Makes sure the chosen move is valid 
	Type chosenType = chessPieces[pieceX][pieceY].GetType();
	// Movement vector
	int diffX = moveX - pieceX;
	int diffY = moveY - pieceY;

	if (chessPieces[pieceX][pieceY].IsWhite() && !playersTurn) {		// CHECKS IF SELECTED PIECE BELONGS TO PLAYER
		std::cout << "You selected a piece that does not belong to you. Behave! W" << endl;
		invalidAction = true;
		return;
	}

	if (!chessPieces[pieceX][pieceY].IsWhite() && playersTurn) {		// CHECKS IF SELECTED PIECE BELONGS TO PLAYER
		std::cout << "You selected a piece that does not belong to you. Behave! B" << endl;
		invalidAction = true;
		return;
	}

	// Checks if the piece the player is attacking is not his own
	if (chessPieces[pieceX][pieceY].IsWhite() == chessPieces[moveX][moveY].IsWhite() && chessPieces[moveX][moveY].GetType() != Empty) {		// CHECKS IF PLAYER IS TRYING TO DELETE THEIR OWN PIECE
		std::cout << "You are trying to yeet your own piece you piece of poop!" << endl;
		invalidAction = true;
		return;
	}

	// This switch statement checks if the selected move is valid based on the piece type
	switch (chosenType)
	{
	case Empty:
		std::cout << "You choose a empty field, pls try again." << endl;
		invalidAction = true;
		break;
	case Pawn:
		//std::cout << "Moving pawn..." << endl;
		// Simple Move
		if (chessPieces[moveX][moveY].GetType() == Empty)	// New position should be empty
		{
				// White
			if (chessPieces[pieceX][pieceY].IsWhite() == true)
			{
				// Evaluates different conditions based on whether it's the first move
				bool canMove = false;
				if (chessPieces[pieceX][pieceY].moveCount == 0) {	// First move
					if (diffX == 0 && diffY <= 2 && diffY > 0)
						canMove = true;
				}
				else {
					if (diffX == 0 && diffY == 1)
						canMove = true;
				}

				// Move if canMove
				if (canMove)
				{
					// Increments move count
					chessPieces[pieceX][pieceY].SetMoveCount(chessPieces[pieceX][pieceY].GetMoveCount() + 1);

					bool collisionDetected = DetectCollision(pieceX, pieceY, moveX, moveY);

					if (!collisionDetected) {
						MovePiece(pieceX, pieceY, moveX, moveY);

						// If pawn arrives at the opposite side of the board, the player can choose another piece
						if (moveY == 7) {
							string input;
							string pieceNames[] = { "R", "H", "B", "Q", "P" };

							std::cout << "What transformation do you want?" << endl;
							std::cin >> input;

							for (int i = 0; i < 5; i++)
							{
								if (input == pieceNames[i]) {
									switch (i)
									{
									case 0:
										chessPieces[moveX][moveY].SetType(Rook);
										break;
									case 1:
										chessPieces[moveX][moveY].SetType(Knight);
										break;
									case 2:
										chessPieces[moveX][moveY].SetType(Bishop);
										break;
									case 3:
										chessPieces[moveX][moveY].SetType(Queen);
										break;
									case 4:
										chessPieces[moveX][moveY].SetType(Pawn);
										break;

									default:
										std::cout << "STOP TROLLING THE SYSTEM!" << endl;
										invalidAction = true;
										break;
									}
								}
							}
						}

					}
					else {
						std::cout << "Invalid position. Are you dumb? Can you not see that there's someone blocking your path?!" << endl;
						invalidAction = true;
					}
				}
			}

			// Black
			if (chessPieces[pieceX][pieceY].IsWhite() == false)
			{
				// Evaluates different conditions based on whether it's the first move
				bool canMove = false;
				if (chessPieces[pieceX][pieceY].moveCount == 0) {	// First move
					if (diffX == 0 && diffY >= -2 && diffY < 0)
						canMove = true;
				}
				else {
					if (diffX == 0 && diffY == -1)
						canMove = true;
				}

				// Move if canMove
				if (canMove)
				{

					chessPieces[pieceX][pieceY].SetMoveCount(chessPieces[pieceX][pieceY].GetMoveCount() + 1);

					bool collisionDetected = DetectCollision(pieceX, pieceY, moveX, moveY);

					if (!collisionDetected) {
						MovePiece(pieceX, pieceY, moveX, moveY);

						// If pawn arrives at the opposite side of the board, the player can choose another piece
						if (moveY == 0) {
							string input;
							string pieceNames[] = { "R", "H", "B", "Q", "P" };

							std::cout << "What transformation do you want?" << endl;
							std::cin >> input;

							for (int i = 0; i < 5; i++)
							{
								if (input == pieceNames[i]) {
									switch (i)
									{
									case 0:
										chessPieces[moveX][moveY].SetType(Rook);
										break;
									case 1:
										chessPieces[moveX][moveY].SetType(Knight);
										break;
									case 2:
										chessPieces[moveX][moveY].SetType(Bishop);
										break;
									case 3:
										chessPieces[moveX][moveY].SetType(Queen);
										break;
									case 4:
										chessPieces[moveX][moveY].SetType(Pawn);
										break;

									default:
										std::cout << "STOP TROLLING THE SYSTEM!" << endl;
										invalidAction = true;
										break;
									}
								}
							}
						}
					}
					else {
						std::cout << "Invalid position. Are you dumb? Can you not see that there's someone blocking your path?!" << endl;
						invalidAction = true;
					}
				}
				else {
					std::cout << "Desired position is not empty!" << endl;
					invalidAction = true;
				}
			}
		}
		if (chessPieces[moveX][moveY].GetType() != Empty)	// ATTACK!
		{
			if (chessPieces[moveX][moveY].IsWhite() == !chessPieces[pieceX][pieceY].IsWhite())
			{
				if (chessPieces[pieceX][pieceY].IsWhite())	// WHITE
				{
					if (diffY == 1 && (diffX == 1 || diffX == -1))
					{
						MovePiece(pieceX, pieceY, moveX, moveY);
					}
					else {
						std::cout << "Desired position is not empty!" << endl;
						invalidAction = true;
					}
				}
				else {											// BLACK
					if (diffY == -1 && (diffX == 1 || diffX == -1))
					{
						MovePiece(pieceX, pieceY, moveX, moveY);
					}
					else {
						std::cout << "Desired position is not empty!" << endl;
						invalidAction = true;
					}
				}
			}
		}
		break;
	case Rook:
		// One needs to be zero, but not both
		if ((diffY == 0 || diffX == 0) && !(diffY == 0 && diffX == 0)) {

			bool collisionDetected = DetectCollision(pieceX, pieceY, moveX, moveY);

			if (!collisionDetected) {
				if (chessPieces[pieceX][pieceY].IsWhite() == true) {
					MovePiece(pieceX, pieceY, moveX, moveY);
				}
				else {
					MovePiece(pieceX, pieceY, moveX, moveY);
				}
			}
			else {
				std::cout << "Invalid position. Is you dumb? Can you not see that there's someone blocking your path?!" << endl;
			}
		}

		break;
	case Knight:
		// If the movement follows the knight-pattern (2,1 or 1,2)
		if ((abs(diffX) == 1 && abs(diffY) == 2) || (abs(diffX) == 2 && abs(diffY) == 1)) {
			if (chessPieces[pieceX][pieceY].IsWhite() == true) {	// WHITE
				MovePiece(pieceX, pieceY, moveX, moveY);
			}
			else {													// BLACK
				MovePiece(pieceX, pieceY, moveX, moveY);
			}
		}
		break;
	case Bishop:
		// Moves diagonally if the increment is the same on the y and the x axis
		if (abs(diffX) == abs(diffY) && diffX * diffY != 0) {

			bool collisionDetected = DetectCollision(pieceX, pieceY, moveX, moveY);

			if (!collisionDetected) {
				if (chessPieces[pieceX][pieceY].IsWhite() == true) {
					MovePiece(pieceX, pieceY, moveX, moveY);
				}
				else {
					MovePiece(pieceX, pieceY, moveX, moveY);
				}
			}
			else {
				std::cout << "Invalid position. Are you dumb? Can you not see that there's someone blocking your path?!" << endl;
				invalidAction = true;
			}
		}
		break;
	case Queen:
		// If the movement behaviour fits to bishop or rook movement respectively
		if (abs(diffX) == abs(diffY) || ((diffY == 0 || diffX == 0) && !(diffY == 0 && diffX == 0))) {

			bool collisionDetected = DetectCollision(pieceX, pieceY, moveX, moveY);

			if (!collisionDetected) {
				if (chessPieces[pieceX][pieceY].IsWhite() == true) {
					MovePiece(pieceX, pieceY, moveX, moveY);
				}
				else {
					MovePiece(pieceX, pieceY, moveX, moveY);
				}
			}
			else {
				std::cout << "Invalid position. Are you dumb? Can you not see that there's someone blocking your path?!" << endl;
				invalidAction = true;
			}
		}
		break;
	case King:
		if (pow(diffX, 2) + pow(diffY, 2) == 2 || pow(diffX, 2) + pow(diffY, 2) == 1) {
			if (chessPieces[pieceX][pieceY].IsWhite() == true) {
				MovePiece(pieceX, pieceY, moveX, moveY);
			}
			else {
				MovePiece(pieceX, pieceY, moveX, moveY);
			}
		}
		break;
	default:
		break;
	}
}

bool DetectCollision(int pieceX, int pieceY, int moveX, int moveY) {

	bool collision = false;

	int dirMagnitude = (abs(moveX - pieceX) > abs(moveY - pieceY)) ? abs(moveX - pieceX) : abs(moveY - pieceY);	// Quick if-statement: condition ? true-result : false-result
	// Calculates directional vector
	int dirX = (moveX - pieceX) / dirMagnitude;
	int dirY = (moveY - pieceY) / dirMagnitude;

	// Loops through all spaces between the piece and the selected space 
	for (int i = 1; i < dirMagnitude; i++)
	{
		if (chessPieces[pieceX + i * dirX][pieceY + i * dirY].GetType() != Empty) {
			collision = true;
		}
	}

	return collision;
}

void UserInput() {
	string pieceInput;
	string moveInput;
	char letters[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
	char numbers[] = { '1', '2', '3', '4', '5', '6', '7', '8' };
	bool validPiece = false;
	bool validMove = false;
	int xPiece = -1;
	int xMove = -1;

	std::cout << "Make your move! Choose the coordinates of the piece you want to move (Make it uppercase) \n";
	std::cin >> pieceInput;
	std::cout << "Nice! Now choose the coordinates for your desired move (Remember uppercase) \n";
	std::cin >> moveInput;

	if (pieceInput.size() == 2 && moveInput.size() == 2) {
		// PIECE INPUT
		for (int i = 0; i < 8; i++)
		{
			if (pieceInput[0] == letters[i])
			{
				for (int j = 0; j < 8; j++)
				{
					if (pieceInput[1] == numbers[j])
					{
						xPiece = i;
						validPiece = true;
					}
				}
			}
		}
		// MOVE INPUT
		for (int i = 0; i < 8; i++)
		{
			if (moveInput[0] == letters[i]) {
				for (int j = 0; j < 8; j++)
				{
					if (moveInput[1] == numbers[j]) {
						xMove = i;
						validMove = true;
					}
				}
			}
		}
	}
	else {
		std::cout << "Input was invalid. Try again!" << endl;
		invalidAction = true;
		return;
	}

	// Converts a char value into in the equivalent integer value by subtracting 48 (the char value of 0)
	int yPiece = (int)pieceInput.at(1) - 1 - 48;
	int yMove = (int)moveInput.at(1) - 1 - 48;

	if (validPiece == true && validMove == true) {
		Validation(xPiece, yPiece, xMove, yMove);
	}

}

string ConvertToString() {	// Converts 2D array chessPieces into a 1D string
	string boardString;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			switch (chessPieces[i][j].GetType())
			{
			case Pawn:
				if (chessPieces[i][j].IsWhite() == true)
				{
					boardString += "P";
					break;
				}
				else {
					boardString += "p";
					break;
				}
			case Rook:
				if (chessPieces[i][j].IsWhite() == true)
				{
					boardString += "R";
					break;
				}
				else {
					boardString += "r";
					break;
				}
			case Knight:
				if (chessPieces[i][j].IsWhite() == true)
				{
					boardString += "H";
					break;
				}
				else {
					boardString += "h";
					break;
				}
			case Bishop:
				if (chessPieces[i][j].IsWhite() == true)
				{
					boardString += "B";
					break;
				}
				else {
					boardString += "b";
					break;
				}
			case Queen:
				if (chessPieces[i][j].IsWhite() == true)
				{
					boardString += "Q";
					break;
				}
				else {
					boardString += "q";
					break;
				}
			case King:
				if (chessPieces[i][j].IsWhite() == true)
				{
					boardString += "K";
					break;
				}
				else {
					boardString += "k";
					break;
				}

			default: boardString += "E";
				break;
			}
			boardString += "";
		}
	}

	return boardString;
}

void ConvertToObj(string boardString) {	// Sets the values of the chessPieces array to the corresponding value in the 1D-string boardString

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			switch (boardString.at(j + i * 8))
			{
			case 'P':
				chessPieces[i][j].SetType(Pawn);
				chessPieces[i][j].SetWhite(true);
				break;
			case 'p':
				chessPieces[i][j].SetType(Pawn);
				chessPieces[i][j].SetWhite(false);
				break;
			case 'R':
				chessPieces[i][j].SetType(Rook);
				chessPieces[i][j].SetWhite(true);
				break;
			case 'r':
				chessPieces[i][j].SetType(Rook);
				chessPieces[i][j].SetWhite(false);
				break;
			case 'H':
				chessPieces[i][j].SetType(Knight);
				chessPieces[i][j].SetWhite(true);
				break;
			case 'h':
				chessPieces[i][j].SetType(Knight);
				chessPieces[i][j].SetWhite(false);
				break;
			case 'B':
				chessPieces[i][j].SetType(Bishop);
				chessPieces[i][j].SetWhite(true);
				break;
			case 'b':
				chessPieces[i][j].SetType(Bishop);
				chessPieces[i][j].SetWhite(false);
				break;
			case 'Q':
				chessPieces[i][j].SetType(Queen);
				chessPieces[i][j].SetWhite(true);
				break;
			case 'q':
				chessPieces[i][j].SetType(Queen);
				chessPieces[i][j].SetWhite(false);
				break;
			case 'K':
				chessPieces[i][j].SetType(King);
				chessPieces[i][j].SetWhite(true);
				break;
			case 'k':
				chessPieces[i][j].SetType(King);
				chessPieces[i][j].SetWhite(false);
				break;

			default: chessPieces[i][j].SetType(Empty);
				break;
			}
		}
	}
}


// **********************************************************************************************************************************************
// ******* SOURCE: https://bitbucket.org/sloankelly/youtube-source-repository/src/master/cpp/networking/BarebonesClient/Barebones_Client/main.cpp
// **********************************************************************************************************************************************
void ConnectToServer() {

	string ipAddress = "172.20.10.2";			// IP Address of the server
	int port = 54000;						// Listening port # on the server

	// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return;
	}

	// Create socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(serverSocket, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	char buf[4096];
	int bytesReceived = recv(serverSocket, buf, 4096, 0);
	if (bytesReceived > 0)
	{
		// Echo response to console
		cout << "COLOUR> " << string(buf, 0, bytesReceived) << endl;
		string colour = buf;
		if (colour == "W")
		{
			playersTurn = true;
		}
		else
		{
			playersTurn = false;
		}

	}
}

void SendBoardToServer(SOCKET serverSocket) {
	char buf[4096];
	
	string sendString = ConvertToString();

	int sendResult = send(serverSocket, sendString.c_str(), sendString.size() + 1, 0);
	std::cout << "Sent!"<<endl;
	if (sendResult != SOCKET_ERROR)
	{
		std::cout << "Hello" << endl;
		// Wait for response
		ZeroMemory(buf, 4096);
		std::cout << "Magic" << endl;
		int bytesReceived = recv(serverSocket, buf, 4096, 0);
		std::cout << "SDFGHJ" << endl;
		if (bytesReceived > 0)
		{
			// Echo response to console
			cout << "BOARD> " << string(buf, 0, bytesReceived) << endl;
			ConvertToObj(string(buf));
			
		}
	}
}

void GracefullyEnd() {
	// Gracefully close down everything
	closesocket(serverSocket);
	WSACleanup();
}