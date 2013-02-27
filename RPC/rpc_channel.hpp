#ifndef __RPC_CHANNEL_HPP
#define __RPC_CHANNEL_HPP

#include <map>
#include <memory>


#include "../MultiThread/atomic/atomic.hpp"
#include "../MultiThread/Lock.hpp"

#include "rpc_codec.hpp"
#include "../include/IOCP/Dispatcher.hpp"


namespace google 
{
	namespace protobuf 
	{
		class Descriptor;            // descriptor.h
		class ServiceDescriptor;     // descriptor.h
		class MethodDescriptor;      // descriptor.h
		class Message;               // message.h

		class Closure;

		class RpcController;
		class Service;

	}  // namespace protobuf
}  // namespace google


namespace async
{
	namespace rpc
	{

		class rpc_channel
			: public google::protobuf::RpcChannel
		{
			typedef async::thread::AutoCriticalSection Mutex;
			typedef async::thread::AutoLock<Mutex> AutoLock;

		private:
			rpc_codec codec_;
			tcp_connection_ptr conn_;
			std::atomic<std::int64_t> id_;
			Mutex mutex_;
			
			struct outstanding_call
			{
				google::protobuf::Message *respone;
				google::protobuf::Closure *done;
			};

			std::map<std::int64_t, outstanding_call> outstandings_;
			const std::map<std::string, google::protobuf::Service *> *services_;

		public:
			rpc_channel();
			explicit rpc_channel(const tcp_connection &con);

		public:
			void set_connection(const tcp_connection &con)
			{
				conn_ = con;
			}

			void set_services(const std::map<std::string, google::protobuf::Service *> *services)
			{
				services_ = services;
			}

			void call_method(const google::protobuf::MethodDescriptor *method,
				google::protobuf::RpcController *controller,
				const google::protobuf::Message *request,
				google::protobuf::Message *response,
				google::protobuf::Closure *done);
			void on_msg(const tcp_connection_ptr &con, AutoBuffer &buffer, time_t recv_time);

		private:
			void on_rpc_msg(const tcp_connection_ptr &con,
				const rpc_msg &msg,
				time_t recv_time);

			void on_done_callback(google::protobuf::Message *response, std::int64_t id);
		};

		typedef std::shared_ptr<rpc_channel> rpc_channel_ptr;
	}

}
#endif