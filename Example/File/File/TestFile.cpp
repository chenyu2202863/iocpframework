// File.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Impl/FileImpl.h"

#include <functional>
#include <set>


int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	iocp::io_dispatcher io(1);

	try
	{
		file_async_rw file(io, _T("D:\\VMwareWorkstation_8.0.1.rar"), _T("C:\\test\\VMwareWorkstation_8.0.1.rar"));
		file.start();

		system("pause");
		file.stop();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	

	system("pause");
	return 0;
}

