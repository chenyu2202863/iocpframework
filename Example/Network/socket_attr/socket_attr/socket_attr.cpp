// socket_attr.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>

#include <iocp/dispatcher.hpp>
#include <network/tcp.hpp>

using namespace async;

int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io(1);
	network::tcp v4 = network::tcp::v4();
	network::socket_handle_ptr sck = network::make_socket(io, v4.family(), v4.type(), v4.protocol());

	network::recv_buffer_size recv_size;
	sck->get_option(recv_size);
	std::cout << *(int *)recv_size.data() << std::endl;
	

	network::send_buffer_size send_size;
	sck->get_option(send_size);
	std::cout << *(int *)send_size.data() << std::endl;

	system("pause");
	return 0;
}

