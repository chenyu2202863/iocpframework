#ifndef __RPC_TCP_CONNECTION_HPP
#define __RPC_TCP_CONNECTION_HPP


#include "../MultiThread/Lock.hpp"

#include <functional>
#include <memory>


namespace async
{
	namespace rpc
	{
		class channel;


		class tcp_connection
			: public std::enable_shared_from_this<tcp_connection>
		{
			iocp::IODispatcher &io_;

			Tcp::Socket socket_;
			std::vector<char> buf_;

			async::iocp::CallbackType readCallback_;
			async::iocp::CallbackType writeCallback_;


		public:
			tcp_connection(const SocketPtr &);
			~tcp_connection();

		private:
			tcp_connection(const tcp_connection &);
			tcp_connection &operator=(const tcp_connection &);

		public:
			
		};
	}
}

#endif