#ifndef __RPC_SERVER_HPP
#define __RPC_SERVER_HPP

#include "tcp_svr.hpp"


namespace google
{
	namespace protobuf
	{
		class Service;
	}
}


namespace async
{
	namespace iocp
	{
		class IODispatcher;
	}

	namespace rpc
	{

		class rpc_svr
		{
			iocp::IODispatcher &io_;
			
			tcp_svr tcp_svr_;
			std::map<std::string, google::protobuf::Service *> services_;

		public:
			svr(iocp::IODispatcher &);

		private:
			svr(const svr &);
			svr &operator=(const svr &);

		public:
			void register_service(google::protobuf::Service *);
			void start();

		private:

		};
	}
}



#endif