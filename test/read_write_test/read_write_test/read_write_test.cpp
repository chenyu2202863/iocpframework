// read_write_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <memory>
#include <cstdint>
#include <vector>
#include <WinSock2.h>

#include "../../../include/async_io/service/read.hpp"
#include "../../../include/async_io/service/write.hpp"
#include "../../../include/async_io/service/read_write_buffer.hpp"
#include "../../../include/async_io/service/dispatcher.hpp"

#include "../../../include/serialize/serialize.hpp"
#include "multi_msg_decoder.hpp"

#ifdef min
#undef min
#endif

using namespace async;



class mock_socket_t
{
	service::io_dispatcher_t io_;
	std::vector<char> buf_;
	std::uint32_t cur_pos_;

public:
	mock_socket_t()
		: io_([](const std::string &){})
		, cur_pos_(0)
	{
		for(char i = 0; i != 32; ++i)
			buf_.push_back('A' + i);
	}

public:
	template < typename BufferT, typename HandlerT, typename AllocatorT >
	void async_read(BufferT &buffer, HandlerT &&handler, const AllocatorT &allocator)
	{
		io_.post(std::bind(&mock_socket_t::handle_read_5<BufferT, HandlerT>, this, buffer, std::move(handler)), allocator);
	}

	template < typename BufferT, typename HandlerT, typename AllocatorT >
	void async_write(const BufferT &buffer, HandlerT &&handler, const AllocatorT &allocator)
	{
		io_.post(std::bind(&mock_socket_t::handle_write<BufferT, HandlerT>, this, buffer, std::move(handler)), allocator);
	}


	template < typename BufferT, typename HandlerT >
	void handle_read_1(BufferT &buffer, HandlerT &handler)
	{
		serialize::mem_serialize in(buffer.data_ + cur_pos_, buffer.size_ - cur_pos_);
		in << buf_;

		handler(std::error_code(0, std::system_category()), in.in_length());
	}

	template < typename BufferT, typename HandlerT >
	void handle_read_2(BufferT &buffer, HandlerT &handler)
	{
		serialize::mem_serialize in(buffer.data_, buffer.size_);

		std::uint32_t handle_len = 10;

		if( cur_pos_ == 0 )
			in << buf_.size();

		std::uint32_t min_len = std::min(buf_.size() - cur_pos_, handle_len);

		in.push_pointer(buf_.data() + cur_pos_, min_len);
		cur_pos_ += min_len;

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;

		handler(std::error_code(0, std::system_category()), in.in_length());
	}

	template < typename BufferT, typename HandlerT >
	void handle_read_3(BufferT &buffer, HandlerT &handler)
	{
		serialize::mem_serialize in(buffer.data_, buffer.size_);

		if( cur_pos_ == 0 )
		{
			in << 15;
			in.push_pointer(buf_.data(), 15);
			in << 15;
			in.push_pointer(buf_.data(), 12);

			cur_pos_ = in.in_length();
		}
		else
		{
			in.push_pointer(buf_.data() + 12, 3);
			cur_pos_ = 0;
		}

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;

		handler(std::error_code(0, std::system_category()), in.in_length());
	}

	template < typename BufferT, typename HandlerT >
	void handle_read_4(BufferT &buffer, HandlerT &handler)
	{
		serialize::mem_serialize in(buffer.data_, buffer.size_);

		if( cur_pos_ == 0 )
		{
			in << std::uint16_t(32);
			cur_pos_ = in.in_length();
		}
		else
		{
			in << std::uint16_t(0);
			in.push_pointer(buf_.data(), 32);
			cur_pos_ = 0;
		}


		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;

		handler(std::error_code(0, std::system_category()), in.in_length());
	}


	template < typename BufferT, typename HandlerT >
	void handle_read_5(BufferT &buffer, HandlerT &handler)
	{
		serialize::mem_serialize in(buffer.data_, buffer.size_);

		if( cur_pos_ == 0 )
		{
			in << std::uint16_t(32);
			cur_pos_ = in.in_length();
		}
		else if( cur_pos_ == 2 )
		{
			in << std::uint16_t(0);
			in.push_pointer(buf_.data(), 16);
			cur_pos_ = 16;
		}
		else if( cur_pos_ == 16 )
		{
			in.push_pointer(buf_.data() + cur_pos_, 15);
			cur_pos_ = in.in_length();
		}
		else if( cur_pos_ == 15 )
		{
			in.push_pointer(buf_.data() + cur_pos_, 1);
			cur_pos_ = 0;

			in << std::uint16_t(32);
			in << std::uint16_t(0);
			in.push_pointer(buf_.data(), 32);
			cur_pos_ = 0;

			in << std::uint32_t(32);
			in.push_pointer(buf_.data(), 18);
			cur_pos_ = 22;
		}
		else if( cur_pos_ == 22 )
		{
			in.push_pointer(buf_.data(), 14);
			cur_pos_ = 0;
		}

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;

		handler(std::error_code(0, std::system_category()), in.in_length());
	}

	template < typename BufferT, typename HandlerT >
	void handle_write(const BufferT &buffer, HandlerT &handler)
	{
		handler(std::error_code(), buffer.size());
	}
};

template < typename SocketT, std::uint32_t N, typename DecoderT >
void read(SocketT &sock, char (&buffer)[N], DecoderT &decoder)
{
	sock.async_read(async::service::buffer(buffer), [&](const std::error_code &, std::uint32_t sz)
	{
		decoder.handle(sock, buffer, sz);
		read(sock, buffer, decoder);
	}, std::allocator<char>());
}


void read_test()
{
	mock_socket_t sock;

	typedef baimo::network::multi_buffer_decode_t<mock_socket_t> msg_decoder_t;
	msg_decoder_t buffer_decoder([](const mock_socket_t &, const buffer_t &buffer)
	{
		
	});

	char buffer[4096] = {0};
	read(sock, buffer, buffer_decoder);
	
	std::cin.get();
}

void write_test()
{
	mock_socket_t sock;
	char buf[1024] = {"test"};

	service::async_write(sock, service::buffer(buf), service::transfer_all(), 
		[](const std::error_code &err, std::uint32_t sz)
	{
		std::cout << __FUNCTION__ << std::endl;
	}, std::allocator<char>());


	//for(std::uint32_t i = 0; i != 100000; ++i)
	{
		service::async_write(sock, service::buffer("test"), service::transfer_all(), 
			[](std::error_code err, std::uint32_t sz)
		{
			//std::cout << __FUNCTION__ << std::endl;
		}, std::allocator<char>());
	}

	system("pause");
};

int _tmain(int argc, _TCHAR* argv[])
{
	read_test();
	write_test();




	system("pause");
	return 0;
}

