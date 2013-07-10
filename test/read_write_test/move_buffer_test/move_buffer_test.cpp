// move_buffer_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "../../../include/async_io/network.hpp"

using namespace async;
using namespace network;





struct ret_helper
{
	char buf_[32];
	int n_;

	ret_helper()
		: n_(10)
	{
		std::memset(buf_, 0, _countof(buf_));
	}

	template < typename T >
	operator T &()
	{
		return buf_;
	}

	operator int &()
	{
		return n_;
	}
};



int _tmain(int argc, _TCHAR* argv[])
{
	
	ret_helper ret;
	char (&test_buf)[32] = ret;

	int n = ret;

	server svr(5050);
	char buf[20] = {0};	
	
	int valxx = 10;
	std::string val2xx = "123";
	std::vector<char> val3xx(10, 'a');

	auto param1 = service::make_static_param([](const session_ptr &session)
	{

	}, std::move(valxx), std::move(val2xx), std::move(val3xx));


	

	svr.register_accept_handler([&buf](const session_ptr &session, const std::string &ip)->bool
	{
		std::cout << "accept remote: " << ip << std::endl;

		
		auto recv_handler = [&buf](const session_ptr &session)
		{
			std::cout << buf << std::endl;

			//session->async_write(std::move(param1));

		};
		
		session->async_read(std::move(service::mutable_buffer_t(buf, sizeof(buf))), std::move(recv_handler));

		
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
	async::timer::win_timer_service_t timer_svr(io);

	client cli(io, timer_svr);
	bool suc = cli.start("127.0.0.1", 5050, std::chrono::seconds(3));
	if( !suc )
		return -1;

	int header_len = 10;
	std::string val2 = "123456";
	std::vector<char> val3(10, 'a');

	auto param2 = service::make_static_param([]()
	{
	}, std::move(header_len), std::move(val2), std::move(val3));

	cli.async_send(std::move(param2));

	char recv_buf[15] = {0};
	cli.async_recv(recv_buf, sizeof(recv_buf), []()
	{

	});


	struct custom_data_t
	{
		std::string data1_;
		std::string data2_;

		custom_data_t()
			: data1_("chenyu")
			, data2_("test")
		{}

		custom_data_t(custom_data_t &&rhs)
			: data1_(std::move(rhs.data1_))
			, data2_(std::move(rhs.data2_))
		{}

		service::const_array_buffer_t buffers() const
		{
			service::const_array_buffer_t buffer;
			buffer << service::buffer(data1_)
				<< service::buffer(data2_);

			return buffer;
		}
	}val4xxx;

	auto param3 = service::make_dynamic_param([]()
	{
	}, std::move(val4xxx));
	cli.async_send(std::move(param3));

	

	system("pause");

	cli.stop();
	svr.stop();
	return 0;
}

