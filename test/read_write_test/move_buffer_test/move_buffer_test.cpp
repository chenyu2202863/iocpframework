// move_buffer_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "../../../include/async_io/network.hpp"
#include "../../../include/serialize/serialize.hpp"

using namespace async;
using namespace network;




int _tmain(int argc, _TCHAR* argv[])
{
	server svr(5050);
	char buf[30] = {0};	
	

	svr.register_accept_handler([&buf](const session_ptr &session, const std::string &ip)->bool
	{
		std::cout << "accept remote: " << ip << std::endl;

		
		auto recv_handler = [&buf](const session_ptr &session, std::uint32_t size)
		{
			serialize::i_serialize in(buf, size);
			int valxx = 0;
			std::string val2xx;
			std::vector<char> val3xx;

			in >> valxx >> val2xx >> val3xx;
			
			session->async_write([](const session_ptr &session, std::uint32_t size)
			{
				std::cout << "ok" << size << std::endl;
			}, std::allocator<char>(), std::move(valxx), std::move(val2xx), std::move(val3xx));
		};
		
		session->async_read_some(std::move(service::mutable_buffer_t(buf, sizeof(buf))), 4, std::move(recv_handler));

		
		return true;
	});

	svr.register_disconnect_handler([](const session_ptr &session)
	{
		std::cout << "disconnect remote: " << session->get_ip() << std::endl;
	});

	svr.register_error_handler([](const session_ptr &session, const std::string &msg)
	{
		std::cout << msg << std::endl;
	});


	svr.start();

	service::io_dispatcher_t io([](const std::string &err){ std::cerr << err << std::endl; }, 1);
	client cli(io);
	bool suc = cli.start("127.0.0.1", 5050);
	if( !suc )
		return -1;

	int header_len = 10;
	std::string val2 = "123456";
	std::vector<char> val3(10, 'a');

	
	char recv_buf[35] = {0};
	cli.async_recv(recv_buf, sizeof(recv_buf), [](std::uint32_t size)
	{

	});


	int valxx = 10;
	std::string val2xx = "123";
	std::vector<char> val3xx(10, 'a');


	cli.async_send([](std::uint32_t size)
	{
	
	}, std::allocator<char>(), std::move(valxx), std::move(val2xx), std::move(val3xx));
	

	system("pause");

	cli.stop();
	svr.stop();
	return 0;
}

