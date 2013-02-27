// TestNetwork.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "impl/ServerImpl.h"
#include "../../../../include/Timer/Timer.hpp"

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

	IODispatcher ioService(GetFitThreadNum());

	try
	{
		Server server(ioService, 5050);
		server.Start();

		//async::timer::Timer time(ioService, 2000, 0);
		//time.BeginWait(std::tr1::bind(&AsyncPrint));

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
