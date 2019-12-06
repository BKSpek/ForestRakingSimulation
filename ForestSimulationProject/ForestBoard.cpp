#include "ForestBoard.h"
#include <iostream>
#include <thread>

ForestBoard::ForestBoard(int height, int width) : height(height), width(width)
{
#ifdef VISUALIZE
	//create the window
	window.create(sf::VideoMode(800, 600), "Forest Simulation");
	//window.create(sf::VideoMode(1280, 600), "Forest Simulation");

	//load the font file
	
	if (!tileSprite.font.loadFromFile("../Font/JerseyM54-aLX9.ttf"))
	{
		std::cout << "Failed to load font file" << std::endl;
		abort();
	}

	//init the tileSprite object with the font
	tileSprite.text.setFont(tileSprite.font);
	tileSprite.text.setCharacterSize(12); //20
	tileSprite.text.setFillColor(sf::Color::Black);


	//update the tile size
	auto windowSize = window.getSize();
	float tileWidth = width / windowSize.x;
	float tileHeight = width / windowSize.x;

	tileSprite.rect.setSize(sf::Vector2f(tileWidth, tileHeight));
	tileSprite.rect.setOutlineColor(sf::Color(0, 0, 0));
	tileSprite.rect.setOutlineThickness(4);
#endif

	//create the board
	board = new ForestTile*[height];

	for (int i = 0; i < height; i++)
		board[i] = new ForestTile[width];

#ifdef VISUALIZE
	//draw the board
	drawBoard();

	//display the board
	display();
#endif
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
		//set the tileSprite position
		tileSprite.rect.setPosition(col * tileSprite.rect.getSize().x, row * tileSprite.rect.getSize().y);

		if(board[row][col].isOnFire) //if on fire, color red
			tileSprite.rect.setFillColor(sf::Color(255, 0, 0));
		else if(board[row][col].leafVolume == 0)
			tileSprite.rect.setFillColor(sf::Color(255, 255, 255)); //white
		else //else color shade of green
			tileSprite.rect.setFillColor(sf::Color(0, std::max(int(255 - (255 * board[row][col].leafVolume)), 0), 0));

		//draw the rectangle
		window.draw(tileSprite.rect);

		//update values + draw the text
		tileSprite.text.setString(std::to_string(board[row][col].leafVolume));
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
			exit(0);
		}
	}
}
