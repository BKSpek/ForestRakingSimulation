#include <vector>
#include <string>
#include <random>
#include <time.h>
#include <iomanip>
#include <utility> 
#include <iostream>
#include <thread>
#include <chrono>

#include "ForestBoard.h"

//Global parameters
int T = 1000;                           //Maximum runtime of simulation in days.
int raking_frequency = 7;                 //Raking cycle in days.
double raking_amount = 0.0001;            //Volume of leaf removed at each raking cycle. Value between 0 - 1.
double nutrient_depletion_rate = 0.001;   //Amount of nutrients depleted from each forest block per day.
double average_leaf_fall = 0.015;         //Average daily leaf fall.
double seasonal_leaf_fall_inc = 0.001;    //Seasonal impact on average leaf fall. 0.001, 0.001, 0.005 and 0.000 for spring, summer, fall & winter, respectively.
double seasonal_leaf_growth_inc = 0.001;  //Seasonal impact on average leaf growth. 0.005, 0.001, 0.000 and 0.000 for spring, summer, fall & winter, respectively.
double average_leaf_growth = 0.008;       //Average daily leaf growth.
int average_fire_duration = 5;            //Avergae length of fire.
int season_length = 91;                   //Length of each season in days.
double p_fire_neighbor_c = 0.050; //Fixed probability increase of catching fire for each corner neighbor on fire. (4*p_fire_neighbor_c + 4*p_fire_neighbor_e <= 0.5)
double p_fire_neighbor_e = 0.075; //Fixed probability increase of catching fire for each edge neighbor on fire. (4*p_fire_neighbor_c + 4*p_fire_neighbor_e <= 0.5)
double p_fire_season = 0.001;     //Fixed probability increase of catching fire by season. 0.001, 0.002, 0.008, 0.004 for spring, summer, fall & winter, respectively.

//Utility Parameters
int rows = 1;                             //Number of rows of forest blocks.
int cols = 1;                             //Number of cols of forest blocks.
int season = 0;                           //Current season. 0, 1, 2, 3 for spring, summer, fall & winter, respectively.

//Initialize random number generator
std::default_random_engine generator;
std::poisson_distribution<int> fire_duration_generator(average_fire_duration);
std::uniform_real_distribution<double> uniform_generator(0, 1);

//Utility Matrices
//Matrices to store indices of corner and edge neighbors. Each matrix stores indices of neighboring nodes in a pair<int, int>, where first = row, second = col.
std::vector<std::vector<std::vector<std::pair<int, int>>>> Neighbors_c(rows, std::vector<std::vector<std::pair<int, int>>>(cols, std::vector<std::pair<int, int>>()));
std::vector<std::vector<std::vector<std::pair<int, int>>>> Neighbors_e(rows, std::vector<std::vector<std::pair<int, int>>>(cols, std::vector<std::pair<int, int>>()));

//Function to resize all matrices to current rows x cols values
void resize_matrices() {
	//Resize rows
	Neighbors_c.resize(rows);
	Neighbors_e.resize(rows);

	//for each row, resize column with default value
	for (int i = 0; i < rows; ++i) {
		Neighbors_c[i].resize(cols);
		Neighbors_e[i].resize(cols);
	};
};

//Function to populate Neighbors_c and Neighbors_e matrices
void update_neighbors() {
	//Iterate over all forest blocks
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			//Depending on block's location on grid, update corner and edge neighbors
			if (i > 0 && j > 0) {
				Neighbors_c[i][j].push_back(std::make_pair(i - 1, j - 1));
				Neighbors_e[i][j].push_back(std::make_pair(i - 1, j));
				Neighbors_e[i][j].push_back(std::make_pair(i, j - 1));
			};
			if (i < (rows - 1) && j < (cols - 1)) {
				Neighbors_c[i][j].push_back(std::make_pair(i + 1, j + 1));
				Neighbors_e[i][j].push_back(std::make_pair(i + 1, j));
				Neighbors_e[i][j].push_back(std::make_pair(i, j + 1));
			};
			if (i < (rows - 1) && j > 0) {
				Neighbors_c[i][j].push_back(std::make_pair(i + 1, j - 1));
			};
			if (i > 0 && j < (cols - 1)) {
				Neighbors_c[i][j].push_back(std::make_pair(i - 1, j + 1));
			};
		};
	};
};

//Function to determine if an absorbing state has been reached. Absorbing states: leaf volume of entire forest = 0 or leaf volume of entire forest = MAX.
bool is_absorbing_state(ForestBoard & board) {
	//Variable to track total leaf volume in forest
	double total_leaf_volume = 0;

	//Iterate over all forest blocks and add leaf volumes
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			total_leaf_volume += board.getForestTile(i, j)->leafVolume;
		};
	};

	//If leaf volume == 0 or MAX, return true. (Note: Adjusted by 0.001 to account for c++ rounding errors)
	if (total_leaf_volume < 0.001 || total_leaf_volume > ((rows*cols) - 0.001)) {
		return true;
	}
	//Else return true
	else {
		return false;
	};
};

//Daily routine to update raking, nutrients and forest fires (Does not include new forest fire generations).
void morning_update(int time, ForestBoard & board) {
	//Checking of raking is required.
	bool raking_required;
	if (time % raking_frequency == 0) {
		raking_required = true;
	}
	else {
		raking_required = false;
	};

	//Iterate over all forest blocks
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {

			//Raking is required. Rake forest block
			if (raking_required == true) {
				//Raking exceeds leaf volume. Rake all leaves
				if (board.getForestTile(i, j)->leafVolume - raking_amount < 0) {
					board.getForestTile(i, j)->leafVolume = 0.0;
				}
				//Leaf volume exceeds raking. Rake = raking_amount
				else {
					board.getForestTile(i, j)->leafVolume = board.getForestTile(i, j)->leafVolume - raking_amount;
				};
			};

			//Nutrient depletion exceeds nutrient volume. deplete all nutrients
			if (board.getForestTile(i, j)->nutrientVolume - nutrient_depletion_rate < 0) {
				board.getForestTile(i, j)->nutrientVolume = 0.0;
			}
			//Nutrient volume exceeds nutrient depletion. Deplete = nutrient_depletion_rate
			else {
				board.getForestTile(i, j)->nutrientVolume = board.getForestTile(i, j)->nutrientVolume - nutrient_depletion_rate;
			};

			//If fire is happening in block, check if scheduled to be done
			if (board.getForestTile(i, j)->isOnFire == true) {
				if (board.getForestTile(i, j)->fireEndTime <= time) {
					board.getForestTile(i, j)->isOnFire = false;
				};
			};

			//If fire is scheduled to start as per previous day, update
			if (board.getForestTile(i, j)->willBeOnFire == true) {
				//Start fire
				board.getForestTile(i, j)->isOnFire = true;
				board.getForestTile(i, j)->willBeOnFire = false;
				//Convert leaf volume to nutrients. Max value 1.0
				if (board.getForestTile(i, j)->nutrientVolume + board.getForestTile(i, j)->leafVolume > 1) {
					board.getForestTile(i, j)->nutrientVolume = 1.0;
				}
				else {
					board.getForestTile(i, j)->nutrientVolume = board.getForestTile(i, j)->nutrientVolume + board.getForestTile(i, j)->leafVolume;
				};
				//Set leaf volume to 0.
				board.getForestTile(i, j)->leafVolume = 0.0;
			};
		};
	};
};

//Check new fire generations
void check_new_fire(int time, ForestBoard & board) {
	//Iterate over all forest blocks
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {

			//Only check if forest block is currently not under fire
			if (board.getForestTile(i, j)->isOnFire == false) {
				//Probability constributions from neighboring blocks
				double p_fire_neighbor = 0.0;

				//Check if corner neighbors are on fire. If yes, increment probability of catching fire
				for (int k = 0; k < Neighbors_c[i][j].size(); ++k) {
					int neighbor_i = Neighbors_c[i][j][k].first;
					int neighbor_j = Neighbors_c[i][j][k].second;
					if (board.getForestTile(neighbor_i, neighbor_j)->isOnFire == true) {
						p_fire_neighbor += p_fire_neighbor_c;
					};
				};

				//Check if edge neighbors are on fire. If yes, increment probability of catching fire
				for (int k = 0; k < Neighbors_e[i][j].size(); ++k) {
					int neighbor_i = Neighbors_e[i][j][k].first;
					int neighbor_j = Neighbors_e[i][j][k].second;
					if (board.getForestTile(neighbor_i, neighbor_j)->isOnFire == true) {
						p_fire_neighbor += p_fire_neighbor_e;
					};
				};

				//Calculate probability contribution from leaf volume
				double p_fire_leaf = board.getForestTile(i, j)->leafVolume * 0.25;
				//Calculate total probability
				double p_fire = p_fire_season + p_fire_neighbor + p_fire_leaf;

				//Check if fire will start
				double rand_var = uniform_generator(generator);
				if (rand_var <= p_fire) {
					//If fire will start, update to start next day, generate and update duration of fire.
					board.getForestTile(i, j)->willBeOnFire = true;
					int t_fire = fire_duration_generator(generator);
					board.getForestTile(i, j)->fireEndTime = time + t_fire;
				};
			};
		};
	};
};

//Update leaves
void update_leaves(ForestBoard & board) {
	//Random number generators for leaf fall and leaf growth. Multiply by 1000 to generate integer, then divide by 1000 for double.
	std::poisson_distribution<int> leaf_fall_generator(1000 * (average_leaf_fall + seasonal_leaf_fall_inc));
	std::poisson_distribution<int> leaf_growth_generator(1000 * (average_leaf_growth + seasonal_leaf_growth_inc));

	//Iterate over all forest blocks
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			//Only update if forest block is currently not under fire
			if (board.getForestTile(i, j)->isOnFire == false) {
				//New leaf fall and growths
				double new_leaf_fall = (double)leaf_fall_generator(generator) / 1000;
				double new_leaf_growth = (double)leaf_growth_generator(generator) / 1000;
				//Change in leaf volume
				double change_in_leaf = new_leaf_growth + new_leaf_fall;
				//Update leaf volume in block. Leaf volume exceeds max. Set to 1.
				if (board.getForestTile(i, j)->leafVolume + change_in_leaf > 1.0) {
					board.getForestTile(i, j)->leafVolume = 1.0;
				}
				//Leaf volume below min. Set to 0.
				else if (board.getForestTile(i, j)->leafVolume + change_in_leaf < 0.0) {
					board.getForestTile(i, j)->leafVolume = 0.0;
				}
				//Leaf volume between min and max. Update by change_in_leaf.
				else {
					board.getForestTile(i, j)->leafVolume = board.getForestTile(i, j)->leafVolume + change_in_leaf;
				};
			};
		};
	};
};

//Utility function to print matrix of doubles
void print_double_matrix(std::vector<std::vector<double>> matrix, int num_rows, int num_cols) {
	for (int i = 0; i < num_rows; ++i) {
		for (int j = 0; j < num_cols; ++j) {
			std::cout << std::setprecision(3) << matrix[i][j] << " ";
		};
		std::cout << std::endl;
	};
};

//Utility function to print matrix of bools
void print_bool_matrix(std::vector<std::vector<bool>> matrix, int num_rows, int num_cols) {
	for (int i = 0; i < num_rows; ++i) {
		for (int j = 0; j < num_cols; ++j) {
			std::cout << matrix[i][j] << " ";
		};
		std::cout << std::endl;
	};
};

//Utility function to print matrix of ints
void print_int_matrix(std::vector<std::vector<int>> matrix, int num_rows, int num_cols) {
	for (int i = 0; i < num_rows; ++i) {
		for (int j = 0; j < num_cols; ++j) {
			std::cout << matrix[i][j] << " ";
		};
		std::cout << std::endl;
	};
};

int main() {

	//I/O to retrieve forest size
	std::cout << "Please enter the number of rows in forest: ";
	std::cin >> rows;
	std::cout << "Please enter the number of cols in forest: ";
	std::cin >> cols;

	//seed the generator
	generator.seed(time(0));

	//init the board
	ForestBoard board(rows, cols);

	//Resize and update all utility matrices as per rows and cols input
	resize_matrices();
	update_neighbors();

	//Perform simulation untill max simulation time is reached or an absorbing state is reached
	bool absorbing_state = false;
	int t = 0;                //Variable to keep track of days.
	int season_counter = 0;   //Variable to track current season.
	while (t < T && !absorbing_state) {
		//Determine current season
		if (season_counter == season_length) {
			++season;
			if (season > 3) {
				season = 0;
			};
			season_counter = 0;
		};

		//Change seasonal parameters.
		//Spring
		if (season == 0) {
			seasonal_leaf_fall_inc = 0.001;
			seasonal_leaf_growth_inc = 0.005;
			p_fire_season = 0.001;
		};
		//Summer
		if (season == 1) {
			seasonal_leaf_fall_inc = 0.001;
			seasonal_leaf_growth_inc = 0.001;
			p_fire_season = 0.002;
		};
		//Fall
		if (season == 2) {
			seasonal_leaf_fall_inc = 0.005;
			seasonal_leaf_growth_inc = 0.000;
			p_fire_season = 0.008;
		};
		//Winter
		if (season == 3) {
			seasonal_leaf_fall_inc = 0.000;
			seasonal_leaf_growth_inc = 0.000;
			p_fire_season = 0.004;
		};

		//Rake leaves if required, update nutrient depletion and check if fire is scheduled to start/end
		morning_update(t, board);
		//Update leaf volumes
		update_leaves(board);
		//Check if new fires will start
		check_new_fire(t, board);
		//Check if absorbing states are reached
		absorbing_state = is_absorbing_state(board);

		//TESTING
			//print_double_matrix(L, rows, cols);
			//print_bool_matrix(F, rows, cols);
			//print_bool_matrix(F_nextday, rows, cols);
			//print_int_matrix(F_endtimes, rows, cols);

		//Increment time counters
		++t;
		++season_counter;

		//visualize
		board.drawBoard();
		board.display();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	};

	//keep the app running
	while (1)
	{
		board.handleInputEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
};