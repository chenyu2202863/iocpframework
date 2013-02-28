// 01.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <network/tcp.hpp>


int _tmain(int argc, _TCHAR* argv[])
{
	async::iocp::io_dispatcher io(1);
	

	system("pause");
	return 0;
}

