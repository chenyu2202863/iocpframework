// timing_wheel_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include "../../../include/utility/circular_buffer.hpp"

void test_circular_buffer()
{
	utility::cicular_buffer_t<int, 3> buffer;

	{
		buffer.push_back(1);
		std::cout << buffer.back() << std::endl;
	}

	{
		buffer.push_back(2);
		std::cout << buffer.back() << std::endl;
	}

	{
		buffer.push_back(3);
		std::cout << buffer.back() << std::endl;
	}

	{
		buffer.push_back(4);
		std::cout << buffer.back() << std::endl;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{

	

	return 0;
}

