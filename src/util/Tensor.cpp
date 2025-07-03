#include <util/Tensor.hpp>
#include <string>

using namespace std;

std::runtime_error error::TensorInit(const std::vector<int>& dim)
{
	std::string str;
	for (const auto& i : dim)
	{
		str += '[' + to_string(i) + ']';
	}
	return std::runtime_error{
		"TensorInit Error: Tensor can not have shape of " + str
	};
}

std::runtime_error error::TensorInit(const std::vector<unsigned>& dim)
{
	std::string str;
	for (const auto& i : dim)
	{
		str += '[' + to_string(i) + ']';
	}
	return std::runtime_error{
		"TensorInit Error: Tensor can not have shape of " + str
	};
}

std::runtime_error error::TensorDimIndex(int dimSize, int index)
{
	return std::runtime_error{
		"TensorDimIndex Error: Tensor have only" + to_string(dimSize) + " dimension, index " + to_string(index) +
		" out of bound"
	};
}
