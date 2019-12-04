#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <time.h>

#include "ForestBoard.h"

int main()
{
	const int rows = 10, cols = 10;
	ForestBoard board(rows, cols);

	std::uniform_int_distribution<int> rand(0,255);
	std::default_random_engine generator;
	generator.seed(time(0));

	while (1)
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				board.getForestTile(i, j)->leafAmount = rand(generator);
				//board.drawTile(i, j);
			}
		}

		board.drawBoard();
		board.display();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	

	std::getchar();

	return 0;
}