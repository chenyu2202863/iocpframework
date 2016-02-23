#ifndef __ASYNC_SERVICE_WRITE_HELPER_HPP
#define __ASYNC_SERVICE_WRITE_HELPER_HPP

#include <cstdint>
#include <system_error>
#include "condition.hpp"
#include "exception.hpp"

namespace async { namespace service {

	// 
	template<typename SyncWriteStreamT, typename ConstBufferT>
	std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer)
	{
		return write(s, buffer, transfer_all(), [](std::error_code, std::uint32_t){});
	}

	template<typename SyncWriteStreamT, typename ConstBufferT, typename HandlerT >
	std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const HandlerT &callback)
	{
		return write(s, buffer, transfer_all(), callback);
	}

	template<typename SyncWriteStreamT, typename ConstBufferT>
	std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const std::uint64_t &offset)
	{
		return write(s, buffer, offset, transfer_all());
	}

	template<typename SyncWriteStreamT, typename ConstBufferT, typename CompleteConditionT, typename HandlerT >
	std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const CompleteConditionT &condition, const HandlerT &callback)
	{
		std::uint32_t transfers = 0;
		const std::uint32_t bufSize = buffer.size();

		while( transfers < condition(transfers) )
		{
			if( transfers >= bufSize )
				break;

			std::uint32_t ret = s.write(buffer + transfers);
			if( ret == 0 )
			{
				s.close();
			}

			transfers += ret;

			callback(std::make_error_code((std::errc)::GetLastError()), transfers);
		}

		return transfers;
	}

	template<typename SyncWriteStreamT, typename ConstBufferT, typename CompleteConditionT>
	std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const std::uint64_t &offset, const CompleteConditionT &condition)
	{
		std::uint32_t transfers = 0;
		const std::uint32_t bufSize = buffer.size();

		while( transfers <= condition(transfers) )
		{
			if( transfers >= bufSize )
				break;

			std::uint32_t ret = s.write(buffer + transfers, offset);
			if( ret == 0 )
			{
				s.close();
			}

			transfers += ret;
		}

		return transfers;
	}



	namespace details
	{
		template<typename AsyncWriteStreamT, typename ConstBufferT, typename CompletionConditionT, typename HandlerT, typename AllocatorT>
		class write_handler_t
		{
			typedef write_handler_t<AsyncWriteStreamT, ConstBufferT, CompletionConditionT, HandlerT, AllocatorT> this_type;

		public:
			AsyncWriteStreamT &stream_;
			ConstBufferT buffer_;
			CompletionConditionT condition_;
			std::uint32_t transfers_;
			const std::uint32_t total_;
			HandlerT handler_;
			AllocatorT &allocator_;

		public:
			write_handler_t(AsyncWriteStreamT &stream, const ConstBufferT &buffer, std::uint32_t total, const CompletionConditionT &condition, std::uint32_t transfer, HandlerT &&handler, AllocatorT &allocator)
				: stream_(stream)
				, buffer_(buffer)
				, condition_(condition)
				, transfers_(transfer)
				, total_(total)
				, handler_(std::move(handler))
				, allocator_(allocator)
			{}

			write_handler_t(write_handler_t &&rhs)
				: stream_(rhs.stream_)
				, buffer_(std::move(rhs.buffer_))
				, condition_(rhs.condition_)
				, transfers_(rhs.transfers_)
				, total_(rhs.total_)
				, handler_(std::move(rhs.handler_))
				, allocator_(std::move(rhs.allocator_))
			{

			}

			~write_handler_t()
			{}

		private:
			write_handler_t(const write_handler_t &);
			write_handler_t &operator=(const write_handler_t &);

		public:
			void operator()(const std::error_code &error, std::uint32_t size)
			{
				transfers_ += size;
				std::uint32_t left = total_ - transfers_;

				if( transfers_ < total_ && size != 0 && !error )
				{
					if( transfers_ < condition_() )
					{
						std::uint32_t write_len = 0;
						if( left > details::MAX_BUFFER_LEN )
							write_len = details::MAX_BUFFER_LEN;
						else
							write_len = left;

						try
						{
							ConstBufferT const_buf = buffer((buffer_ + transfers_).data(), write_len);
							this_type this_val(stream_, buffer_, total_, condition_, transfers_, std::move(handler_), allocator_);
							stream_.async_write(const_buf, std::move(this_val), allocator_);
							return;
						}
						catch(::exception::exception_base &e)
						{
							const_cast<std::error_code &>(error) = e.code();
							e.dump();
						}
					}
				}

				// 回调
				handler_(error, transfers_);
			}
		};

		template<typename AsyncWriteStreamT, typename ConstBufferT, typename CompletionConditionT, typename HandlerT>
		class write_offset_handler_t
		{
			typedef write_offset_handler_t<AsyncWriteStreamT, ConstBufferT, CompletionConditionT, HandlerT>	ThisType;

		public:
			AsyncWriteStreamT &stream_;
			ConstBufferT buffer_;
			CompletionConditionT condition_;
			const std::uint64_t offset_;
			std::uint32_t transfers_;
			const std::uint32_t total_;
			HandlerT handler_;

		public:
			write_offset_handler_t(AsyncWriteStreamT &stream, const ConstBufferT &buffer, std::uint32_t total, const std::uint64_t &offset, const CompletionConditionT &condition, std::uint32_t transfer, HandlerT &&handler)
				: stream_(stream)
				, buffer_(std::move(buffer))
				, offset_(offset)
				, condition_(condition)
				, transfers_(transfer)
				, total_(total)
				, handler_(std::move(handler))
			{}

			void operator()(const std::error_code &error, std::uint32_t size)
			{
				transfers_ += size;

				if( transfers_ < total_ && size != 0 && !error )
				{
					if( transfers_ < condition_() )
					{
						try
						{
							stream_.async_write(buffer_ + size, offset_,
								ThisType(stream_, buffer_ + size, total_, offset_, condition_, transfers_, std::forward<HandlerT>(handler_)));

							return;
						}
						catch(exception::exception_base &e)
						{
							const_cast<std::error_code &>(error) = e.code();
							e.dump();
						}
					}
				}

				// 回调
				handler_(error, transfers_);
			}
		};
	}

	// 异步写入指定的数据

	//
	template<typename SyncWriteStreamT, typename ConstBufferT, typename HandlerT, typename AllocatorT>
	void async_write(SyncWriteStreamT &s, const ConstBufferT &buffer, const HandlerT &handler, AllocatorT &allocator)
	{
		async_write(s, buffer, transfer_all(), handler, allocator);
	}

	template<typename SyncWriteStreamT, typename ConstBufferT, typename HandlerT, typename AllocatorT>
	void async_write(SyncWriteStreamT &s, const ConstBufferT &buffer, const std::uint64_t &offset, const HandlerT &handler, AllocatorT &allocator)
	{
		async_write(s, buffer, offset, transfer_all(), handler, allocator);
	}

	// 
	template<typename SyncWriteStreamT, typename ConstBufferT, typename ComplateConditionT, typename HandlerT, typename AllocatorT>
	void async_write(SyncWriteStreamT &s, const ConstBufferT &buf, const ComplateConditionT &condition, HandlerT &&handler, AllocatorT &allocator)
	{
		typedef details::write_handler_t<SyncWriteStreamT, ConstBufferT, ComplateConditionT, HandlerT, AllocatorT> HookWriteHandler;

		HookWriteHandler hook_handler(s, buf, buf.size(), condition, 0, std::forward<HandlerT>(handler), allocator);
		s.async_write(buf, std::move(hook_handler), allocator);
	}

	template<typename SyncWriteStreamT, typename ConstBufferT, typename ComplateConditionT, typename HandlerT>
	void async_write(SyncWriteStreamT &s, const ConstBufferT &buffer, const std::uint64_t &offset, const ComplateConditionT &condition, HandlerT &&handler)
	{
		typedef details::write_offset_handler_t<SyncWriteStreamT, ConstBufferT, ComplateConditionT, HandlerT> HookWriteHandler;

		s.async_write(buffer, offset, HookWriteHandler(s, buffer, buffer.size(), offset, condition, 0, std::forward<HandlerT>(handler)));
	}

}
}







#endif