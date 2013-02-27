#ifndef __RPC_CODEC_HPP
#define __RPC_CODEC_HPP

#include <memory>
#include <functional>

#include "../include/IOCP/Buffer.hpp"


namespace async
{
	namespace rpc
	{
		
		class rpc_msg;
	
		class tcp_connection;
		typedef std::shared_ptr<tcp_connection> tcp_connection_ptr;



		class rpc_codec
		{
		public:
			enum error_code
			{
				kNoError = 0,
				kInvalidLength,
				kCheckSumError,
				kInvalidNameLen,
				kUnknownMessageType,
				kParseError
			};

			typedef std::function<void (const tcp_connection_ptr &,
				const rpc_msg &,
				time_t)> protobuf_msg_callback_type;

			typedef std::function<void (const tcp_connection_ptr &,
				AutoBuffer &,
				time_t,
				error_code)> error_callback_type;


		private:
			protobuf_msg_callback_type msg_handler_;
			error_callback_type error_handler_;

			const static int kHeaderLen		= sizeof(std::int32_t);
			const static int kMinMessageLen = 2 * kHeaderLen;	// RPC0 + checkSum
			const static int kMaxMessageLen = 64 * 1024 * 1024; // same as 

		public:
			explicit rpc_codec(const protobuf_msg_callback_type &msg_callback)
				: msg_handler_(msg_callback)
			{}

			rpc_codec(const protobuf_msg_callback_type &msg_callback, const error_callback_type &err_callback)
				: msg_handler_(msg_callback)
				, error_handler_(err_callback)
			{}



		};
	}
}


#endif