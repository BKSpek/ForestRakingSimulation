#pragma once
#include <SFML/Graphics.hpp>

struct ForestTile
{
	int leafAmount = 0;
};

struct TileSprite
{
	sf::Text text;
	sf::RectangleShape rect;
};

class ForestBoard
{
public:
	void drawTile(int row, int col); //update the tile graphics
	void drawBoard(); //update every tile on the board

	void display();

	ForestTile* getForestTile(int row, int col);
	bool isValidTile(int row, int col);

	ForestBoard(int height, int width);
	~ForestBoard();
private:
	bool isValidTile(int row, int col, std::string funcName);
	void handleInputEvents();

	int width, height;
	ForestTile** board;
	sf::RenderWindow window;

	TileSprite tileSprite;
};