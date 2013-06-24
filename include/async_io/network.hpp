#ifndef __ASYNC_NETWORK_HPP
#define __ASYNC_NETWORK_HPP

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <chrono>

#include "service/dispatcher.hpp"
#include "service/read_write_buffer.hpp"
#include "service/multi_buffer.hpp"
#include "network/tcp.hpp"
#include "timer/timer.hpp"


namespace async { namespace network { 

	class session;
	typedef std::shared_ptr<session> session_ptr;
	typedef std::weak_ptr<session> session_weak_ptr;

	typedef std::function<void(const session_ptr &, const std::string &msg)>	error_handler_type;
	typedef std::function<bool(const session_ptr &, const std::string &ip)>		accept_handler_type;
	typedef std::function<void(const session_ptr &)>							disconnect_handler_type;

	std::string error_msg(const std::error_code &err);

	// ----------------------------------

	class session
		: public std::enable_shared_from_this<session>
	{
		service::io_dispatcher_t &io_;
		tcp::socket sck_;

		void *data_;

	public:
		const error_handler_type &error_handler_;
		const disconnect_handler_type &disconnect_handler_;

	public:
		session(service::io_dispatcher_t &io, socket_handle_t &&sck, 
			const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler);
		~session();

	private:
		session(const session &);
		session &operator=(const session &);

	public:
		tcp::socket &get() { return sck_; }
		std::string get_ip() const;

		template < typename BufferT, typename HandlerT >
		bool async_read(BufferT &&buffer, HandlerT &&read_handler);

		template < typename BufferT, typename HandlerT >
		bool async_write(BufferT &&buffer, HandlerT &&write_handler);

		template < typename ParamT >
		bool async_write(ParamT &&param);

		void additional_data(void *data);
		void *additional_data() const;

		void disconnect();

	private:
		template < typename HandlerT >
		void _handle_read(const std::error_code &error, std::uint32_t size, const HandlerT &read_handler);

		template < typename HandlerT >
		void _handle_write(const std::error_code &error, std::uint32_t size, const HandlerT &write_handler);
	};

	template < typename HandlerT >
	bool async_read(const session_ptr &val, char *buf, std::uint32_t len, HandlerT &&handler)
	{
		if( !val )
			return false;

		return val->async_read(std::move(async::service::buffer(buf, len)), std::forward<HandlerT>(handler));
	}

	template < typename HandlerT >
	bool async_write(const session_ptr &val, const char *buf, std::uint32_t len, HandlerT &&handler)
	{
		if( !val )
			return false;

		return val->async_write(std::move(async::service::buffer(buf, len)), std::forward<HandlerT>(handler));
	}

	template < typename HandlerT >
	bool async_write(const session_ptr &remote, async::service::const_array_buffer_t &&buf, HandlerT &&handler)
	{
		if( !remote )
			return false;

		return remote->async_write(buf, std::forward<HandlerT>(handler));
	}

	// fix MS BUG
	template < typename HandlerT >
	struct handler_wrapper_t
		: HandlerT
	{
		typedef handler_wrapper_t<HandlerT> this_type;

		session_ptr session_; 
		const error_handler_type &error_handler_;

		handler_wrapper_t(session_ptr &&val, const error_handler_type &error_handler, HandlerT &&handler)
			: HandlerT(std::move(handler))
			, session_(std::move(val))
			, error_handler_(error_handler)
		{}
		handler_wrapper_t(handler_wrapper_t &&rhs)
			: HandlerT(std::move(rhs))
			, session_(rhs.session_)
			, error_handler_(rhs.error_handler_)
		{}

		void operator()(const std::error_code &error, std::uint32_t size)
		{
			if( error.value() == 0 )	// success
			{
				if( size == 0 )
				{
					// disconnect
					session_->disconnect();
				}
				else
					static_cast<HandlerT *>(this)->operator()(std::cref(session_), size);
			}
			else
			{
				if( error_handler_ )
				{
					error_handler_(std::cref(session_), std::cref(error_msg(error)));
				}
				session_->disconnect();
			}
		}

		friend void *allocate_handler(std::uint32_t sz, this_type *this_handler)
		{
			return service::allocate_handler(sz, *static_cast<HandlerT *>(this_handler));
		}

		friend void deallocate_handler(void *p, std::uint32_t sz, this_type *this_handler)
		{
			service::deallocate_handler(p, sz, *static_cast<HandlerT *>(this_handler));
		}
	};

	template < typename BufferT, typename HandlerT >
	bool session::async_read(BufferT &&buffer, HandlerT &&read_handler)
	{
		try
		{
			service::async_read(
				sck_, 
				buffer, 
				service::transfer_all(), 
				std::move(handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(shared_from_this(), error_handler_, std::forward<HandlerT>(read_handler))));

			// MS BUG!!
			//std::bind(&session::_handle_read<HandlerT>, shared_from_this(), async::service::_Error, async::service::_Size, std::move(read_handler)));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();

			return false;
		}
		catch(std::exception &e)
		{
			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}

	template < typename BufferT, typename HandlerT >
	bool session::async_write(BufferT &&buffer, HandlerT &&write_handler)
	{
		try
		{
			service::async_write(
				sck_, 
				buffer, 
				service::transfer_all(), 
				std::move(handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(shared_from_this(), error_handler_, std::forward<HandlerT>(write_handler))));
			//std::bind(&session::_handle_write<HandlerT>, shared_from_this(), async::service::_Error, async::service::_Size, std::move(write_handler)));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));
			disconnect();

			return false;
		}
		catch(std::exception &e)
		{
			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}

	template < typename ParamT >
	bool session::async_write(ParamT &&param)
	{
		try
		{
			sck_.async_write(std::move(handler_wrapper_t<typename std::remove_all_extents<ParamT>::type>(shared_from_this(), error_handler_, std::forward<ParamT>(param))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));
			disconnect();

			return false;
		}
		catch(std::exception &e)
		{
			if( error_handler_ )
				error_handler_(shared_from_this(), std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}
	// ----------------------------------

	class server
	{

	private:	
		struct impl ;
		std::unique_ptr<impl> impl_;

	public:
		explicit server(std::uint16_t port, std::uint32_t thr_cnt = 0);
		~server();

	private:
		server(const server &);
		server &operator=(const server &);

	public:
		bool start();
		bool stop();

		void register_error_handler(const error_handler_type &);
		void register_accept_handler(const accept_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);
	};


	// ----------------------------------

	class client
	{
	public:
		enum { DEFAULT_TIME_OUT = 3 };

	public:
		typedef std::function<void(const std::string &msg)>		error_handler_type;
		typedef std::function<void(bool)>						connect_handler_type;
		typedef std::function<void()>							disconnect_handler_type;

	private:	
		service::io_dispatcher_t &io_;
		tcp::socket socket_;

		timer::win_timer_service_t &timer_svr_;
		//timer::timer_handle_ptr timer_;

		client::error_handler_type error_handle_;
		client::connect_handler_type connect_handle_;
		client::disconnect_handler_type disconnect_handle_;


	public:
		explicit client(service::io_dispatcher_t &io, timer::win_timer_service_t &timer_svr);
		~client();

	private:
		client(const client &);
		client &operator=(const client &);

	public:
		bool start(const std::string &ip, std::uint16_t port, std::chrono::seconds time_out);
		void stop();

		void register_error_handler(const error_handler_type &);
		void register_connect_handler(const connect_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);

	public:
		template < typename BufferT, typename HandlerT >
		bool async_send(BufferT &&buf, HandlerT &&handler);
		template < typename ParamT >
		bool async_send(ParamT &&handler);
		template < typename HandlerT >
		bool async_send(const char *buf, std::uint32_t len, HandlerT &&handler);

		template < typename HandlerT >
		bool async_recv(char *buf, std::uint32_t len, HandlerT &&handler);

		std::uint32_t send(const char *buf, std::uint32_t len);
		std::uint32_t recv(char *buf, std::uint32_t len);

		void disconnect();

	private:
		void _on_connect(const std::error_code &errorerror);
		
	};


	// fix MS BUG
	template < typename HandlerT >
	struct cli_handler_wrapper_t
		: HandlerT
	{
		typedef cli_handler_wrapper_t<HandlerT> this_type;

		client &session_; 
		const client::error_handler_type &error_handler_;

		cli_handler_wrapper_t(client &val, const client::error_handler_type &error_handler, HandlerT &&handler)
			: HandlerT(std::move(handler))
			, session_(val)
			, error_handler_(error_handler)
		{}
		cli_handler_wrapper_t(cli_handler_wrapper_t &&rhs)
			: HandlerT(std::move(rhs))
			, session_(rhs.session_)
			, error_handler_(rhs.error_handler_)
			
		{}

		void operator()(const std::error_code &error, std::uint32_t size)
		{
			if( error.value() == 0 )	// success
			{
				if( size == 0 )
				{
					// disconnect
					session_.disconnect();
				}
				else
					static_cast<HandlerT *>(this)->operator()(size);
			}
			else
			{
				if( error_handler_ )
				{
					error_handler_(std::cref(error_msg(error)));
				}
				session_.disconnect();
			}
		}

		friend void *allocate_handler(std::uint32_t sz, this_type *this_handler)
		{
			return service::allocate_handler(sz, *static_cast<HandlerT *>(this_handler));
		}

		friend void deallocate_handler(void *p, std::uint32_t sz, this_type *this_handler)
		{
			service::deallocate_handler(p, sz, *static_cast<HandlerT *>(this_handler));
		}
	};

	template < typename BufferT, typename HandlerT >
	bool client::async_send(BufferT &&buf, HandlerT &&handler)
	{
		try
		{
			service::async_write(socket_, std::forward<BufferT>(buf), service::transfer_all(),
				std::move(cli_handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(*this, error_handle_, std::forward<HandlerT>(handler))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}

		return true;
	}
	template < typename ParamT >
	bool client::async_send(ParamT &&handler)
	{
		typedef ParamT param_t;

		try
		{
			socket_.async_write(std::move(cli_handler_wrapper_t<typename std::remove_all_extents<param_t>::type>(*this, error_handle_, std::forward<param_t>(handler))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}

		return true;
	}

	template < typename HandlerT >
	bool client::async_send(const char *buf, std::uint32_t len, HandlerT &&handler)
	{
		try
		{
			service::async_write(socket_, service::buffer(buf, len), service::transfer_all(),
				std::move(cli_handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(*this, error_handle_, std::forward<HandlerT>(handler))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));
			disconnect();
			return false;
		}

		return true;
	}

	template < typename HandlerT >
	bool client::async_recv(char *buf, std::uint32_t len, HandlerT &&handler)
	{
		try
		{
			service::async_read(socket_, async::service::buffer(buf, len), service::transfer_all(), 
				std::move(cli_handler_wrapper_t<typename std::remove_all_extents<HandlerT>::type>(*this, error_handle_, std::forward<HandlerT>(handler))));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::cref(std::string(e.what())));

			disconnect();
			return false;
		}

		return true;
	}
}
}

#endif