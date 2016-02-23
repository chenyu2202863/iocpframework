#include <async_io\network.hpp>
#include <iostream>

#include "handler_allocator.hpp"
using namespace async;

struct session_buffer_t
{
	std::vector<char> read_buf_;
	handler_allocator read_allocator_;
	handler_allocator write_allocator_;

	session_buffer_t(std::uint32_t block_size)
	{
		read_buf_.resize(block_size);
	}
};

void read(const network::session_ptr &session)
{
	session_buffer_t *buffer = session->additional_data<session_buffer_t *>();
	network::async_read(session, buffer->read_buf_.data(), buffer->read_buf_.size(), 
		[](const network::session_ptr &session, std::uint32_t len)
	{
		session_buffer_t *buffer = session->additional_data<session_buffer_t *>();
		network::async_write(session, buffer->read_buf_.data(), buffer->read_buf_.size(),
			[](const network::session_ptr &session, std::uint32_t len)
		{
			read(session);
		}, buffer->write_allocator_);
	}, buffer->read_allocator_);
}

void svr_start(char **argv)
{
	network::server svr(std::atoi(argv[0]), std::atoi(argv[1]));

	std::uint32_t block_size = std::atoi(argv[2]);
	svr.register_accept_handler([block_size](const network::session_ptr &session, const std::string &ip)->bool
	{
		session->additional_data(new session_buffer_t(block_size), std::allocator<session_buffer_t *>());
		read(session);

		return true;
	});

	svr.register_disconnect_handler([](const network::session_ptr &session)
	{
	});

	svr.register_error_handler([](const network::session_ptr &session, const std::string &msg)
	{
		std::cout << msg << std::endl;
	});

	svr.start();
	std::cin.get();
	svr.stop();
}