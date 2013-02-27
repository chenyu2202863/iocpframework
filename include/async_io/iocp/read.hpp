#ifndef __IOCP_READ_HELPER_HPP
#define __IOCP_READ_HELPER_HPP

#include <cstdint>
#include "internal/Condition.hpp"
#include "../../exception/exception_base.hpp"

namespace async
{
	namespace iocp
	{

		//
		template<typename SyncWriteStreamT, typename MutableBufferT>
		std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer)
		{
			return read(s, buffer, transfer_all(), 0);
		}

		template<typename SyncWriteStreamT, typename MutableBufferT>
		std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, const rw_callback_type &callback)
		{
			return read(s, buffer, transfer_all(), callback);
		}

		template<typename SyncWriteStreamT, typename MutableBufferT >
		std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, const std::uint64_t &offset)
		{
			return read(s, buffer, offset, transfer_all());
		}

		// 
		template<typename SyncWriteStreamT, typename MutableBufferT, typename CompleteConditionT>
		std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, CompleteConditionT &condition)
		{
			return read(s, buffer, condition, 0);
		}

		template<typename SyncWriteStreamT, typename MutableBufferT, typename CompleteConditionT>
		std::uint32_t read(SyncWriteStreamT &s, MutableBufferT &buffer, CompleteConditionT &condition, const rw_callback_type &callback)
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

				if( callback != 0 )
					callback(win32_error_2_error_code(::GetLastError()), transfers);
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



		namespace detail
		{
			template< typename AsyncWriteStreamT, typename MutableBufferT, typename CompletionConditionT, typename ReadHandlerT >
			class read_handler_t
			{
				typedef read_handler_t<AsyncWriteStreamT, MutableBufferT, CompletionConditionT, ReadHandlerT>	ThisType;

			private:
				AsyncWriteStreamT &stream_;
				MutableBufferT buffer_;
				CompletionConditionT condition_;
				std::uint32_t transfers_;
				const std::uint32_t total_;
				ReadHandlerT handler_;

			public:
				read_handler_t(AsyncWriteStreamT &stream, MutableBufferT &buffer, std::uint32_t total, const CompletionConditionT &condition, std::uint32_t transfer, const ReadHandlerT &handler)
					: stream_(stream)
					, buffer_(buffer)
					, condition_(condition)
					, transfers_(transfer)
					, total_(total)
					, handler_(handler)
				{}

				void operator()(error_code error, u_long size)
				{
					transfers_ += size;
					std::uint32_t left = total_ - transfers_;

					if( transfers_ < total_ && size != 0 && error == 0 )
					{
						if( transfers_ < condition_() )
						{
							std::uint32_t read_len = 0;
							if( left > detail::MAX_BUFFER_LEN )
								read_len = detail::MAX_BUFFER_LEN;
							else
								read_len = left;
		
							try
							{
								stream_.async_read(buffer((buffer_ + transfers_).data(), read_len),
									ThisType(stream_, buffer_, total_, condition_, transfers_, handler_));
								return;
							}
							catch(exception::exception_base &e)
							{
								error = win32_error_2_error_code(::GetLastError());
								e.dump();
							}
						}
					}
					
					// 回调	
					handler_(error, transfers_);
				}
			};

			template< typename AsyncWriteStreamT, typename MutableBufferT, typename OffsetT, typename CompletionConditionT, typename ReadHandlerT >
			class read_offset_handler_t
			{
				typedef read_offset_handler_t<AsyncWriteStreamT, MutableBufferT, OffsetT, CompletionConditionT, ReadHandlerT>	ThisType;

			private:
				AsyncWriteStreamT &stream_;
				MutableBufferT buffer_;
				CompletionConditionT condition_;
				const OffsetT offset_;
				std::uint32_t transfers_;
				const std::uint32_t total_;
				ReadHandlerT handler_;

			public:
				read_offset_handler_t(AsyncWriteStreamT &stream, MutableBufferT &buffer, std::uint32_t total, const OffsetT &offset, const CompletionConditionT &condition, std::uint32_t transfer, const ReadHandlerT &handler)
					: stream_(stream)
					, buffer_(buffer)
					, condition_(condition)
					, offset_(offset)
					, transfers_(transfer)
					, total_(total)
					, handler_(handler)
				{}

				void operator()(error_code error, u_long size)
				{
					transfers_ += size; 

					if( transfers_ < total_ && size != 0 && error == 0 )
					{
						if( transfers_ < condition_() )
						{
							try
							{
								stream_.async_read(buffer_ + size, offset_,
									ThisType(stream_, buffer_ + size, total_, offset_, condition_, transfers_, handler_));

								return;
							}
							catch(::exception::exception_base &e)
							{
								error = win32_error_2_error_code(::GetLastError());
								e.dump();
							}
						}
					}

					// 回调
					handler_(error, transfers_);
				}
			};
		}

		// 异步读取指定的数据

		//
		template<typename SyncWriteStreamT, typename MutableBufferT, typename HandlerT>
		void async_read(SyncWriteStreamT &s, MutableBufferT &buffer, const HandlerT &handler)
		{
			async_read(s, buffer, transfer_all(), handler);
		}

		template<typename SyncWriteStreamT, typename MutableBufferT, typename HandlerT>
		void async_read(SyncWriteStreamT &s, MutableBufferT &buffer, const LARGE_INTEGER &offset, const HandlerT &handler)
		{
			async_read(s, buffer, offset, transfer_all(), handler);
		}

		// 
		template<typename SyncWriteStreamT, typename MutableBufferT, typename ComplateConditionT, typename HandlerT>
		void async_read(SyncWriteStreamT &s, MutableBufferT &buf, const ComplateConditionT &condition, const HandlerT &handler)
		{
			typedef detail::read_handler_t<SyncWriteStreamT, MutableBufferT, ComplateConditionT, HandlerT> HookReadHandler;

			std::uint32_t recv_len = detail::MAX_BUFFER_LEN > buf.size() ? buf.size() : detail::MAX_BUFFER_LEN;
			s.async_read(buffer(buf.data(), recv_len), HookReadHandler(s, buf, buf.size(), condition, 0, handler));
		}
	
		template<typename SyncWriteStreamT, typename MutableBufferT, typename OffsetT, typename ComplateConditionT, typename HandlerT>
		void async_read(SyncWriteStreamT &s, MutableBufferT &buffer, const OffsetT &offset, const ComplateConditionT &condition, const HandlerT &handler)
		{
			typedef detail::read_offset_handler_t<SyncWriteStreamT, MutableBufferT, OffsetT, ComplateConditionT, HandlerT> HookReadHandler;

			s.async_read(buffer, offset, HookReadHandler(s, buffer, buffer.size(), offset, condition, 0, handler));
		}

	}
}







#endif