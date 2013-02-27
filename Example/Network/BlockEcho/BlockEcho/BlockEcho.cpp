// BlockEcho.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <network/tcp.hpp>
#include <iocp/buffer.hpp>
#include <multi_thread/thread.hpp>


const size_t MaxLenth = 1024;

using namespace async;

class sync_socket
{
	iocp::io_dispatcher &io_;
	u_short port_;
	network::tcp::accpetor accpetor_;
	
	bool is_shutdown_;	// std::atomic<bool>

public:
	sync_socket(iocp::io_dispatcher &io, u_short port)
		: io_(io)
		, port_(port)
		, accpetor_(io_, network::tcp::v4(), port_)
		, is_shutdown_(false)
	{}

	void start()
	{
		try
		{
			while(!is_shutdown_)
			{
				auto sock = accpetor_.accept();

				std::shared_ptr<multi_thread::thread_impl_ex> thr(new multi_thread::thread_impl_ex);
				thr->register_callback(std::bind(&sync_socket::_handle, this, sock, thr));
				thr->start();
			}
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			stop();
		}
	}

	void stop()
	{
		is_shutdown_ = true;
		accpetor_.close();
	}

	DWORD _handle(network::socket_handle_ptr sck, std::shared_ptr<multi_thread::thread_impl_ex> thr)
	{
		try
		{
			char data[MaxLenth] = {0};
			iocp::auto_buffer_ptr buffer(iocp::make_buffer(data));
			
			while(!is_shutdown_)
			{
				size_t len = sck->read(iocp::mutable_buffer(buffer->data(), buffer->size()), 0);
				buffer->resize(len);
				if( len == 0 )
					break;

				std::cout.write(buffer->data(), buffer->size()) << std::endl;

				sck->write(iocp::const_buffer(buffer->data(), buffer->size()), 0);
			}
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			stop();
		}

		thr->stop();
		return 0;
	}
};




int _tmain(int argc, _TCHAR* argv[])
{	
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	try
	{
		iocp::io_dispatcher io(1);
		sync_socket sync_svr(io, 5050);

		sync_svr.start();

		system("pause");
		sync_svr.stop();
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	system("pause");
	return 0;
}

