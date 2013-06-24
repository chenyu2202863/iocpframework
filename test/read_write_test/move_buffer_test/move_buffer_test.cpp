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

	auto bufferxx = std::make_tuple(std::move(valxx), std::move(val2xx), std::move(val3xx));
	auto paramxx = service::make_static_param(
		std::move(bufferxx), 
		std::move([](const session_ptr &session, std::uint32_t len)
	{

	}));

	svr.register_accept_handler([&buf, &paramxx](const session_ptr &session, const std::string &ip)->bool
	{
		std::cout << "accept remote: " << ip << std::endl;

		auto recv_handler = [&buf, &paramxx](const session_ptr &session, std::uint32_t len)
		{
			std::cout << buf << std::endl;
			session->async_write(std::move(paramxx));
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

	service::io_dispatcher_t io(1);
	async::timer::win_timer_service_t timer_svr(io);

	client cli(io, timer_svr);
	bool suc = cli.start("127.0.0.1", 5050, std::chrono::seconds(3));
	if( !suc )
		return -1;

	int header_len = 10;
	std::string val2 = "123";
	std::vector<char> val3(10, 'a');

	auto buffer = std::make_tuple(header_len, std::move(val2), std::move(val3));
	auto param = service::make_static_param(std::move(buffer), std::move([](std::uint32_t len)
	{
		std::cout << len << std::endl;
	}));

	cli.async_send(std::move(param));

	char recv_buf[15] = {0};
	cli.async_recv(recv_buf, sizeof(recv_buf), [](std::uint32_t len)
	{
	});

	valxx = 2;
	val2xx = "456";
	val3xx.insert(val3xx.begin(), 1024 * 1024 * 100, 'x');

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

	auto dynamic_param = service::make_dynamic_param(
		std::move(std::make_tuple(std::move(valxx), std::move(val2xx), std::move(val3xx), std::move(val4xxx))),
		std::move([](std::uint32_t len)
	{

	}));
	
	cli.async_send(std::move(dynamic_param));

	

	system("pause");

	cli.stop();
	svr.stop();
	return 0;
}

