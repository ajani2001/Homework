#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include <stdexcept>
using namespace std;

enum CellType { EMPTY_CELL, EMPTY_SHOOT_CELL, SHIP_CELL, SHIP_SHOOT_CELL, SELECTED_CELL };
enum ActionType { SELECT_CELL, PLACE_SHIP, SHOOT, CONFIRM };
enum GameStage { SETUP, BATTLE };

using CellVector = vector<CellType>;

class Action {
	ActionType type;
	COORD* targetCell;
public:
	Action(ActionType type, COORD* targetCell = nullptr) :type(type) {
		if (targetCell == nullptr) {
			this->targetCell = nullptr;
		}
		else {
			this->targetCell = new COORD(*targetCell);
		}
	}
	operator ActionType() const {
		return type;
	}
	COORD* getTargetCell() const {
		return targetCell;
	}
	~Action() {
		delete targetCell;
		targetCell = nullptr;
	}
};

class Gamer {
protected:
	string name;
public:
	string getName() const {
		return name;
	}
	virtual Action act(GameStage stage) = 0;
};

class Player : public Gamer {
	enum Side { LEFT, RIGHT, UP, DOWN };
	COORD selectedCell;
	COORD fieldSize;
	void moveSelectedCell(Side direction) {
		switch (direction) {
		case LEFT:
			if (selectedCell.X > 0) --selectedCell.X;
			return;
		case RIGHT:
			if (selectedCell.X < fieldSize.X - 1) ++selectedCell.X;
			return;
		case UP:
			if (selectedCell.Y > 0) --selectedCell.Y;
			return;
		case DOWN:
			if (selectedCell.Y < fieldSize.Y - 1) ++selectedCell.Y;
			return;
		}
	}
public:
	Player(const string& name, COORD fieldSize) : fieldSize(fieldSize) {
		this->name = name;
		selectedCell.X = selectedCell.Y = 0;
	}
	virtual Action act(GameStage stage) {
		HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
		INPUT_RECORD buffer[128];
		DWORD recordsRead;
		for (;;) {
			ReadConsoleInput(inputHandle, buffer, DWORD(128), &recordsRead);
			for (int i = 0; i < recordsRead; ++i) {
				if (buffer[i].EventType == KEY_EVENT && buffer[i].Event.KeyEvent.bKeyDown) {
					switch (buffer[i].Event.KeyEvent.wVirtualKeyCode) {
					case 13:
						if (stage == SETUP) return Action(CONFIRM);
					case 32:
						if (stage == SETUP) return Action(PLACE_SHIP, &selectedCell);
						if (stage == BATTLE) return Action(SHOOT, &selectedCell);
					case 37:
						moveSelectedCell(LEFT);
						return Action(SELECT_CELL, &selectedCell);
					case 38:
						moveSelectedCell(UP);
						return Action(SELECT_CELL, &selectedCell);
					case 39:
						moveSelectedCell(RIGHT);
						return Action(SELECT_CELL, &selectedCell);
					case 40:
						moveSelectedCell(DOWN);
						return Action(SELECT_CELL, &selectedCell);
					}
				}
			}
		}
	}

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
			if (selectedField != 0 && selectedField != 1) throw 7;
			if (selectedCell->X >= fieldSize.X || selectedCell->Y >= fieldSize.Y) throw 8;
			COORD requiredFieldPosition = selectedField == 0 ? field1Pos : field2Pos;
			COORD newPosition; newPosition.X = selectedCell->X + requiredFieldPosition.X + 1; newPosition.Y = selectedCell->Y + requiredFieldPosition.Y + 1;
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
	vector<Gamer*> gamers;
	map<Gamer*, CellVector> fieldStorage;
	GameView* display;
	COORD fieldSize;
	bool fieldIsReady(CellVector field) {
		return true;
	}
	bool gameFinished() {
		return false;
	}
	CellVector getEnemyFieldView(Gamer* enemy) {
		CellVector field = fieldStorage[enemy];
		for (CellType& cell : field) {
			if (cell == SHIP_CELL)cell = EMPTY_CELL;
		}
		return field;
	}
public:
	Game(GameView* display, Gamer* gamer1, Gamer* gamer2, COORD fieldSize) : display(display), fieldSize(fieldSize) {
		fieldStorage[gamer1] = CellVector(fieldSize.X * fieldSize.Y, EMPTY_CELL);
		fieldStorage[gamer2] = CellVector(fieldSize.X * fieldSize.Y, EMPTY_CELL);
		gamers = { gamer1, gamer2 };
	}
	void run(){
		int currentGamerIndex = 0;
		for (int i = 0; i < 2; ++i) {
			COORD selectedCell; selectedCell.X = selectedCell.Y = 0;
			for (bool continueCond = true; continueCond;) {
				Action move = gamers[currentGamerIndex]->act(SETUP);
				switch (move) {
				case SELECT_CELL:
					selectedCell = *move.getTargetCell();
					display->update(fieldStorage[gamers[currentGamerIndex]], CellVector(fieldSize.X * fieldSize.Y, EMPTY_CELL), 0, &selectedCell);
					break;
				case CONFIRM:
					if (fieldIsReady(fieldStorage[gamers[currentGamerIndex]])) continueCond = false;
					break;
				case PLACE_SHIP:
					if (move.getTargetCell() != nullptr) selectedCell = *move.getTargetCell();
					CellType& currentCell = fieldStorage[gamers[currentGamerIndex]][fieldSize.X * selectedCell.Y + selectedCell.X];
					currentCell = currentCell == SHIP_CELL ? EMPTY_CELL : SHIP_CELL;
					display->update(fieldStorage[gamers[currentGamerIndex]], CellVector(fieldSize.X * fieldSize.Y, EMPTY_CELL), 0, &selectedCell);
					break;
				}
			}
			currentGamerIndex = (currentGamerIndex + 1) % 2;
		}
		for (; !gameFinished();) {
			COORD selectedCell; selectedCell.X = selectedCell.Y = 0;
			for (bool continueCond = true; continueCond;) {
				Action move = gamers[currentGamerIndex]->act(BATTLE);
				switch (move) {
				case SELECT_CELL:
					selectedCell = *move.getTargetCell();
					display->update(fieldStorage[gamers[currentGamerIndex]], getEnemyFieldView(gamers[(currentGamerIndex + 1) % 2]), 1, &selectedCell);
					break;
				case SHOOT:
					if (move.getTargetCell() != nullptr) selectedCell = *move.getTargetCell();
					CellType& currentCell = fieldStorage[gamers[(currentGamerIndex + 1) % 2]][fieldSize.X * selectedCell.Y + selectedCell.X];
					switch (currentCell) {
					case SHIP_CELL:
						currentCell = SHIP_SHOOT_CELL;
						break;
					case EMPTY_CELL:
						currentCell = EMPTY_SHOOT_CELL;
						continueCond = false;
						break;
					}
					display->update(fieldStorage[gamers[currentGamerIndex]], getEnemyFieldView(gamers[(currentGamerIndex + 1) % 2]), 1, &selectedCell);
					break;
				}
			}
			currentGamerIndex = (currentGamerIndex + 1) % 2;
		}
	}
};

int main(int argc, char** argv) {
	COORD fieldSize; fieldSize.X = fieldSize.Y = 10;
	COORD field1Pos, field2Pos;
	field1Pos.X = field1Pos.Y = 2;
	field2Pos.X = 15; field2Pos.Y = 2;
	Player p1("P1", fieldSize);
	Player p2("P2", fieldSize);
	ConsoleView display("FieldFrames.txt", field1Pos, field2Pos, fieldSize);
	Game game(&display, &p1, &p2, fieldSize);
	game.run();
	return 0;
}