// Comm.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../../include/Comm/BasicComm.hpp"

#include <iostream>


using namespace async;

int _tmain(int argc, _TCHAR* argv[])
{
	iocp::IODispatcher io;

	try
	{
		comm::BasicComm impl(io, "COM1");
		impl.SetOption(comm::BaudRate(192000));
		impl.SetOption(comm::FlowControl(comm::FlowControl::None));
		impl.SetOption(comm::Parity(comm::Parity::None));
		impl.SetOption(comm::StopBits(comm::StopBits::One));
		impl.SetOption(comm::CharacterSize(8));

		//impl.AsyncRead()
	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}


	system("pause");
	return 0;
}

