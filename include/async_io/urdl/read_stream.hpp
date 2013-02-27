#ifndef URDL_READ_STREAM_HPP
#define URDL_READ_STREAM_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "urdl/http.hpp"
#include "urdl/option_set.hpp"
#include "urdl/url.hpp"
#include "urdl/detail/coroutine.hpp"
#include "urdl/detail/file_read_stream.hpp"
#include "urdl/detail/http_read_stream.hpp"


namespace urdl 
{


	class read_stream
	{
	public:

		explicit read_stream(boost::asio::io_service& io_service)
			: io_service_(io_service),
			file_(io_service, options_),
			http_(io_service, options_),
			protocol_(unknown)
		{
		}


		boost::asio::io_service& get_io_service()
		{
			return io_service_;
		}

		template <typename Option>
		void set_option(const Option& option)
		{
			options_.set_option(option);
		}

		void set_options(const option_set& options)
		{
			options_.set_options(options);
		}

		template <typename Option>
		Option get_option() const
		{
			return options_.get_option<Option>();
		}

		option_set get_options() const
		{
			return options_;
		}

		bool is_open() const
		{
			switch (protocol_)
			{
			case file:
				return file_.is_open();
			case http:
				return http_.is_open();
			default:
				return false;
			}
		}


		void open(const url& u)
		{
			std::error_code ec;
			if (open(u, ec))
			{
				throw std::system_error(ec);
			}
		}


		std::error_code open(const url& u, std::error_code& ec)
		{
			url tmp_url = u;
			std::size_t redirects = 0;
			for (;;)
			{
				if (tmp_url.protocol() == "file")
				{
					protocol_ = file;
					return file_.open(tmp_url, ec);
				}
				else if (tmp_url.protocol() == "http")
				{
					protocol_ = http;
					http_.open(tmp_url, ec);
					if (ec == http::errc::moved_permanently || ec == http::errc::found)
					{
						std::size_t max_redirects = options_.get_option<
							urdl::http::max_redirects>().value();
						if (redirects < max_redirects)
						{
							++redirects;
							tmp_url = http_.location();
							http_.close(ec);
							continue;
						}
					}
					return ec;
				}
				else
				{
					ec = std::error::operation_not_supported;
					return ec;
				}
			}
		}


		template <typename Handler>
		void async_open(const url& u, Handler handler)
		{
			open_coro<Handler>(this, u, handler)(std::error_code());
		}


		void close()
		{
			std::error_code ec;
			if (close(ec))
			{
				throw std::system_error(ec);
			}
		}


		std::error_code close(std::error_code& ec)
		{
			switch (protocol_)
			{
			case file:
				return file_.close(ec);
			case http:
				return http_.close(ec);
			default:
				ec = std::error_code();
				break;
			}

			return ec;
		}


		std::string content_type() const
		{
			switch (protocol_)
			{
			case file:
				return std::string();
			case http:
				return http_.content_type();
			default:
				return std::string();
			}
		}


		std::size_t content_length() const
		{
			switch (protocol_)
			{
			case file:
				return ~std::size_t(0);
			case http:
				return http_.content_length();
			default:
				return ~std::size_t(0);
			}
		}


		std::string headers() const
		{
			switch (protocol_)
			{
			case file:
				return std::string();
			case http:
				return http_.headers();
			default:
				return std::string();
			}
		}

		template <typename MutableBufferSequence>
		std::size_t read_some(const MutableBufferSequence& buffers)
		{
			std::error_code ec;
			std::size_t bytes_transferred = read_some(buffers, ec);
			if (ec)
			{
				throw std::system_error(ec);
			}
			return bytes_transferred;
		}


		template <typename MutableBufferSequence>
		std::size_t read_some(const MutableBufferSequence& buffers,
			std::error_code& ec)
		{
			switch (protocol_)
			{
			case file:
				return file_.read_some(buffers, ec);
			case http:
				return http_.read_some(buffers, ec);
			default:
				ec = std::error::operation_not_supported;
				return 0;
			}
		}


		template <typename MutableBufferSequence, typename Handler>
		void async_read_some(const MutableBufferSequence& buffers, Handler handler)
		{
			switch (protocol_)
			{
			case file:
				file_.async_read_some(buffers, handler);
				break;
			case http:
				http_.async_read_some(buffers, handler);
				break;
			default:
				std::error_code ec = std::error::operation_not_supported;
				io_service_.post(boost::asio::detail::bind_handler(handler, ec, 0));
				break;
			}
		}

	private:
		template <typename Handler>
		class open_coro : detail::coroutine
		{
		public:
			open_coro(read_stream* this_ptr, const url& u, Handler handler)
				: this_(this_ptr),
				url_(u),
				handler_(handler)
			{
			}

			void operator()(std::error_code ec)
			{
				URDL_CORO_BEGIN;

				for (;;)
				{
					if (url_.protocol() == "file")
					{
						this_->protocol_ = file;
						URDL_CORO_YIELD(this_->file_.async_open(url_, *this));
						handler_(ec);
						return;
					}
					else if (url_.protocol() == "http")
					{
						this_->protocol_ = http;
						URDL_CORO_YIELD(this_->http_.async_open(url_, *this));
						if (ec == http::errc::moved_permanently || ec == http::errc::found)
						{
							url_ = this_->http_.location();
							this_->http_.close(ec);
							continue;
						}
						handler_(ec);
						return;
					}
					else
					{
						ec = std::error::operation_not_supported;
						this_->io_service_.post(
							boost::asio::detail::bind_handler(handler_, ec));
						return;
					}
				}

				URDL_CORO_END;
			}

			friend void* asio_handler_allocate(std::size_t size,
				open_coro<Handler>* this_handler)
			{
				using boost::asio::asio_handler_allocate;
				return asio_handler_allocate(size, &this_handler->handler_);
			}

			friend void asio_handler_deallocate(void* pointer, std::size_t size,
				open_coro<Handler>* this_handler)
			{
				using boost::asio::asio_handler_deallocate;
				asio_handler_deallocate(pointer, size, &this_handler->handler_);
			}

			template <typename Function>
			friend void asio_handler_invoke(const Function& function,
				open_coro<Handler>* this_handler)
			{
				using boost::asio::asio_handler_invoke;
				asio_handler_invoke(function, &this_handler->handler_);
			}

		private:
			read_stream* this_;
			url url_;
			Handler handler_;
		};

		template <typename Handler> friend class open_coro;

		boost::asio::io_service& io_service_;
		option_set options_;
		detail::file_read_stream file_;
		detail::http_read_stream<boost::asio::ip::tcp::socket> http_;

		enum { unknown, file, http, https } protocol_;
	};

} 

#endif 
