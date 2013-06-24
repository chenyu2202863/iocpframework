#ifndef __ASYNC_SERVICE_READ_HELPER_HPP
#define __ASYNC_SERVICE_READ_HELPER_HPP

#include <cstdint>
#include <system_error>

#include "condition.hpp"
#include "exception.hpp"

namespace async { namespace service {

	//
	template<typename SyncWriteStreamT, typename MutableBufferT>
	std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer)
	{
		return read(s, buffer, transfer_all(), [](std::error_code, std::uint32_t){});
	}

	template<typename SyncWriteStreamT, typename MutableBufferT, typename HandlerT >
	std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, const HandlerT &callback)
	{
		return read(s, buffer, transfer_all(), callback);
	}

	template<typename SyncWriteStreamT, typename MutableBufferT >
	std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, const std::uint64_t &offset)
	{
		return read(s, buffer, offset, transfer_all());
	}

	template<typename SyncWriteStreamT, typename MutableBufferT, typename CompleteConditionT, typename HandlerT >
	std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, CompleteConditionT &condition, const HandlerT &callback)
	{
		std::uint32_t transfers = 0;
		const std::uint32_t bufSize = buffer.size();

		while( transfers <= condition(transfers) )
		{
			if( transfers >= bufSize )
				break;

			std::uint32_t ret = s.read(buffer + transfers);
			if( ret == 0 )
			{
				s.close();
			}

			transfers += ret;

			callback(std::make_error_code((std::errc::errc)::GetLastError()), transfers);
		}

		return transfers;
	}

	template<typename SyncWriteStreamT, typename MutableBufferT, typename CompleteConditionT>
	std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, const std::uint64_t &offset, CompleteConditionT &condition)
	{
		std::uint32_t transfers = 0;
		const std::uint32_t bufSize = buffer.size();

		while( transfers <= condition(transfers) )
		{
			if( transfers >= bufSize )
				break;

			std::uint32_t ret = s.read(buffer + transfers, offset);
			if( ret == 0 )
				s.close();

			transfers += ret;
		}

		return transfers;
	}



	namespace details
	{
		template< typename AsyncWriteStreamT, typename MutableBufferT, typename CompletionConditionT, typename HandlerT >
		class read_handler_t
		{
			typedef read_handler_t<AsyncWriteStreamT, MutableBufferT, CompletionConditionT, HandlerT> this_type;

		public:
			AsyncWriteStreamT &stream_;
			MutableBufferT buffer_;
			CompletionConditionT condition_;
			std::uint32_t transfers_;
			const std::uint32_t total_;
			HandlerT handler_;

		public:
			read_handler_t(AsyncWriteStreamT &stream, MutableBufferT &&buffer, std::uint32_t total, const CompletionConditionT &condition, std::uint32_t transfer, HandlerT &&handler)
				: stream_(stream)
				, buffer_(std::move(buffer))
				, condition_(condition)
				, transfers_(transfer)
				, total_(total)
				, handler_(std::move(handler))
			{}

			read_handler_t(read_handler_t &&rhs)
				: stream_(rhs.stream_)
				, buffer_(std::move(rhs.buffer_))
				, condition_(rhs.condition_)
				, transfers_(rhs.transfers_)
				, total_(rhs.total_)
				, handler_(std::move(rhs.handler_))
			{
			}

			~read_handler_t()
			{

			}

		private:
			read_handler_t(const read_handler_t &);
			read_handler_t &operator=(const read_handler_t &);

		public:
			void operator()(const std::error_code &error, std::uint32_t size)
			{
				transfers_ += size;
				std::uint32_t left = total_ - transfers_;

				if( transfers_ < total_ && size != 0 && error == 0 )
				{
					if( transfers_ < condition_() )
					{
						std::uint32_t read_len = 0;
						if( left > details::MAX_BUFFER_LEN )
							read_len = details::MAX_BUFFER_LEN;
						else
							read_len = left;

						try
						{
							MutableBufferT mutable_buf = buffer((buffer_ + transfers_).data(), read_len);
							this_type this_val(stream_, std::move(buffer_), total_, condition_, transfers_, std::move(handler_));
							stream_.async_read(mutable_buf, std::move(this_val));
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
				handler_(std::cref(error), transfers_);
			}

			friend void *allocate_handler(std::uint32_t sz, this_type *this_handler)
			{
				return service::allocate_handler(sz, this_handler->handler_);
			}

			friend void deallocate_handler(void *p, std::uint32_t sz, this_type *this_handler)
			{
				service::deallocate_handler(p, sz, this_handler->handler_);
			}
		};

		template< typename AsyncWriteStreamT, typename MutableBufferT, typename OffsetT, typename CompletionConditionT, typename HandlerT >
		class read_offset_handler_t
		{
			typedef read_offset_handler_t<AsyncWriteStreamT, MutableBufferT, OffsetT, CompletionConditionT, HandlerT>	this_type;

		public:
			AsyncWriteStreamT &stream_;
			MutableBufferT buffer_;
			CompletionConditionT condition_;
			const OffsetT offset_;
			std::uint32_t transfers_;
			const std::uint32_t total_;
			HandlerT handler_;

		public:
			read_offset_handler_t(AsyncWriteStreamT &stream, MutableBufferT &&buffer, std::uint32_t total, const OffsetT &offset, const CompletionConditionT &condition, std::uint32_t transfer, HandlerT &&handler)
				: stream_(stream)
				, buffer_(std::move(buffer))
				, condition_(condition)
				, offset_(offset)
				, transfers_(transfer)
				, total_(total)
				, handler_(std::move(handler))
			{}

			void operator()(const std::error_code &error, std::uint32_t size)
			{
				transfers_ += size; 

				if( transfers_ < total_ && size != 0 && error == 0 )
				{
					if( transfers_ < condition_() )
					{
						try
						{
							stream_.async_read(buffer_ + size, offset_,
								this_type(stream_, buffer_ + size, total_, offset_, condition_, transfers_, std::forward<HandlerT>(handler_)));

							return;
						}
						catch(::exception::exception_base &e)
						{
							error = e.code();
							e.dump();
						}
					}
				}

				// 回调
				handler_(std::cref(error), transfers_);
			}

			friend void *allocate_handler(std::uint32_t sz, this_type *this_handler)
			{
				allocate_handler(sz, this_handler->handler_);
			}

			friend void deallocate_handler(void *p, std::uint32_t sz, this_type *this_handler)
			{
				deallocate_handler(p, sz, this_handler->handler_);
			}
		};
	}

	// 异步读取指定的数据

	//
	template<typename SyncWriteStreamT, typename MutableBufferT, typename HandlerT>
	void async_read(SyncWriteStreamT &s, MutableBufferT &&buffer, const HandlerT &handler)
	{
		async_read(s, std::forward<MutableBufferT>(buffer), transfer_all(), handler);
	}

	template<typename SyncWriteStreamT, typename MutableBufferT, typename HandlerT>
	void async_read(SyncWriteStreamT &s, MutableBufferT &&buffer, const LARGE_INTEGER &offset, const HandlerT &handler)
	{
		async_read(s, std::forward<MutableBufferT>(buffer), offset, transfer_all(), handler);
	}

	// 
	template<typename SyncWriteStreamT, typename MutableBufferT, typename ComplateConditionT, typename HandlerT>
	void async_read(SyncWriteStreamT &s, MutableBufferT &&buf, const ComplateConditionT &condition, HandlerT &&handler)
	{
		typedef std::remove_reference<MutableBufferT>::type mutable_buffer_t;
		typedef details::read_handler_t<SyncWriteStreamT, mutable_buffer_t, ComplateConditionT, HandlerT> HookReadHandler;

		HookReadHandler hook_handler(s, std::forward<mutable_buffer_t>(buf), buf.size(), condition, 0, std::forward<HandlerT>(handler));
		s.async_read(hook_handler.buffer_, std::move(hook_handler));
	}

	template<typename SyncWriteStreamT, typename MutableBufferT, typename OffsetT, typename ComplateConditionT, typename HandlerT>
	void async_read(SyncWriteStreamT &s, MutableBufferT &&buffer, const OffsetT &offset, const ComplateConditionT &condition, HandlerT &&handler)
	{
		typedef details::read_offset_handler_t<SyncWriteStreamT, MutableBufferT, OffsetT, ComplateConditionT, HandlerT> HookReadHandler;

		s.async_read(buffer, offset, HookReadHandler(s, std::forward<MutableBufferT>(buffer), buffer.size(), offset, condition, 0, std::forward<HandlerT>(handler)));
	}

}
}







#endif