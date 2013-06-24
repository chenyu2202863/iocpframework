#ifndef __ASYNC_NETWORK_CONNECT_HPP
#define __ASYNC_NETWORK_CONNECT_HPP

#include <system_error>
#include <cstdint>


namespace async { namespace network { namespace details {

	// Hook User Connect Callback
	template < typename HandlerT >
	struct connect_handle_t
	{	
		socket_handle_t &remote_;
		HandlerT handler_;

		connect_handle_t(socket_handle_t &remote, HandlerT &&handler)
			: remote_(remote)
			, handler_(std::move(handler))
		{}

	public:
		void operator()(std::error_code error, std::uint32_t size)
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