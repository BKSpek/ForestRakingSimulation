#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <set>
#include <algorithm>

double calcSampleVariance(std::vector<int> t_vals)
{
	//either empty or 1 value variance is 0
	if (t_vals.size() < 2) return 0;

	//find the sample mean
	double sample_mean_t_value = double(std::accumulate(t_vals.begin(), t_vals.end(), 0)) / double(t_vals.size());

	//find the sample variance
	double sampleVariance = 0;
	for (auto t : t_vals) sampleVariance += pow(t - sample_mean_t_value, 2);
	sampleVariance = sampleVariance / double(t_vals.size() - 1);
	sampleVariance = sqrt(sampleVariance);
	
	return sampleVariance;
}

int main(int argc, char* argv[])
{
	if (argc < 2) return -1;

	std::string fname = std::string(argv[1]);

	std::ifstream ifile(fname.c_str());
	if (!ifile.is_open())
	{
		std::cout << "can't open file" << std::endl;
		return -1;
	}

	//populate t_vals from file
	std::vector<int> t_vals;
	{
		int fInput;
		while (ifile >> fInput)
		{
			t_vals.push_back(fInput);
		}		
	}
	
	//read intervals from user
	std::set<int> intervals;
	{
		int input = 0;
		while (input >= 0)
		{
			std::cout << "enter interval : ";
			std::cin >> input;
			if (input > 0 && input < 18250)
			{
				intervals.insert(input);
			}
		}
	}
	
	//space it out
	std::cout << std::endl;

	//split into stratum
	std::vector<std::vector<int>> stratums(intervals.size() + 1);
	for (int val = 0; val < t_vals.size(); val++)
	{
		//find which stratem each value goes into
		int stratum = 0;
		for (auto interval : intervals)
		{
			if (t_vals[val] > interval)
			{
				stratum++;
			}
			else
			{
				stratums[stratum].push_back(t_vals[val]);
				break; //break out of the ranged for loop, move on
			}

			//if val belongs in right most bucket
			if (stratum == intervals.size())
			{
				stratums[stratum].push_back(t_vals[val]);
			}
		}
	}

	double stratifiedSampleVariance = 0;
	//for each stratum find the variance
	for (auto & stratum : stratums)
	{
		stratifiedSampleVariance += calcSampleVariance(stratum) * (double(stratum.size()) / double(t_vals.size()));
	}

	//find the sample mean
	double sample_mean_t_value = double(std::accumulate(t_vals.begin(), t_vals.end(), 0)) / double(t_vals.size());

	//define z for 95% confidence interval
	double z = 1.96;

	//compute the confidence interval
	double CI = z * (stratifiedSampleVariance / sqrt(t_vals.size()));

	printf("The mean t for file %s is %f +- %f \n", argv[1], sample_mean_t_value, CI);

	std::ofstream ofile("variance_" + std::string(argv[1]));
	ofile << "The mean t is " << sample_mean_t_value << " +- " << CI << std::endl;
	ofile.close();
}