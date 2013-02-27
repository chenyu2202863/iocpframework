// FileMonitor.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <iocp/dispatcher.hpp>
#include <file/file_monitor.hpp>


void Monitor(int action, LPCTSTR fileName, size_t len)
{
	std::wcout.imbue(std::locale("chs"));
	std::wcout.write(fileName, len) << " action: " << action << std::endl;
}


int _tmain(int argc, _TCHAR* argv[])
{
	async::iocp::io_dispatcher io(1);

	try
	{
		async::filesystem::change_monitor monitor(io, L"C:/test");

		using namespace std::tr1::placeholders;
		monitor.monitor(std::tr1::bind(&Monitor, _2, _3, _4), true);

		system("pause");
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	system("pause");

	return 0;
}

