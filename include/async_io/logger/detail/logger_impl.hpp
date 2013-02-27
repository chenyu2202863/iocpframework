#ifndef __LOGGER_LOGGER_IMPL_HPP
#define __LOGGER_LOGGER_IMPL_HPP


#include "logger_service.hpp"
#include "basic_logger.hpp"


namespace async
{
	namespace iocp
	{
		class io_dispatcher;
	}

	namespace logger
	{
		typedef basic_logger_t< service_t<iocp::io_dispatcher> >	basic_log;
		typedef std::shared_ptr<basic_log>							basic_log_ptr;
	}
}






#endif