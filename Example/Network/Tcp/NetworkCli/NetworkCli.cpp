// NetworkCli.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "ClientImpl/ClientImpl.hpp"

#include <atlbase.h>
#include <atlconv.h>


int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	IODispatcher ioService(GetFitThreadNum());
	

	try
	{
		std::string ip;
		if( argc > 1 )
			ip = CT2A(argv[1]);
		else
			ip = "127.0.0.1";

		std::vector<Client *> clis;
		size_t cnt = 1;
		if( argc > 2 )
			cnt = _tstoi(argv[2]);

		clis.resize(cnt);
		for(size_t i = 0; i != cnt; ++i)
			clis.push_back(new Client(ioService, ip.c_str(), 5050));


		system("pause");
	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	
	return 0;
}

