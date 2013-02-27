#ifndef __LOGGER_LOGGER_SERVICE_HPP
#define __LOGGER_LOGGER_SERVICE_HPP

#include <sstream>
#include <fstream>
#include <string>

#include "../iocp/dispatcher.hpp"


namespace async
{
	namespace logger
	{
		// -------------------------------------------
		// class LoggerService

		template < typename ServiceT >
		class service_t
		{
		public:
			typedef ServiceT	dispatcher_type;

		private:
			dispatcher_type &io_;
			std::ofstream out_;

		public:
			// Logger 实现
			struct impl_t
			{
				std::string id_;
				explicit impl_t(const std::string &id)
					: id_(id)
				{}
			};
			typedef impl_t * impl_type;


		public:
			service_t(dispatcher_type &io)
				: io_(io)
			{}
			~service_t()
			{}

		private:
			service_t(const service_t &);
			service_t &operator=(const service_t &);

		public:
			static service_t &instance(dispatcher_type &io)
			{
				static service_t service(io);
				return service;
			}

		public:
			void create(impl_type &impl, const std::string &id)
			{
				impl = new impl_t(id);
			}
			void destroy(impl_type &impl)
			{
				delete impl;
				impl = nullptr;
			}

			// 设置日志文件
			void use_file(impl_type &/*impl*/, const std::string &file)
			{
				io_.dispatch(std::bind(&service_t::_use_file_impl, this, file));
			}

			// 记录信息
			void log(impl_type &impl, const std::string &msg)
			{
				// 格式化
				std::ostringstream os;
				os << impl->id_ << ": " << msg;

				io_.dispatch(std::bind(&service_t::_log_impl, this, os.str()));
			}

		private:
			void _use_file_impl(const std::string &file)
			{
				out_.close();
				out_.clear();
				out_.open(file.c_str());
			}

			void _log_impl(const std::string &text)
			{
				if( out_ )
					out_ << text << std::endl;
			}
		};
	}
}




#endif