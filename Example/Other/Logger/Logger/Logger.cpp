// Logger.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "../../../../include/Logger/Logger.hpp"





int _tmain(int argc, _TCHAR* argv[])
{
	
	async::iocp::IODispatcher io(2);

	try
	{
		async::logger::LoggerPtr logger(new async::logger::Logger(io, "Test"));
		logger->UseFile("log.txt");

		logger->Log("Test 1");
		logger->Log("Test 2");
		logger->Log("Test 3");
		logger->Log("Test 4");
		logger->Log("Test 5");

	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}

