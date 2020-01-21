#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include <stdexcept>
using namespace std;

enum CellType { EMPTY_CELL, EMPTY_SHOOT_CELL, SHIP_CELL, SHIP_SHOOT_CELL, SELECTED_CELL };
enum Action { MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN, PLACE_SHIP, SHOOT };

using CellVector = vector<CellType>;

class Gamer {
protected:
	string name;
public:
	string getName() const {
		return name;
	}
	virtual void actSetup() = 0;
	virtual void actGame() = 0;
};

class GameView {
public:
	virtual void update(CellVector field1, CellVector field2, int selectedField = 0, const COORD* selectedCell = nullptr) = 0;
};

class ConsoleView : public GameView {
	HANDLE consoleOutput;
	CHAR_INFO* symbolArray;
	COORD field1Pos;
	COORD field2Pos;
	COORD fieldSize;
	COORD gameSize;
	char getTextureChar(CellType cell) {
		switch (cell) {
		case EMPTY_CELL:
			return ' ';
			break;
		case EMPTY_SHOOT_CELL:
			return '•';
			break;
		case SHIP_CELL:
			return char(178);
			break;
		case SHIP_SHOOT_CELL:
			return 'X';
			break;
		default:
			throw 5;
			return '\0';
			break;
		}
	}
public:
	ConsoleView(const string& fieldFramesFile, COORD field1Pos, COORD field2Pos, COORD fieldSize)
		: field1Pos(field1Pos), field2Pos(field2Pos), fieldSize(fieldSize) {
		consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		ifstream framesFile(fieldFramesFile);
		framesFile.exceptions(ifstream::failbit);
		if (!framesFile.is_open()) throw 1;
		vector<string> vBuffer;
		string sBuffer;
		try {
			for (;;) {
				getline(framesFile, sBuffer);
				vBuffer.push_back(sBuffer);
			}
		}
		catch (ifstream::failure) {}
		if (vBuffer.size() == 0) throw 2;
		for (const string& str : vBuffer) {
			if (str.length() != vBuffer[0].length()) throw 3;
		}
		gameSize.X = vBuffer[0].length();
		gameSize.Y = vBuffer.size();
		symbolArray = new CHAR_INFO[gameSize.X * gameSize.Y];
		if (field1Pos.X + fieldSize.X > gameSize.X ||
			field1Pos.Y + fieldSize.Y > gameSize.Y ||
			field2Pos.X + fieldSize.X > gameSize.X ||
			field2Pos.Y + fieldSize.Y > gameSize.Y) throw 4;
		for (int i = 0; i < gameSize.Y; ++i) {
			for (int j = 0; j < gameSize.X; ++j) {
				symbolArray[i * gameSize.X + j].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
				symbolArray[i * gameSize.X + j].Char.AsciiChar = vBuffer[i][j];
			}
		}
		for (int i = 0; i < fieldSize.Y; ++i) {
			for (int j = 0; j < fieldSize.X; ++j) {
				symbolArray[gameSize.X * (i + field1Pos.Y) + field1Pos.X + j].Attributes
					= symbolArray[gameSize.X * (i + field2Pos.Y) + field2Pos.X + j].Attributes
					= BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
			}
		}
	}
	virtual void update(CellVector field1, CellVector field2, int selectedField = 0, const COORD* selectedCell = nullptr) {
		if (field1.size() != fieldSize.X * fieldSize.Y || field2.size() != fieldSize.X * fieldSize.Y) throw 6;
		for (int i = 0; i < fieldSize.Y; ++i) {
			for (int j = 0; j < fieldSize.X; ++j) {
				symbolArray[gameSize.X * (i + field1Pos.Y) + field1Pos.X + j].Char.AsciiChar = getTextureChar(field1[fieldSize.X * i + j]);
				symbolArray[gameSize.X * (i + field2Pos.Y) + field2Pos.X + j].Char.AsciiChar = getTextureChar(field2[fieldSize.X * i + j]);
			}
		}
		COORD bufferCoord; bufferCoord.X = bufferCoord.Y = 0;
		SMALL_RECT rect; rect.Top = rect.Left = 1; rect.Right = gameSize.X + 1; rect.Bottom = gameSize.Y + 1;
		WriteConsoleOutput(consoleOutput, symbolArray, gameSize, bufferCoord, &rect);
		if (selectedCell != nullptr) {
			if (selectedField != 1 && selectedField != 2) throw 7;
			if (selectedCell->X >= fieldSize.X || selectedCell->Y >= fieldSize.Y) throw 8;
			COORD requiredFieldPosition = selectedField == 1 ? field1Pos : field2Pos;
			COORD newPosition; newPosition.X = selectedCell->X + requiredFieldPosition.X; newPosition.Y = selectedCell->Y + requiredFieldPosition.Y;
			CONSOLE_CURSOR_INFO cursor; cursor.bVisible = TRUE; cursor.dwSize = 100;
			SetConsoleCursorInfo(consoleOutput, &cursor);
			SetConsoleCursorPosition(consoleOutput, newPosition);
		}
		else {
			CONSOLE_CURSOR_INFO cursor; cursor.bVisible = FALSE; cursor.dwSize = 100;
			SetConsoleCursorInfo(consoleOutput, &cursor);
		}
		SMALL_RECT windowRect; windowRect.Top = windowRect.Left = 0; windowRect.Right = gameSize.X - 1; windowRect.Left = gameSize.Y - 1;
		SetConsoleWindowInfo(consoleOutput, TRUE, &windowRect);
	}
	~ConsoleView() {
		delete[] symbolArray;
		symbolArray = nullptr;
	}
};

class Game {
private:
	map<Gamer, CellVector> fieldStorage;
	GameView* display;
public:
};

int main(int argc, char** argv) {
	COORD field1Pos, field2Pos, fieldSize;
	field1Pos.X = 2; field1Pos.Y = 2;
	field2Pos.X = 15; field2Pos.Y = 2;
	fieldSize.X = fieldSize.Y = 10;
	ConsoleView aaaaaaa("FieldFrames.txt", field1Pos, field2Pos, fieldSize);
	CellVector f1(100, CellType(SHIP_CELL));
	CellVector f2(100, CellType(EMPTY_SHOOT_CELL));
	aaaaaaa.update(f1, f2, 1, &field1Pos);
	Sleep(10000);
	return 0;
}