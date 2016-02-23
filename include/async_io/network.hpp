#ifndef __ASYNC_NETWORK_HPP
#define __ASYNC_NETWORK_HPP

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <chrono>
#include <list>
#include <atomic>

#include "basic.hpp"
#include "service/read_write_buffer.hpp"
#include "service/multi_buffer.hpp"
#include "network/tcp.hpp"
#include "timer/timer.hpp"

#include "../utility/move_wrapper.hpp"
#include "../memory_pool/sgi_memory_pool.hpp"
#include "../memory_pool/fixed_memory_pool.hpp"
#include "../extend_stl/allocator/container_allocator.hpp"
#include "../utility/object_pool.hpp"
#include "../extend_stl/container/sync_container.hpp"

namespace async { namespace network {

	typedef std::shared_ptr<socket_handle_t> socket_ptr;
	typedef memory_pool::mt_memory_pool socket_memory_pool_t;
	typedef stdex::allocator::pool_allocator_t<socket_ptr, socket_memory_pool_t> socket_allocator_t;
	typedef std::list<socket_ptr, socket_allocator_t> socket_list_t;
	typedef stdex::container::sync_sequence_container_t<socket_ptr, socket_list_t> socket_pool_list_t;

}
}

namespace utility {

	template < >
	struct object_pool_traits_t<async::network::socket_pool_list_t>
	{
		typedef async::network::socket_pool_list_t pool_t;
		typedef async::network::socket_ptr value_t;

		static value_t pop(pool_t &l)
		{
			if( l.empty() )
			{
				return value_t();
			}
			else
			{
				value_t val = l.top();
				l.pop_top();
				return val;
			}
		}

		static void push(pool_t &l, value_t && val)
		{
			l.push_back(std::move(val));
		}
	};
}


namespace async { namespace network { 

	class server;
	class session;
	typedef std::shared_ptr<session> session_ptr;
	typedef std::weak_ptr<session> session_weak_ptr;

	typedef std::function<void(const session_ptr &, const std::string &msg)>	error_handler_type;
	typedef std::function<bool(const session_ptr &, const std::string &ip)>		accept_handler_type;
	typedef std::function<void(const session_ptr &)>							disconnect_handler_type;

	typedef utility::object_pool_t<socket_handle_t, socket_pool_list_t> socket_pool_t;


	std::string error_msg(const std::error_code &err);

	// ----------------------------------

	class session
		: public std::enable_shared_from_this<session>
	{
		struct holder_t
		{
			virtual ~holder_t() {}
			virtual void *get() = 0;
		};

		template < typename T >
		struct holder_impl_t
			: holder_t
		{
			T val_;

			holder_impl_t(const T &val)
				: val_(val)
			{}

			virtual void *get()
			{
				return &val_;
			}
		};

		server &svr_;
		mutable std::shared_ptr<socket_handle_t> sck_;
		std::shared_ptr<holder_t> data_;

	public:
		const error_handler_type &error_handler_;
		const disconnect_handler_type &disconnect_handler_;

	public:
		session(server &svr, std::shared_ptr<socket_handle_t> &&sck, 
			const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler);
		~session();

	private:
		session(const session &);
		session &operator=(const session &);

	public:
		socket_handle_t &get() { return *sck_; }
		std::string get_ip() const;

		template < typename HandlerT, typename AllocatorT>
		bool async_read(service::mutable_buffer_t &, HandlerT &&, AllocatorT &allocator);
		template < typename HandlerT, typename AllocatorT>
		bool async_read_some(service::mutable_buffer_t &, std::uint32_t min_len, HandlerT &&, AllocatorT &allocator);

		template < typename HandlerT, typename AllocatorT>
		bool async_write(const service::const_buffer_t &, HandlerT &&, AllocatorT &allocator);
		template < typename HandlerT, typename AllocatorT, typename ...Args >
		typename std::enable_if<!std::is_same<HandlerT, service::const_buffer_t>::value, bool>::type
			async_write(HandlerT &&, AllocatorT &, const Args &...);

		template < typename T, typename AlocatorT >
		void additional_data(const T &t, AlocatorT &allocator);

		template < typename T >
		T &additional_data();

		void shutdown();
		void disconnect();

		template < typename HandlerT >
		void _handle_read(const std::error_code &error, std::uint32_t size, const HandlerT &read_handler);

		template < typename HandlerT >
		void _handle_write(const std::error_code &error, std::uint32_t size, const HandlerT &write_handler);
	
		template < typename HandlerT >
		bool _run_impl(HandlerT &&handler, bool is_read_op);
	};

	template < typename HandlerT, typename AllocatorT >
	bool async_read(const session_ptr &val, char *buf, std::uint32_t len, HandlerT &&handler, AllocatorT &allocator)
	{
		if( !val )
			return false;

		return val->async_read(service::buffer(buf, len), std::forward<HandlerT>(handler), allocator);
	}

	template < typename HandlerT, typename AllocatorT >
	bool async_write(const session_ptr &val, const char *buf, std::uint32_t len, HandlerT &&handler, AllocatorT &allocator)
	{
		if( !val )
			return false;

		return val->async_write(service::const_buffer_t(buf, len), std::forward<HandlerT>(handler), allocator);
	}


	template < typename HandlerT, typename AllocatorT >
	bool session::async_read(service::mutable_buffer_t &buffer, HandlerT &&read_handler, AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			auto this_val = shared_from_this();
			auto handler_val = utility::make_move_obj(std::forward<HandlerT>(read_handler));

			service::async_read(*sck_,
				buffer,
				service::transfer_all(),
				[this_val, handler_val](const std::error_code &err, std::uint32_t len)
			{
				this_val->_handle_read(err, len, handler_val.value_);
			}, allocator);
		}, true);
	}

	template < typename HandlerT, typename AllocatorT >
	bool session::async_read_some(service::mutable_buffer_t &buffer, std::uint32_t min_len, HandlerT && handler, AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			auto this_val = shared_from_this();
			auto handler_val = utility::make_move_obj(std::forward<HandlerT>(handler));

			sck_->async_read(buffer, 
				[this_val, handler_val](const std::error_code &err, std::uint32_t len)
			{
				this_val->_handle_read(err, len, handler_val.value_);
			}, allocator);
		}, true);
	}

	template < typename HandlerT, typename AllocatorT >
	bool session::async_write(const service::const_buffer_t &buffer, HandlerT &&write_handler, AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			auto this_val = shared_from_this();
			auto handler_val = utility::make_move_obj(std::forward<HandlerT>(write_handler));

			service::async_write(
				*sck_, 
				buffer, 
				service::transfer_all(), 
				[this_val, handler_val](const std::error_code &err, std::uint32_t len) 
			{ 
				this_val->_handle_write(err, len, handler_val.value_);
			}, allocator);
		}, false);
	}

	template < typename HandlerT, typename AllocatorT, typename ...Args >
	typename std::enable_if<!std::is_same<HandlerT, service::const_buffer_t>::value, bool>::type
		session::async_write(HandlerT &&handler, AllocatorT &allocator, const Args &...args)
	{
		return _run_impl([&]()
		{
			sck_->async_write(handler, allocator, args...);
		}, false);
	}

	template < typename HandlerT >
	bool session::_run_impl(HandlerT && handler, bool is_read_op)
	{
		try
		{
			handler();
			return true;
		}
		catch (::exception::exception_base &e)
		{
			e.dump();

			if (error_handler_)
				error_handler_(shared_from_this(), std::string(e.what()));

			if( is_read_op )
				disconnect();

			return false;
		}
		catch (std::exception &e)
		{
			if (error_handler_)
				error_handler_(shared_from_this(), std::string(e.what()));

			if( is_read_op )
				disconnect();

			return false;
		}
	}

	template < typename HandlerT >
	void session::_handle_read(const std::error_code &error, std::uint32_t size, const HandlerT &read_handler)
	{
		try
		{
			if( !error )	// success
			{
				if( size == 0 )
				{
					// disconnect
					disconnect();
				}
				else
					read_handler(shared_from_this(), size);
			}
			else
			{
				if( size != 0 && error_handler_ )
				{
					error_handler_(shared_from_this(), error_msg(error));
				}

				disconnect();
			}
		}
		catch (...)
		{
			assert(0 && "has an exception in read_handler_");
			error_handler_(shared_from_this(), "has an exception in read_handler");
		}
	}

	template < typename HandlerT >
	void session::_handle_write(const std::error_code &error, std::uint32_t size, const HandlerT &write_handler)
	{
		try
		{
			if( !error )	// success
			{
				if( size != 0 )
					write_handler(shared_from_this(), size);
			}
			else
			{
				if( error_handler_ )
					error_handler_(shared_from_this(), error_msg(error));
			}
		}
		catch (...)
		{
			assert(0 && "has an exception in write_handler_");
			error_handler_(shared_from_this(), "has an exception in write_handler_");
		}
	}

	template < typename T, typename AlocatorT >
	void session::additional_data(const T &t, AlocatorT &allocator)
	{
		data_ = std::allocate_shared<holder_impl_t<T>>(allocator, t);
	}

	template < typename T >
	T &session::additional_data()
	{
		assert(data_ && "data is empty");
		void *val = data_->get();

		return *static_cast<T *>(val);
	}

	// ----------------------------------

	class server
	{
		friend class session;
	private:	
		struct impl;
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
		typedef std::function<void(const std::string &msg)>		error_handler_type;
		typedef std::function<void(bool)>						connect_handler_type;
		typedef std::function<void()>							disconnect_handler_type;

	private:	
		service::io_dispatcher_t &io_;
		tcp::socket socket_;

		client::error_handler_type error_handle_;
		client::connect_handler_type connect_handle_;
		client::disconnect_handler_type disconnect_handle_;

	public:
		explicit client(service::io_dispatcher_t &io);
		~client();

	private:
		client(const client &);
		client &operator=(const client &);

	public:
		bool start(const std::string &ip, std::uint16_t port);
		void stop();

		void register_error_handler(const error_handler_type &);
		void register_connect_handler(const connect_handler_type &);
		void register_disconnect_handler(const disconnect_handler_type &);

	public:
		template < typename HandlerT, typename AllocatorT >
		bool async_send(const service::const_buffer_t &, HandlerT &&, AllocatorT &allocator);
		template < typename HandlerT, typename AllocatorT, typename ...Args >
		typename std::enable_if<!std::is_same<HandlerT, service::const_buffer_t>::value, bool>::type
			async_send(HandlerT &&, AllocatorT &, Args &&...);

		template < typename HandlerT, typename AllocatorT>
		bool async_recv(char *, std::uint32_t, HandlerT &&, AllocatorT &allocator);
		template < typename HandlerT, typename AllocatorT>
		bool async_read_some(service::mutable_buffer_t &, std::uint32_t min_len, HandlerT &&, AllocatorT &allocator);


		bool send(const char *buf, std::uint32_t len);
		bool recv(char *buf, std::uint32_t len);

		void disconnect();

	private:
		template < typename HandlerT >
		bool _run_impl(HandlerT && handler);

		void _on_connect(const std::error_code &);
		
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
			if( !error )	// success
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
				if( size != 0 && error_handler_ )
				{
					error_handler_(error_msg(error));
				}
				session_.disconnect();
			}
		}
	};

	template < typename HandlerT, typename AllocatorT >
	bool client::async_send(const service::const_buffer_t &buf, HandlerT &&handler, AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			service::async_write(socket_, buf, service::transfer_all(),
				std::move(cli_handler_wrapper_t<HandlerT>(*this, error_handle_, std::forward<HandlerT>(handler))), allocator);
		});
	}


	template < typename HandlerT, typename AllocatorT, typename ...Args >
	typename std::enable_if<!std::is_same<HandlerT, service::const_buffer_t>::value, bool>::type
		client::async_send(HandlerT &&handler, AllocatorT &allocator, Args &&...args)
	{
		return _run_impl([&]()
		{
			auto handler_val = service::make_param(std::forward<HandlerT>(handler), std::forward<Args>(args)...);
			return socket_.async_write(cli_handler_wrapper_t<decltype(handler_val)>(*this, 
					error_handle_, 
					std::move(handler_val)), allocator);
		});
	}

	template < typename HandlerT, typename AllocatorT  >
	bool client::async_recv(char *buf, std::uint32_t len, HandlerT &&handler, AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			service::async_read(socket_, async::service::buffer(buf, len), service::transfer_all(), 
				cli_handler_wrapper_t<HandlerT>(*this, error_handle_, std::forward<HandlerT>(handler)), allocator);
		});
	}

	template < typename HandlerT, typename AllocatorT  >
	bool client::async_read_some(service::mutable_buffer_t &buffer, std::uint32_t min_len, HandlerT && handler, AllocatorT &allocator)
	{
		return _run_impl([&]()
		{
			socket_.async_read(buffer, 
				cli_handler_wrapper_t<HandlerT>(*this, error_handle_, std::forward<HandlerT>(handler)), 
				allocator);
		});
	}

	template < typename HandlerT >
	bool client::_run_impl(HandlerT && handler)
	{
		try
		{
			handler();
			return true;
		}
		catch(::exception::exception_base &e )
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::string(e.what()));

			disconnect();
			return false;
		}
		catch( std::exception &e )
		{
			if( error_handle_ )
				error_handle_(std::string(e.what()));

			disconnect();
			return false;
		}
	}
}
}

#endif