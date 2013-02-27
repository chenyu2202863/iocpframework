#ifndef __IOCP_INTERNAL_CONDITION_HPP
#define __IOCP_INTERNAL_CONDITION_HPP

#include <limits>

namespace async
{
	namespace iocp
	{

		namespace detail
		{
			// 默认单次传输字节大小
			//enum { DEFAULT_MAX_TRANSFER =  };

			// 分包最大缓冲区大小
			static const std::uint32_t MAX_BUFFER_LEN = 8192;

			// ----------------------------------------------------
			// struct TransferAll

			// 传输所有字节
			struct transfer_all_t
			{
				typedef size_t	result_type;

				result_type operator()(size_t min = 0) const
				{
					return std::numeric_limits<result_type>::max();
				}
			};


			// ----------------------------------------------------
			// class TransferAtLeat

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

		inline detail::transfer_all_t transfer_all()
		{
			return detail::transfer_all_t();
		}

		inline detail::transfer_at_leat_t transfer_at_least(size_t min)	
		{
			return detail::transfer_at_leat_t(min);
		}
	}
}








#endif