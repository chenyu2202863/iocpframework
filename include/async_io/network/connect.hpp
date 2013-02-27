#ifndef __NETWORK_CONNECT_HPP
#define __NETWORK_CONNECT_HPP



namespace async
{
	namespace network
	{
		namespace detail
		{

			// Hook User Connect Callback
			template < typename HandlerT >
			struct connect_handle_t
			{	
				socket_handle &remote_;
				HandlerT handler_;

				connect_handle_t(socket_handle &remote, const HandlerT &handler)
					: remote_(remote)
					, handler_(std::move(handler))
				{}
				
			public:
				void operator()(iocp::error_code error, u_long size)
				{
					// ∏¥÷∆socket Ù–‘
					remote_.set_option(update_connect_context());

					handler_(error);
				}
			};
		}

	}

}



#endif