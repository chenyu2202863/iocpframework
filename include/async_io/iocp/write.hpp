#ifndef __IOCP_WRITE_HELPER_HPP
#define __IOCP_WRITE_HELPER_HPP

#include <cstdint>
#include "internal/condition.hpp"


namespace async
{
	namespace iocp
	{

		// 
		template<typename SyncWriteStreamT, typename ConstBufferT>
		std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer)
		{
			return write(s, buffer, transfer_all(), 0);
		}

		template<typename SyncWriteStreamT, typename ConstBufferT>
		std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const rw_callback_type &callback)
		{
			return write(s, buffer, transfer_all(), callback);
		}

		template<typename SyncWriteStreamT, typename ConstBufferT>
		std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const std::uint64_t &offset)
		{
			return write(s, buffer, offset, transfer_all());
		}

		// 
		template<typename SyncWriteStreamT, typename ConstBufferT, typename CompleteConditionT>
		std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const CompleteConditionT &condition)
		{
			return write(s, buffer, condition, 0);
		}

		template<typename SyncWriteStreamT, typename ConstBufferT, typename CompleteConditionT>
		std::uint32_t write(SyncWriteStreamT &s, const ConstBufferT &buffer, const CompleteConditionT &condition, const rw_callback_type &callback)
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

				if( callback != 0 )
					callback(win32_error_2_error_code(::GetLastError()), transfers);
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



		namespace detail
		{
			template<typename AsyncWriteStreamT, typename ConstBufferT, typename CompletionConditionT, typename WriteHandlerT>
			class write_handler_t
			{
				typedef write_handler_t<AsyncWriteStreamT, ConstBufferT, CompletionConditionT, WriteHandlerT>	ThisType;
				
			private:
				AsyncWriteStreamT &stream_;
				ConstBufferT buffer_;
				CompletionConditionT condition_;
				std::uint32_t transfers_;
				const std::uint32_t total_;
				WriteHandlerT handler_;

			public:
				write_handler_t(AsyncWriteStreamT &stream, const ConstBufferT &buffer, std::uint32_t total, const CompletionConditionT &condition, std::uint32_t transfer, const WriteHandlerT &handler)
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
							std::uint32_t write_len = 0;
							if( left > detail::MAX_BUFFER_LEN )
								write_len = detail::MAX_BUFFER_LEN;
							else
								write_len = left;

							try
							{
								stream_.async_write(buffer((buffer_ + transfers_).data(), write_len),
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

			template<typename AsyncWriteStreamT, typename ConstBufferT, typename CompletionConditionT, typename WriteHandlerT>
			class write_offset_handler_t
			{
				typedef write_offset_handler_t<AsyncWriteStreamT, ConstBufferT, CompletionConditionT, WriteHandlerT>	ThisType;

			private:
				AsyncWriteStreamT &stream_;
				ConstBufferT buffer_;
				CompletionConditionT condition_;
				const u_int64 offset_;
				std::uint32_t transfers_;
				const std::uint32_t total_;
				WriteHandlerT handler_;

			public:
				write_offset_handler_t(AsyncWriteStreamT &stream, const ConstBufferT &buffer, std::uint32_t total, const u_int64 &offset, const CompletionConditionT &condition, std::uint32_t transfer, const WriteHandlerT &handler)
					: stream_(stream)
					, buffer_(buffer)
					, offset_(offset)
					, condition_(condition)
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
								stream_.async_write(buffer_ + size, offset_,
									ThisType(stream_, buffer_ + size, total_, offset_, condition_, transfers_, handler_));
							
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
		}

		// 异步写入指定的数据

		//
		template<typename SyncWriteStreamT, typename ConstBufferT, typename HandlerT>
		void async_write(SyncWriteStreamT &s, const ConstBufferT &buffer, const HandlerT &handler)
		{
			async_write(s, buffer, transfer_all(), handler);
		}

		template<typename SyncWriteStreamT, typename ConstBufferT, typename HandlerT>
		void async_write(SyncWriteStreamT &s, const ConstBufferT &buffer, const u_int64 &offset, const HandlerT &handler)
		{
			async_write(s, buffer, offset, transfer_all(), handler);
		}

		// 
		template<typename SyncWriteStreamT, typename ConstBufferT, typename ComplateConditionT, typename HandlerT>
		void async_write(SyncWriteStreamT &s, const ConstBufferT &buf, const ComplateConditionT &condition, const HandlerT &handler)
		{
			typedef detail::write_handler_t<SyncWriteStreamT, ConstBufferT, ComplateConditionT, HandlerT> HookWriteHandler;

			std::uint32_t send_len = detail::MAX_BUFFER_LEN > buf.size() ? buf.size() : detail::MAX_BUFFER_LEN;
			s.async_write(buffer(buf.data(), send_len), HookWriteHandler(s, buf, buf.size(), condition, 0, handler));
		}

		template<typename SyncWriteStreamT, typename ConstBufferT, typename ComplateConditionT, typename HandlerT>
		void async_write(SyncWriteStreamT &s, const ConstBufferT &buffer, const u_int64 &offset, const ComplateConditionT &condition, const HandlerT &handler)
		{
			typedef detail::write_offset_handler_t<SyncWriteStreamT, ConstBufferT, ComplateConditionT, HandlerT> HookWriteHandler;

			s.async_write(buffer, offset, HookWriteHandler(s, buffer, buffer.size(), offset, condition, 0, handler));
		}

	}
}







#endif