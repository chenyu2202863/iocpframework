#ifndef __NETWORK_ACCEPT_HPP
#define __NETWORK_ACCEPT_HPP



namespace async
{
	namespace iocp
	{
		static std::tr1::_Ph<2> _Socket;
		static std::tr1::_Ph<3> _Address;
	}
	
	namespace network
	{
		namespace detail
		{
		
			static const size_t SOCKET_ADDR_SIZE = sizeof(sockaddr_in) + 16;

			// Hook User Accept Callback
			template < typename HandlerT >
			struct accept_handle_t
			{	
				socket_handle &acceptor_;
				socket_handle_ptr remote_sck_;
				iocp::auto_buffer_ptr buf_;
				HandlerT handler_;

				accept_handle_t(socket_handle &acceptor, const socket_handle_ptr &remoteSocket, const iocp::auto_buffer_ptr &buf, const HandlerT &handler)
					: acceptor_(acceptor)
					, remote_sck_(remoteSocket)
					, buf_(buf)
					, handler_(handler)
				{}

			public:
				void operator()(iocp::error_code error, u_long size)
				{
					// ¸´ÖÆListen socketÊôÐÔ
					update_accept_context context(acceptor_);
					remote_sck_->set_option(context);

					sockaddr *local = 0, *remote = 0;
					int local_size = 0, remote_size = 0;

					socket_provider::singleton().GetAcceptExSockaddrs(buf_->data(), 0, 
						SOCKET_ADDR_SIZE, SOCKET_ADDR_SIZE, 
						&local, &local_size, 
						&remote, &remote_size);

					handler_(error, std::cref(remote_sck_), reinterpret_cast<SOCKADDR_IN *>(remote));
				}
			};
		}

	}

}



#endif