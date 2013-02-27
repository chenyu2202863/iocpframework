// BlockClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <network/tcp.hpp>


using namespace async;

int _tmain(int argc, _TCHAR* argv[])
{

	try
	{
		iocp::io_dispatcher io(1);

		network::tcp::socket sck(io, network::tcp::v4());
		sck.connect(network::ip_address::parse("127.0.0.1"), 5050);

		char request[1024] = {0};
		std::cin.getline(request, 1024);
		size_t requestLen = ::strlen(request);
		iocp::auto_buffer_ptr buf(iocp::make_buffer(request, requestLen));
		size_t sendLen = sck.write(iocp::const_buffer(buf->data(), buf->size()));


		char reply[1024] = {0};
		iocp::auto_buffer_ptr recvBuf(iocp::make_buffer(reply));
		size_t replyLen = sck.read(iocp::mutable_buffer(recvBuf->data(), recvBuf->capacity()));
		recvBuf->resize(replyLen);

		std::cout.write(reply, replyLen);
		std::cout << std::endl;
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	

	system("pause");
	return 0;
}

