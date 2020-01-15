#include <iostream>
#include <vector>
#include <string>
#include <map>
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
	virtual void update(CellVector leftField, CellVector rightField) = 0;
};

class ConsoleView : public GameView {
public:
	virtual void update(CellVector leftField, CellVector rightField) {

	}
};

class Game {
private:
	map<Gamer, CellVector> fieldStorage;
	GameView* display;
public:
};

int main(int argc, char** argv) {

}