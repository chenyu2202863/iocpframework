// asio_pingpong.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "server.hpp"
#include "client.hpp"

int main(int argc, char* argv[])
{
	if( *argv[1] == 's' )
		svr_start(argc - 1, argv + 1);
	else if( *argv[1] == 'c' )
		cli::cli_start(argc - 1, argv + 1);

	system("pause");
	return 0;
}

