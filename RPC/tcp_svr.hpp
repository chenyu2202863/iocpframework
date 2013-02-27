#ifndef __RPC_TCP_SERVER_HPP
#define __RPC_TCP_SERVER_HPP


#include <cstdint>

#include "../include/Network/TCP.hpp"
#include "../include/Timer/Timer.hpp"


namespace async
{
	namespace rpc
	{

		class tcp_svr
		{
		private:
			IODispatcher &io_;
			Tcp::Accpetor acceptor_;

		public:
			Server(IODispatcher &io, std::uint16_t port)
				: io_(io)
				, acceptor_(io_, Tcp::V4(), port, INADDR_ANY)
			{}

			~Server();

		public:
			void Start();
			void Stop();

		private:
			void _StartAccept();
			void _OnAccept(u_long error, const SocketPtr &acceptSocket);
		};

	}
}




#endif