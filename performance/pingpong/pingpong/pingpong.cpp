// pingpong.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "pingpong_svr.hpp"
#include "pingpong_cli.hpp"

int main(int argc, char* argv[])
{
	if( *argv[1] == 's' )
	{
		if( argc < 4 )
		{
			std::cerr << "usage: <s> <port> <threads> <block_size>" << std::endl;
			return -1;
		}

		svr_start(argv + 2);
	}
	else if( *argv[1] == 'c' )
	{
		if( argc < 4 )
		{
			std::cerr << "usage: <c> <ip> <port> <sessions> <threads> <block_size> <timeout>" << std::endl;
			return -1;
		}

		client_start(argv + 2);
	}

	system("pause");
	return 0;
}

