#include <iostream>
#include <chrono>
#include <thread>

#include "ForestBoard.h"

int main()
{
	ForestBoard board(20,20);

	while (1)
	{
		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				board.getForestTile(i, j)->leafAmount = i + (j * 10);
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