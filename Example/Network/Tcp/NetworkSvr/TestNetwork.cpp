// TestNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include "impl/ServerImpl.h"

#include <timer/timer.hpp>



#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


void AsyncPrint()
{
	std::cout << " remote number: " << g_ClientNum << std::endl;
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	iocp::io_dispatcher io;

	try
	{
		Server server(io, 5050);
		server.Start();

		timer::timer_handle time(io, 2000, 0, std::bind(&AsyncPrint));
		time.async_wait();

		system("pause");
		server.Stop();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	system("pause");
	return 0;
}
