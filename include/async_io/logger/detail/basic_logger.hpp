#ifndef __LOGGER_BASIC_LOGGER_HPP
#define __LOGGER_BASIC_LOGGER_HPP



namespace async
{
	namespace logger
	{
		template< typename ServiceT >
		class basic_logger_t
		{
			typedef ServiceT								service_type;
			typedef typename service_type::dispatcher_type	dispatcher_type;
			typedef typename service_type::impl_type		impl_type;

		private:
			service_type &service_;
			impl_type impl_;

		public:
			explicit basic_logger_t(dispatcher_type &io, const std::string &id)
				: service_(service_type::instance(io))
				, impl_(nullptr)
			{
				service_.create(impl_, id);
			}
			~basic_logger_t()
			{
				service_.destroy(impl_);
			}

		public:
			// 设置日志文件
			void use_file(const std::string &file)
			{
				service_.use_file(impl_, file);
			}
			// 记录消息
			void log(const std::string &msg)
			{
				service_.log(impl_, msg);
			}
		};
	}
}

#endif