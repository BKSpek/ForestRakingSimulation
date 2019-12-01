#include "ForestBoard.h"
#include <iostream>
#include <thread>

ForestBoard::ForestBoard(int height, int width) : height(height), width(width)
{
	//create the window
	window.create(sf::VideoMode(800, 600), "Forest Simulation");

	//load the font file
	sf::Font font;
	if (!font.loadFromFile("../Font/JerseyM54-aLX9.ttf"))
	{
		std::cout << "Failed to load font file" << std::endl;
		abort();
	}

	//init the tileSprite object with the font
	tileSprite.text.setFont(font);
	tileSprite.text.setCharacterSize(20);


	//update the tile size
	auto windowSize = window.getSize();
	float tileWidth = width / windowSize.x;
	float tileHeight = width / windowSize.x;

	tileSprite.rect.setSize(sf::Vector2f(tileWidth, tileHeight));
	tileSprite.rect.setOutlineColor(sf::Color(0, 0, 0));
	tileSprite.rect.setOutlineThickness(4);

	//create the board
	board = new ForestTile*[height];

	for (int i = 0; i < height; i++)
		board[i] = new ForestTile[width];

	//draw the board
	drawBoard();

	//display the board
	display();
}

ForestBoard::~ForestBoard()
{
	for (int i = 0; i < height; i++)
		delete[] board[i];

	delete[] board;
}

void ForestBoard::drawTile(int row, int col)
{
	if (isValidTile(row, col, "drawTile"))
	{
		//draw the rectangle
		tileSprite.rect.setPosition(col * tileSprite.rect.getSize().x, row * tileSprite.rect.getSize().y);
		tileSprite.rect.setFillColor(sf::Color(0, 255 - (board[row][col].leafAmount / 10), 0));
		window.draw(tileSprite.rect);

		//draw the text
		tileSprite.text.setString(std::to_string(board[row][col].leafAmount));
		tileSprite.text.setPosition(tileSprite.rect.getPosition());
		window.draw(tileSprite.text);
	}

	handleInputEvents();
}

void ForestBoard::drawBoard()
{
	auto windowSize = window.getSize();
	float tileWidth = windowSize.x / width;
	float tileHeight = windowSize.y / height;

	tileSprite.rect.setSize(sf::Vector2f(tileWidth, tileHeight));

	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			drawTile(row, col);
		}
	}

	handleInputEvents();
}

void ForestBoard::display()
{
	window.display();

	handleInputEvents();
}

//returns nullptr if invalid row/col
ForestTile * ForestBoard::getForestTile(int row, int col)
{
	if (isValidTile(row, col, "getForestTile"))
		return &board[row][col];	

	return nullptr;
}

bool ForestBoard::isValidTile(int row, int col)
{
	return (row >= 0 && col >= 0 && row < height && col < width);
}

bool ForestBoard::isValidTile(int row, int col, std::string funcName)
{
	if (!isValidTile(row, col))
	{
		std::cout << "Out of bounds tile r : " << row << " c : " << col
			<< " height : " << height << " width : " << width 
			<< " from function : " << funcName << std::endl;

		return false;
	}
	
	return true;
}

void ForestBoard::handleInputEvents()
{
	sf::Event e;
	while (window.pollEvent(e))
	{
		if (e.type == sf::Event::Closed)
		{
			window.close();
			abort();
		}
	}
}
