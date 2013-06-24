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

#ifdef min
#undef min
#endif

using namespace async;


typedef std::shared_ptr<char> buffer_data_t;
typedef std::pair<std::uint32_t, buffer_data_t> buffer_t;
typedef std::shared_ptr<buffer_t> buffer_ptr;

buffer_ptr make_buffer()
{
	return std::make_shared<buffer_t>();
}

buffer_ptr make_buffer(std::uint32_t len)
{
	buffer_data_t data((char *)::operator new(len), [](char *p){ ::operator delete(p); });

	auto buffer = std::make_shared<buffer_t>(len, data);
	return buffer;
}

void reallocate_buffer(std::uint32_t len)
{

}

class mock_socket_t
{
	service::io_dispatcher_t io_;
	std::vector<char> buf_;
	std::uint32_t cur_pos_;

public:
	mock_socket_t()
		: io_(1)
		, cur_pos_(0)
	{
		for(char i = 0; i != 32; ++i)
			buf_.push_back('A' + i);
	}

public:
	template < typename BufferT, typename HandlerT >
	void async_read(BufferT &&buffer, HandlerT &&handler)
	{
		io_.post(std::bind(&mock_socket_t::handle_read_1<BufferT, HandlerT>, this, std::move(buffer), std::move(handler)));
	}

	template < typename BufferT, typename HandlerT >
	void async_write(BufferT &&buffer, HandlerT &&handler)
	{
		io_.post(std::bind(&mock_socket_t::handle_write<BufferT, HandlerT>, this, std::move(buffer), std::move(handler)));
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

		handler(std::error_code(0, std::system_category()), in.in_length());

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;
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


		//in.push_array(buf.data(), buf.size());
		handler(std::error_code(0, std::system_category()), in.in_length());

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;
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


		//in.push_array(buf.data(), buf.size());
		handler(std::error_code(0, std::system_category()), in.in_length());

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;
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

		//in.push_array(buf.data(), buf.size());
		handler(std::error_code(0, std::system_category()), in.in_length());

		if( cur_pos_ == buf_.size() )
			cur_pos_ = 0;
	}

	template < typename BufferT, typename HandlerT >
	void handle_write(const BufferT &buffer, HandlerT &handler)
	{
		handler(std::error_code(0, std::system_category()), 1);
	}
};

void read_test()
{
	struct mock_multi_read_t
	{
		mock_socket_t sock_;
		std::uint32_t cur_data_len_;
		std::uint32_t cur_header_len_;
		std::uint32_t header_offset_;
		char header_data_[4];
		buffer_ptr msg_buffer_;

		std::vector<char> buffer_;
		char *buffer_data_;

		mock_multi_read_t()
		{
			cur_header_len_ = 0;
			cur_data_len_ = 0;
			header_offset_ = 0;
			buffer_.resize(4096);
			buffer_data_ = buffer_.data();
		}

		void start_recv()
		{
			sock_.async_read(service::buffer(buffer_), 
				[this](std::error_code err, std::uint32_t len)
			{
				handle_read(err, len);
			});
		}

		void handle_read(std::error_code err, std::uint32_t len)
		{
			std::uint32_t min_header_len = 0;
			if( cur_data_len_ == 0 &&
				(cur_header_len_ == 0 && len < sizeof(std::uint32_t) ||
				cur_header_len_ != 0 && cur_header_len_ != sizeof(std::uint32_t)) )
			{
				min_header_len = std::min(len, sizeof(std::uint32_t) - cur_header_len_);
				std::memcpy(&header_data_[cur_header_len_], buffer_data_ + header_offset_, min_header_len);
				cur_header_len_ += min_header_len;

				if( cur_header_len_ < sizeof(std::uint32_t) )
					return start_recv();
				else
				{
					len -= min_header_len;
				}
			}

			std::uint32_t need_len = 0;

			buffer_ptr tmp_buffer;
			if( cur_data_len_ != 0 )
			{
				tmp_buffer = msg_buffer_;
				need_len = tmp_buffer->first - cur_data_len_;
			}
			else
			{
				if( cur_header_len_ != 0 )
					std::memcpy(&need_len, header_data_, sizeof(header_data_));
				else
					need_len = *(reinterpret_cast<std::uint32_t *>(buffer_data_));

				tmp_buffer = make_buffer(need_len);
			}

			std::uint32_t header_len = 0;
			if( cur_data_len_ == 0 )
			{
				if( cur_header_len_ == 0)
				{
					len -= sizeof(std::uint32_t);
					header_len += sizeof(std::uint32_t);
				}
				else
				{
					cur_header_len_ = 0;
					header_len += min_header_len;
				}
			}

			if( cur_data_len_ + len == tmp_buffer->first )
			{
				std::memcpy(tmp_buffer->second.get() + cur_data_len_, buffer_data_ + header_len, need_len);
				std::cout.write(tmp_buffer->second.get(), tmp_buffer->first) << std::endl;

				cur_data_len_ = 0;
				header_offset_ = 0;
				return start_recv();
			}
			else if( cur_data_len_ + len > tmp_buffer->first )
			{
				std::memcpy(tmp_buffer->second.get() + cur_data_len_, buffer_data_ + header_len, need_len);
				std::cout.write(tmp_buffer->second.get(), tmp_buffer->first) << std::endl;

				len -= need_len;
				cur_data_len_ = 0;
				buffer_data_ += header_len + need_len;
				handle_read(err, len);
				buffer_data_ = buffer_.data();
				header_offset_ = 0;
			}
			else//cur_len_ + len < tmp_buffer->first
			{
				msg_buffer_ = tmp_buffer;
				std::memcpy(msg_buffer_->second.get() + cur_data_len_, buffer_data_ + header_len, len);

				cur_data_len_ += len;
				header_offset_ = 0;
				return start_recv();
			}

		}
	};

	mock_multi_read_t t;
	t.start_recv();

	std::cin.get();
}

void write_test()
{
	mock_socket_t sock;
	char buf[1024] = {"test"};

	service::async_write(sock, service::buffer(buf), service::transfer_all(), 
		[](std::error_code err, std::uint32_t sz)
	{
		std::cout << __FUNCTION__ << std::endl;
	});

	service::const_array_buffer_t buffer;
	buffer << service::buffer("test")
		<< service::buffer("const array");

	//for(std::uint32_t i = 0; i != 100000; ++i)
	{
		service::async_write(sock, std::move(buffer), service::transfer_all(), 
			[](std::error_code err, std::uint32_t sz)
		{
			//std::cout << __FUNCTION__ << std::endl;
		});
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

