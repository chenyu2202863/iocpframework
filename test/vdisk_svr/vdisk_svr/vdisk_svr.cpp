// vdisk_svr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <async_io\network.hpp>
#include <iostream>

using namespace async;

#define FRAME_START_FLAG				'.mgs'

#define FRAME_CMD_QUERY					0x01
#define FRAME_CMD_READ					0x02
#define FRAME_CMD_INFO					0x04

#define CONNECT_TYPE_USERMAIN			2	

typedef struct VDiskRequest
{
	ULONG			flag;		// FRAME_START_FLAG
	ULONG			length;		// total length.
	ULONG			command;	
	ULONG			status;		

	union
	{
		struct
		{
			ULONG			resv1[8];
		}Param;
		struct
		{
			LARGE_INTEGER	start;
			ULONG			number;
		}Read;
		struct
		{
			LARGE_INTEGER	start;
			ULONG			number;
		}Write;
		struct
		{
			UCHAR			type;		
			DWORD			rspid;		
			DWORD			remoteip;	
		}ConnectInfo;
	};


}VDISKREQUEST, *LPVDISKREQUEST;

typedef struct VDiskAck
{
	ULONG			flag;			// FRAME_START_FLAG
	ULONG			length;			// total length.
	ULONG			command;	
	ULONG			status;		
}VDISKACK, *LPVDISKACK;


struct session_buffer_t
{
	std::vector<char> read_buffer_;
	std::vector<char> write_buffer_;

	typedef memory_pool::sgi_memory_pool_t<true, 256> pool_allocator_t;

	pool_allocator_t read_allocator_;
	pool_allocator_t write_allocator_;
};

void msg_complete(const network::session_ptr &session)
{
	auto buffer = session->additional_data<session_buffer_t *>();
	auto req = (VDISKREQUEST *)(buffer->read_buffer_.data());

	VDISKACK ack = {0};
	ack.command = req->command;
	ack.flag = req->flag;

	switch( req->command )
	{
	case FRAME_CMD_QUERY:
	{
		ack.length = sizeof(VDISKACK) + sizeof(std::uint64_t);
		std::uint64_t length = (100 * 1024ull * 1024 * 1024) / 512;
		session->async_write([](const auto &err, auto size)
		{}, buffer->write_allocator_, ack, length);
	}
	break;

	case FRAME_CMD_READ:
	{
		static std::vector<char> data;
		data.resize(req->Read.number * 512, 0X01);

		ack.length = sizeof(VDISKACK) + data.size();
		session->async_write([](const auto &err, auto size)
		{}, buffer->write_allocator_, ack, data);
	}
	break;

	case FRAME_CMD_INFO:
	{
		ack.length = sizeof(VDISKACK);
		session->async_write([](const auto &err, auto size)
		{}, buffer->write_allocator_, ack);
	}
	break;

	default:
	assert(0);
	break;
	}
}

void read(const network::session_ptr &session)
{
	session_buffer_t *buffer = session->additional_data<session_buffer_t *>();
	if( buffer->read_buffer_.size() < sizeof(VDISKREQUEST) )
		buffer->read_buffer_.resize(sizeof(VDISKREQUEST));

	// header
	network::async_read(session, buffer->read_buffer_.data(), buffer->read_buffer_.size(),
		[&](const network::session_ptr &session, std::uint32_t len)
	{
		// body
		session_buffer_t *buffer = session->additional_data<session_buffer_t *>();
		auto header = (VDISKREQUEST *)buffer->read_buffer_.data();
		auto msg_size = header->length;
		if( buffer->read_buffer_.size() < msg_size )
			buffer->read_buffer_.resize(msg_size);

		if( msg_size = len )
		{
			msg_complete(session);
			read(session);

			return;
		}

		network::async_read(session, buffer->read_buffer_.data() + sizeof(VDISKREQUEST), 
			msg_size - sizeof(VDISKREQUEST),
			[](const network::session_ptr &session, std::uint32_t len)
		{
			msg_complete(session);
			read(session);
		}, buffer->read_allocator_);
	}, buffer->read_allocator_);
}

int main()
{
	network::server svr(5050);

	

	svr.register_accept_handler([](const network::session_ptr &session, const std::string &ip)->bool
	{
		session->additional_data(new session_buffer_t, std::allocator<session_buffer_t *>());
		read(session);

		return true;
	});

	svr.register_disconnect_handler([](const network::session_ptr &session)
	{
		std::cout << "session leave" << std::endl;
	});

	svr.register_error_handler([](const network::session_ptr &session, const std::string &msg)
	{
		std::cout << msg << std::endl;
	});

	svr.start();
	std::cin.get();
	svr.stop();

    return 0;
}

