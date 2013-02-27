#ifndef __ASYNC_URDL_HTTP_HPP
#define __ASYNC_URDL_HTTP_HPP

#include <string>
#include <system_error>



namespace urdl 
{
	namespace http 
	{

		const std::error_category& error_category();


		class request_method
		{
		private:
			std::string value_;

		public:
			request_method()
				: value_("GET")
			{
			}

			explicit request_method(const std::string& v)
				: value_(v)
			{
			}

		public:
			const std::string &value() const
			{
				return value_;
			}

			void value(const std::string &v)
			{
				value_ = v;
			}
		};



		class request_content
		{
		private:
			std::string value_;

		public:
			request_content()
				: value_("")
			{
			}

			explicit request_content(const std::string& v)
				: value_(v)
			{
			}

		public:
			const std::string &value() const
			{
				return value_;
			}

			void value(const std::string& v)
			{
				value_ = v;
			}
		};


		class request_content_type
		{
		private:
			std::string value_;

		public:
			request_content_type()
				: value_("")
			{
			}

			explicit request_content_type(const std::string& v)
				: value_(v)
			{
			}

		public:
			const std::string &value() const
			{
				return value_;
			}

			void value(const std::string& v)
			{
				value_ = v;
			}
		};


		class max_redirects
		{
		private:
			std::size_t value_;

		public:
			max_redirects()
				: value_(~std::size_t(0))
			{
			}

			explicit max_redirects(std::size_t v)
				: value_(v)
			{
			}

		public:
			std::size_t value() const
			{
				return value_;
			}


			void value(std::size_t v)
			{
				value_ = v;
			}
		};


		class user_agent
		{
		private:
			std::string value_;

		public:
			user_agent()
				: value_("")
			{
			}

			explicit user_agent(const std::string& v)
				: value_(v)
			{
			}

		public:
			const std::string &value() const
			{
				return value_;
			}

			void value(const std::string& v)
			{
				value_ = v;
			}
		};



		namespace errc 
		{

			enum errc_t
			{
				// Client-generated errors.

				/// The response's status line was malformed.
				malformed_status_line = 1,

				/// The response's headers were malformed.
				malformed_response_headers = 2,

				// Server-generated status codes.

				/// The server-generated status code "100 Continue".
				continue_request = 100,

				/// The server-generated status code "101 Switching Protocols".
				switching_protocols = 101,

				/// The server-generated status code "200 OK".
				ok = 200,

				/// The server-generated status code "201 Created".
				created = 201,

				/// The server-generated status code "202 Accepted".
				accepted = 202,

				/// The server-generated status code "203 Non-Authoritative Information".
				non_authoritative_information = 203,

				/// The server-generated status code "204 No Content".
				no_content = 204,

				/// The server-generated status code "205 Reset Content".
				reset_content = 205,

				/// The server-generated status code "206 Partial Content".
				partial_content = 206,

				/// The server-generated status code "300 Multiple Choices".
				multiple_choices = 300,

				/// The server-generated status code "301 Moved Permanently".
				moved_permanently = 301,

				/// The server-generated status code "302 Found".
				found = 302,

				/// The server-generated status code "303 See Other".
				see_other = 303,

				/// The server-generated status code "304 Not Modified".
				not_modified = 304,

				/// The server-generated status code "305 Use Proxy".
				use_proxy = 305,

				/// The server-generated status code "307 Temporary Redirect".
				temporary_redirect = 307,

				/// The server-generated status code "400 Bad Request".
				bad_request = 400,

				/// The server-generated status code "401 Unauthorized".
				unauthorized = 401,

				/// The server-generated status code "402 Payment Required".
				payment_required = 402,

				/// The server-generated status code "403 Forbidden".
				forbidden = 403,

				/// The server-generated status code "404 Not Found".
				not_found = 404,

				/// The server-generated status code "405 Method Not Allowed".
				method_not_allowed = 405,

				/// The server-generated status code "406 Not Acceptable".
				not_acceptable = 406,

				/// The server-generated status code "407 Proxy Authentication Required".
				proxy_authentication_required = 407,

				/// The server-generated status code "408 Request Time-out".
				request_timeout = 408,

				/// The server-generated status code "409 Conflict".
				conflict = 409,

				/// The server-generated status code "410 Gone".
				gone = 410,

				/// The server-generated status code "411 Length Required".
				length_required = 411,

				/// The server-generated status code "412 Precondition Failed".
				precondition_failed = 412,

				/// The server-generated status code "413 Request Entity Too Large".
				request_entity_too_large = 413,

				/// The server-generated status code "414 Request URI Too Large".
				request_uri_too_large = 414,

				/// The server-generated status code "415 Unsupported Media Type".
				unsupported_media_type = 415,

				/// The server-generated status code "416 Requested Range Not Satisfiable".
				requested_range_not_satisfiable = 416,

				/// The server-generated status code "417 Expectation Failed".
				expectation_failed = 417,

				/// The server-generated status code "500 Internal Server Error".
				internal_server_error = 500,

				/// The server-generated status code "501 Not Implemented".
				not_implemented = 501,

				/// The server-generated status code "502 Bad Gateway".
				bad_gateway = 502,

				/// The server-generated status code "503 Service Unavailable".
				service_unavailable = 503,

				/// The server-generated status code "504 Gateway Timeout".
				gateway_timeout = 504,

				/// The server-generated status code "505 HTTP Version Not Supported".
				version_not_supported = 505
			};


			inline std::error_code make_error_code(errc_t e)
			{
				return std::error_code(static_cast<int>(e), http::error_category());
			}

		} // namespace errc
	} // namespace http


} // namespace urdl

namespace std
{

	template <>
	struct is_error_code_enum<urdl::http::errc::errc_t>
	{
		static const bool value = true;
	};
} 




#endif // URDL_HTTP_HPP
