#ifndef __ASYNC_URDL_ISTREAMBUF_HPP
#define __ASYNC_URDL_ISTREAMBUF_HPP

#include <streambuf>
#include <system_error>
#include "urdl/option_set.hpp"
#include "urdl/url.hpp"



namespace urdl 
{


	class istreambuf
		: public std::streambuf
	{
	private:
		struct body;
		body* body_;

	public:
		istreambuf();
		~istreambuf();


	public:
		template <typename Option>
		void set_option(const Option& option)
		{
			option_set options;
			options.set_option(option);
			set_options(options);
		}

		void set_options(const option_set& options);

		template <typename Option>
		Option get_option() const
		{
			option_set options(get_options());
			return options.get_option<Option>();
		}

		option_set get_options() const;

		bool is_open() const;
		istreambuf* open(const url& u);
		istreambuf* close();

		const std::error_code& puberror() const;

		std::size_t open_timeout() const;
		void open_timeout(std::size_t milliseconds);
		std::size_t read_timeout() const;
		void read_timeout(std::size_t milliseconds);

		std::string content_type() const;
		std::size_t content_length() const;
		std::string headers() const;

	protected:
		int_type underflow();
		virtual const std::error_code& error() const;

	private:
		void init_buffers();


	};

} 



#endif
