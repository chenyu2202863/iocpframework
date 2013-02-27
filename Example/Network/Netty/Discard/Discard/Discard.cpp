// Discard.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <network/tcp.hpp>
#include <timer/timer.hpp>

#include <atlbase.h>
#include <atltime.h>
#include <iostream>
#include <cstdint>
#include <memory>

using namespace async;


// 这些都线程不安全，可以考虑使用std::atomic<std::uint64_t>
std::uint64_t transffered_;
std::uint64_t recevied_;

std::uint64_t oldCounter_;
CTime startTime_;
volatile long connectedCnt_;


class Session
	: public std::enable_shared_from_this<Session>
{
private:
	network::tcp::socket socket_;
	std::vector<char> buf_;

	iocp::rw_callback_type readCallback_;
	iocp::rw_callback_type writeCallback_;

public:
	explicit Session(const network::socket_handle_ptr &sock, size_t bufLen)
		: socket_(sock)
	{
		buf_.resize(bufLen);
		::InterlockedIncrement(&connectedCnt_);
	}
	~Session()
	{
		::InterlockedDecrement(&connectedCnt_);
	}


public:
	network::tcp::socket &GetSocket()
	{
		return socket_;
	}

	void Start()
	{
		try
		{		
			readCallback_	= std::tr1::bind(&Session::_HandleRead, shared_from_this(), iocp::_Size, iocp::_Error);
			writeCallback_	= std::tr1::bind(&Session::_HandleWrite, shared_from_this(), iocp::_Size, iocp::_Error);

			iocp::async_read(socket_, iocp::buffer(buf_), iocp::transfer_at_least(1), readCallback_);

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Stop()
	{
		socket_.close();

		readCallback_	= 0;
		writeCallback_	= 0;
	}

private:
	void _HandleRead(u_long bytes, iocp::error_code error)
	{
		try
		{
			if( bytes == 0 )
			{
				socket_.async_disconnect(std::bind(&Session::_DisConnect, shared_from_this()));
				return;
			}

			recevied_ += bytes;
			transffered_ += bytes;

			async_write(socket_, iocp::buffer(&buf_[0], bytes), iocp::transfer_all(), writeCallback_);
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _HandleWrite(u_long bytes, u_long error)
	{
		try
		{		
			iocp::async_read(socket_, iocp::buffer(buf_), iocp::transfer_at_least(1), readCallback_);
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _DisConnect()
	{
		Stop();
	}
};

typedef std::tr1::shared_ptr<Session> SessionPtr;




// 定制自己的工厂
namespace async
{
	namespace iocp
	{
		template<>
		struct object_factory_t< Session >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(Session)>	PoolType;
			typedef object_pool_t< PoolType >							ObjectPoolType;
		};
	}
}


inline SessionPtr CreateSession(const network::socket_handle_ptr &socket, size_t bufLen)
{
	return SessionPtr(iocp::object_allocate<Session>(socket, bufLen), &iocp::object_deallocate<Session>);
}


class DiscadSvr
{
	iocp::io_dispatcher &io_;
	network::tcp::accpetor acceptor_;
	timer::timer_handle timer_;
	size_t bufLen_;
	
public:
	DiscadSvr(iocp::io_dispatcher &io, u_short port, size_t size)
		: io_(io)
		, acceptor_(io, network::tcp::v4(), port)
		, timer_(io)
		, bufLen_(size)
	{}

	void Start()
	{
		try
		{
			_StartAccept();
			timer_.async_wait(std::bind(&DiscadSvr::_OnTimer, this), 3000, 0);
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

private:
	void _StartAccept()
	{
		network::socket_handle_ptr sck(network::make_socket(io_, 
			network::tcp::v4().family(), network::tcp::v4().type(), network::tcp::v4().protocol()));
		acceptor_.async_accept(sck, std::bind(&DiscadSvr::_OnAccept, this, iocp::_Error, iocp::_Socket));
	}

	void _OnTimer()
	{
		CTime endTime = CTime::GetCurrentTime();
		auto bytes = transffered_ - oldCounter_;
		auto msgs = recevied_;
		recevied_ = 0;

		CTimeSpan span = endTime - startTime_;
		printf("%4.3f MiB/s %4.3f Ki Msgs/s %6.2f bytes per msg\t connected count: %ld\n",
			static_cast<double>(bytes)/span.GetTimeSpan()/1024/1024,
			static_cast<double>(msgs)/span.GetTimeSpan()/1024,
			static_cast<double>(bytes)/static_cast<double>(msgs),
			connectedCnt_);

		oldCounter_ = transffered_;
		startTime_ = endTime;
	}

	void _OnAccept(iocp::error_code error, const network::socket_handle_ptr &acceptSocket)
	{
		if( error != 0 )
			return;

		try
		{
			SessionPtr session(CreateSession(acceptSocket, bufLen_));
			session->Start();

			_StartAccept();
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	iocp::io_dispatcher io;
	size_t msgSize = 512;
	if( argc > 1 )
		msgSize = _tstoi(argv[1]);
	DiscadSvr svr(io, 5050, msgSize);

	svr.Start();

	system("pause");
	return 0;
}

