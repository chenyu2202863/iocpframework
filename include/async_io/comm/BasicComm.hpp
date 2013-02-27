#ifndef __COMM_BASIC_COMM_HPP
#define __COMM_BASIC_COMM_HPP

#include "Comm.hpp"
#include "Option.hpp"

#include "../iocp/ReadWriteBuffer.hpp"
#include "../iocp/Write.hpp"
#include "../iocp/Read.hpp"


namespace async
{
	namespace comm
	{

		// -------------------------------------------
		// class BasicComm

		class BasicComm
		{
		public:
			typedef CommPtr							ImplementType;
			typedef Comm::AsyncIODispatcherType		AsyncIODispatcherType;	

		private:
			ImplementType impl_;

		public:
			explicit BasicComm(AsyncIODispatcherType &io)
				: impl_(MakeComm(io))
			{}
			explicit BasicComm(const ImplementType &impl)
				: impl_(impl)
			{}
			BasicComm(AsyncIODispatcherType &io, const std::string &device)
				: impl_(MakeComm(io, device))
			{}

		public:
			// 显示获取
			ImplementType &Get() 
			{
				return impl_;
			}
			const ImplementType &Get() const
			{
				return impl_;
			}

			// 支持隐式转换
			operator ImplementType()
			{
				return impl_;
			}
			operator const ImplementType&() const
			{
				return impl_;
			}

		public:
			void Open(const std::string &device)
			{
				return impl_->Open(device);
			}

			void Close()
			{
				return impl_->Close();
			}

			bool IsOpen() const
			{
				return impl_->IsOpen();
			}

			template < typename OptionT >
			void GetOption(OptionT &option)
			{
				impl_->GetOption(option);
			}

			template < typename OptionT >
			void SetOption(const OptionT &option)
			{
				impl_->SetOption(option);
			}

		public:
			// 阻塞式发送数据直到数据发送成功或出错
			template<typename ConstBufferT>
			size_t Write(const ConstBufferT &buffer)
			{
				return impl_->Write(buffer.data(), buffer.size());
			}

			// 异步发送数据
			template<typename ConstBufferT, typename HandlerT>
			void AsyncWrite(const ConstBufferT &buffer, const HandlerT &callback)
			{
				return impl_->AsyncWrite(buffer.data(), buffer.size(), callback);
			}


			// 阻塞式接收数据直到成功或出错
			template<typename MutableBufferT>
			size_t Read(MutableBufferT &buffer)
			{
				return impl_->Read(buffer.data(), buffer.size());
			}

			// 异步接收数据
			template<typename MutableBufferT, typename HandlerT>
			void AsyncRead(MutableBufferT &buffer, const HandlerT &callback)
			{
				return impl_->AsyncRead(buffer.data(), buffer.size(), callback);
			}
		};
	}
}



#endif