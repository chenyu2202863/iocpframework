// Sock4a.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Tunnel.hpp"

int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io;

	Server svr(io, 5050, "127.0.0.1", 5051);
	svr.Start();

	system("pause");

	svr.Stop();
	return 0;
}

