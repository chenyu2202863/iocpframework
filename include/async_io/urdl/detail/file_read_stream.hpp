#ifndef __ASYNC_URDL_DETAIL_FILE_READ_STREAM_HPP
#define __ASYNC_URDL_DETAIL_FILE_READ_STREAM_HPP



#include <cctype>
#include <fstream>

#include "urdl/option_set.hpp"
#include "urdl/url.hpp"
#include "IOCP/Dispatcher.hpp"


namespace urdl 
{
	namespace detail 
	{

		class file_read_stream
		{
		private:
			async::iocp::IODispatcher &io_;
			option_set &options_;
			std::ifstream file_;

		public:
			explicit file_read_stream(async::iocp::IODispatcher &io, option_set &options)
				: io_(io)
				, options_(options)
			{
			}

			std::error_code open(const url& u, std::error_code& ec)
			{
				file_.clear();
				std::string path = u.path();

				if( path.length() >= 3 && 
					path[0] == '/' && 
					std::isalpha(path[1]) && 
					path[2] == ':' )
					path = path.substr(1);

				file_.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
				if( !file_ )
				{
					ec = make_error_code(std::errc::no_such_file_or_directory);
					return ec;
				}

				ec = std::error_code();
				return ec;
			}

			template< typename HandlerT >
			void async_open(const url& u, const HandlerT &handler)
			{
				std::error_code ec;
				open(u, ec);

				io_.Post(std::bind(handler, ec));
			}

			std::error_code close(std::error_code& ec)
			{
				file_.close();
				file_.clear();

				ec = std::error_code();
				return ec;
			}

			bool is_open() const
			{
				return file_.is_open();
			}

			std::size_t read_some(const async::iocp::MutableBuffer &buffer, std::error_code& ec)
			{
				if( !file_ )
				{
					ec = std::error::eof;
					return 0;
				}


				file_.read(buffer.data(), buffer.size());
				size_t length = file_.gcount();
				if( length == 0 && !file_ )
					ec = std::error::eof;
				return length;
			}

			template < typename HandlerT >
			void async_read_some(const async::iocp::MutableBuffer& buffers, const HandlerT &handler)
			{
				std::error_code ec;
				std::size_t bytes_transferred = read_some(buffers, ec);
				io_.Post(std::bind(handler, ec, bytes_transferred));
			}


		};

	}
}



#endif 
