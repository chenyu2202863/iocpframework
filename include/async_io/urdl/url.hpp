#ifndef __ASYNC_URDL_URL_HPP
#define __ASYNC_URDL_URL_HPP

#include <string>
#include <system_error>

namespace urdl 
{

	class url
	{
	private:
		std::string protocol_;
		std::string user_info_;
		std::string host_;
		std::string port_;
		std::string path_;
		std::string query_;
		std::string fragment_;

	public:
		url()
		{
		}

		url(const char* s)
		{
			*this = from_string(s);
		}


		url(const std::string& s)
		{
			*this = from_string(s);
		}

	public:
		const std::string &protocol() const
		{
			return protocol_;
		}


		const std::string &user_info() const
		{
			return user_info_;
		}


		const std::string &host() const
		{
			return host_;
		}

		unsigned short port() const;

		std::string path() const;


		const std::string &query() const
		{
			return query_;
		}


		const std::string &fragment() const
		{
			return fragment_;
		}

		/// Components of the URL, used with @c from_string.
		enum components_type
		{
			protocol_component = 1,
			user_info_component = 2,
			host_component = 4,
			port_component = 8,
			path_component = 16,
			query_component = 32,
			fragment_component = 64,
			all_components = protocol_component | user_info_component | host_component
			| port_component | path_component | query_component | fragment_component
		};


		std::string to_string(int components = all_components) const;

	public:
		static url from_string(const char* s);
		static url from_string(const char* s, std::error_code& ec);
		static url from_string(const std::string& s);
		static url from_string(const std::string& s, std::error_code& ec);


		friend bool operator==(const url& a, const url& b);
		friend bool operator!=(const url& a, const url& b);
		friend bool operator<(const url& a, const url& b);

	private:
		static bool _unescape_path(const std::string& in, std::string& out);


	};

} // namespace urdl

#endif 
