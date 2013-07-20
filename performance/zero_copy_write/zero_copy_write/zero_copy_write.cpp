// zero_copy_write.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <atomic>
#include <memory>
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <array>

#include <utility/performance_counter.hpp>
#include <async_io/network.hpp>
#include <serialize/serialize.hpp>

#include "arg_helper.hpp"

using namespace async;
using namespace network;


static const int cnt = 100000;

typedef std::shared_ptr<char> buffer_data_t;
typedef std::pair<std::uint32_t, buffer_data_t> buffer_t;




buffer_data_t make_buffer(std::uint32_t len)
{
	buffer_data_t data((char *) ::operator new(len), [](char *p){ ::operator delete(p); });

	return data;
}

struct multi_buffer_decode_t
{
	buffer_t tmp_buffer_;
	std::uint32_t tmp_buffer_len_;

	char header_data_[4];
	std::uint32_t header_len_;

	typedef std::function<void(const buffer_t &)> buffer_callback_t;
	buffer_callback_t buffer_callback_;

	multi_buffer_decode_t(const buffer_callback_t &buffer_callback)
		: tmp_buffer_len_(0)
		, header_len_(0)
		, buffer_callback_(buffer_callback)
	{
		
	}

	void handle(const char *buffer, std::uint32_t len)
	{
		if( tmp_buffer_.first == 0 )
		{
			if( len < sizeof(std::uint32_t) )
			{
				std::copy(buffer, buffer + len, header_data_);
				header_len_ = len;
				return;
			}

			std::uint32_t data_len = 0;
			if( header_len_ != 0 )
			{
				auto level_header_len = sizeof(std::uint32_t) - header_len_;
				std::copy(buffer, buffer + level_header_len,
						  stdext::make_unchecked_array_iterator(header_data_ + header_len_));
				data_len = *reinterpret_cast<std::uint32_t *>(header_data_);
				header_len_ = 0;

				buffer += level_header_len;
				len -= level_header_len;
			}
			else
			{
				data_len = *reinterpret_cast<const std::uint32_t *>(buffer);
				buffer += sizeof(std::uint32_t);
				len -= sizeof(std::uint32_t);
			}


			buffer_t buffer_data;
			buffer_data.first = data_len;
			buffer_data.second = make_buffer(data_len);


			if( data_len == len )
			{
				std::copy(buffer, buffer + data_len,
						  stdext::make_unchecked_array_iterator(buffer_data.second.get()));
				buffer_callback_(std::move(buffer_data));
			}
			else if( data_len > len )
			{
				std::copy(buffer, buffer + len,
						  stdext::make_unchecked_array_iterator(buffer_data.second.get()));
				tmp_buffer_len_ = len;
				tmp_buffer_ = buffer_data;
			}
			else
			{
				std::copy(buffer, buffer + data_len,
						  stdext::make_unchecked_array_iterator(buffer_data.second.get()));
				buffer_callback_(buffer_data);

				handle(buffer + data_len, len - data_len);
			}
		}
		else
		{
			auto over_len = tmp_buffer_.first - tmp_buffer_len_;
			std::copy(buffer, buffer + over_len,
					  stdext::make_unchecked_array_iterator(tmp_buffer_.second.get() + tmp_buffer_len_));
			buffer_callback_(tmp_buffer_);
			tmp_buffer_.first = 0;
			tmp_buffer_len_ = 0;

			len -= over_len;
			if( len > 0 )
				handle(buffer + over_len, len);
		}
	}
};

multi_buffer_decode_t buffer_decode([](const buffer_t &buffer)
{ 
	std::string val2;
	std::vector<char> val3;
	std::vector<std::string> val4;
	std::map<int, std::string> val5;
	auto len = 0;

	serialize::mem_serialize os(buffer.second.get(), buffer.first);
	os >> len >> val2 >> val3 >> val4 >> val5;
	//std::cout << "ok" << std::endl;
});

std::array<char, 4192> buffer;

void recv_handler(const session_ptr &session)
{
	session->async_read_some(service::mutable_buffer_t(reinterpret_cast<char *>(buffer.data()), buffer.size()), 
		sizeof(std::uint32_t),
		[](const session_ptr &session, std::uint32_t len)
	{
		buffer_decode.handle(buffer.data(), len);
		recv_handler(session);
	});
}




template < typename ConnectionT, typename ...Args >
void async_send(ConnectionT &connector, Args &&...args)
{
	std::uint32_t len = 0;
	baimo::network::paramter_size(len, args...);

	auto param = service::make_dynamic_param([]()
	{
	}, std::move(len), std::forward<Args>( args )...);

	connector.async_send(std::move(param));
}

int _tmain(int argc, _TCHAR* argv[])
{
	server svr(5050);
	

	svr.register_accept_handler([](const session_ptr &session, const std::string &ip)->bool
	{
		recv_handler(session);
		return true;
	});

	svr.register_disconnect_handler([](const session_ptr &session)
	{
	});

	svr.register_error_handler([](const session_ptr &session, const std::string &msg)
	{
	});

	svr.start();
	
	service::io_dispatcher_t io([](const std::string &err){}, 1);
	async::timer::win_timer_service_t timer_svr(io);

	client cli(io, timer_svr);
	bool suc = cli.start("127.0.0.1", 5050, std::chrono::seconds(3));
	if (!suc)
		return -1;

	
	//std::cout << std::endl << "static param test: " << std::endl;
	//{
	//	
	//	utility::performance_t per;

	//	
	//	for (volatile auto i = 0; i != cnt; ++i)
	//	{
	//		std::string val2 = "abcdefghijklmnopqrstuvwxyz";
	//		std::vector<char> val3(100, 'a');
	//		std::vector<std::pair<std::uint32_t, std::string>> val4(100, {val2.size(), val2});
	//		std::map<int, std::pair<std::uint32_t, std::string>> val5;
	//		val5.insert( {1, {val2.size(), val2}});
	//		val5.insert( {2, {val2.size(), val2}});

	//		auto len = 11;

	//		async_send(cli,
	//				   /* baimo::network::make_arg(std::move(len)),
	//				   baimo::network::make_arg(std::move(val2)),
	//				   baimo::network::make_arg(std::move(val3)),
	//				   baimo::network::make_arg(std::move(val4)),*/
	//				   baimo::network::make_arg(std::move(val5)));
	//	}

	//	per.time(std::cout);
	//}


	std::cout << std::endl << "serialize to array test: " << std::endl;
	{
		utility::performance_t per;

		for( volatile auto i = 0; i != cnt; ++i )
		{
			std::string val2 = "abcdefghijklmnopqrstuvwxyz";
			std::vector<char> val3(100, 'a');
			std::vector<std::string> val4(100, val2);
			auto len = 2 * sizeof(std::uint32_t) + val2.size() + sizeof(int) + val3.size() + sizeof(int) + 3000;


			std::shared_ptr<char> buffer((char *) ::operator new(len + 4), [](char *p)
			{::operator delete(p); });
			serialize::mem_serialize os(buffer.get(), len + 4);

			os << len << len << val2 << val3 << val4;

			cli.async_send(buffer.get(), os.in_length(), [buffer]()
			{});
		}

		per.time(std::cout);
	}

	system("pause");

	svr.stop();
	cli.stop();

	
	return 0;
}

