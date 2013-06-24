#ifndef __ASYNC_SERVICE_CONDITION_HPP
#define __ASYNC_SERVICE_CONDITION_HPP

#include <cstdint>
#include <limits>

#ifdef max
#undef max
#endif

namespace async { namespace service {

	namespace details
	{
		static const std::uint32_t MAX_BUFFER_LEN = 64 * 1024;


		// 传输所有字节
		struct transfer_all_t
		{
			typedef size_t	result_type;

			result_type operator()(size_t min = 0) const
			{
				return std::numeric_limits<result_type>::max();
			}
		};


		// 最少传输字节
		class transfer_at_leat_t
		{
		public:
			typedef size_t	result_type;

		private:
			size_t min_;

		public:
			explicit transfer_at_leat_t(size_t min)
				: min_(min)
			{}

		public:
			result_type operator()(size_t min = 0) const
			{
				return min_;
			}
		};
	}


	// 传输条件

	inline details::transfer_all_t transfer_all()
	{
		return details::transfer_all_t();
	}

	inline details::transfer_at_leat_t transfer_at_least(size_t min)	
	{
		return details::transfer_at_leat_t(min);
	}
}
}








#endif