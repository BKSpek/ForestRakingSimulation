#pragma once
#include <SFML/Graphics.hpp>

struct ForestTile
{
	double leafVolume = 0;
	bool isOnFire = false;
	bool willBeOnFire = false;
	int fireEndTime = 0;
	double nutrientVolume = 0;
};

struct TileSprite
{
	sf::Text text;
	sf::RectangleShape rect;
	sf::Font font;
};

class ForestBoard
{
public:
	void drawTile(int row, int col); //update the tile graphics
	void drawBoard(); //update every tile on the board

	void display();

	ForestTile* getForestTile(int row, int col);
	bool isValidTile(int row, int col);

	void handleInputEvents();

	ForestBoard(int height, int width);
	~ForestBoard();
private:
	bool isValidTile(int row, int col, std::string funcName);
	
	int width, height;
	ForestTile** board;
	sf::RenderWindow window;

	TileSprite tileSprite;
};