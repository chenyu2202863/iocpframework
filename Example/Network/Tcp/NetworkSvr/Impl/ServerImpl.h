#ifndef __SERVICE_HPP
#define __SERVICE_HPP



#include "../../../../../include/Network/TCP.hpp"
#include "../../../../../include/Timer/Timer.hpp"


#include "../CRC32.hpp"

using namespace async::network;
using namespace async::timer;

volatile long g_ClientNum = 0;


class Session
	: public std::tr1::enable_shared_from_this<Session>
{
private:
	Tcp::Socket socket_;
	std::vector<char> buf_;

	async::iocp::CallbackType readCallback_;
	async::iocp::CallbackType writeCallback_;

public:
	explicit Session(const SocketPtr &sock)
		: socket_(sock)
	{
		buf_.resize(1024 * 1024 + sizeof(size_t));
		socket_.IOControl(NonBlockingIO(true));
		::InterlockedIncrement(&g_ClientNum);
	}
	~Session()
	{
		//Stop();
		::InterlockedDecrement(&g_ClientNum);
	}


public:
	Tcp::Socket& GetSocket()
	{
		return socket_;
	}

	void Start()
	{
		try
		{		
			readCallback_	= std::tr1::bind(&Session::_HandleRead, shared_from_this(), _Size, _Error);
			writeCallback_	= std::tr1::bind(&Session::_HandleWrite, shared_from_this(), _Size, _Error);

			AsyncRead(socket_, Buffer(buf_), TransferAll(), readCallback_);

		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Stop()
	{
		socket_.Close();

		readCallback_	= 0;
		writeCallback_	= 0;
	}

private:
	void _HandleRead(u_long bytes, u_long error)
	{
		try
		{
			if( bytes == 0 )
			{
				socket_.AsyncDisconnect(std::tr1::bind(&Session::_DisConnect, shared_from_this()));
				return;
			}

			size_t crc = algorithm::crc::cac_crc32(&buf_[sizeof(size_t)], bytes - sizeof(size_t));
			size_t dst = *(size_t *)(&buf_[0]);
			assert(crc == dst);

			AsyncWrite(socket_, Buffer(buf_), TransferAll(), writeCallback_);
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
			AsyncRead(socket_, Buffer(buf_), TransferAtLeast(1), readCallback_);
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
		struct ObjectFactory< Session >
		{
			typedef async::memory::FixedMemoryPool<true, sizeof(Session)>	PoolType;
			typedef ObjectPool< PoolType >									ObjectPoolType;
		};
	}
}


template<typename T>
struct NoneDeletor
{
	void operator()(T *)
	{}
};


inline SessionPtr CreateSession(const SocketPtr &socket)
{
	return SessionPtr(ObjectAllocate<Session>(socket), &ObjectDeallocate<Session>);
	//return SessionPtr(new Session(io, socket));
}



class Server
{
private:
	IODispatcher &io_;
	Tcp::Accpetor acceptor_;
	std::auto_ptr<Timer> timer_;

public:
	Server(IODispatcher &io, short port)
		: io_(io)
		, acceptor_(io_, Tcp::V4(), port, INADDR_ANY)
	{}

	~Server()
	{
		//_StopServer();
	}

public:
	void Start()
	{
		timer_.reset(new Timer(io_, 2000, 0, std::tr1::bind(&Server::_OnTimer, this)));
		timer_->AsyncWait();

		_StartAccept();
	}

	void Stop()
	{
		_StopServer();
	}

private:
	void _StartAccept()
	{		
		try
		{
			acceptor_.AsyncAccept(0, 
				std::tr1::bind(&Server::_OnAccept, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2/*std::tr1::cref(acceptSock)*/));
		} 
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _StopServer()
	{
		acceptor_.Close();
	}

	void _OnTimer()
	{
		std::cout << "Clients: " << g_ClientNum << std::endl;
	}

private:
	void _OnAccept(u_long error, const SocketPtr &acceptSocket)
	{
		if( error != 0 )
			return;

		try
		{
			SessionPtr session(CreateSession(acceptSocket));
			session->Start();
	
			_StartAccept();
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
};



#endif