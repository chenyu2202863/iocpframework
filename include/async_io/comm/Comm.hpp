#ifndef __COMM_HPP
#define __COMM_HPP

#include "../IOCP/Dispatcher.hpp"
#include "../IOCP/Buffer.hpp"
#include "../IOCP/Read.hpp"
#include "../IOCP/Write.hpp"
#include "../IOCP/ReadWriteBuffer.hpp"


namespace async
{
	using namespace iocp;

	namespace comm
	{
		// forward declare

		class Comm;
		typedef std::tr1::shared_ptr<Comm> CommPtr;


		//--------------------------------------------------------------------------------
		// class File

		class Comm
		{
		public:
			typedef iocp::IODispatcher	AsyncIODispatcherType;

		private:
			// Comm Handle
			HANDLE comm_;
			// IO服务
			AsyncIODispatcherType &io_;

		public:
			explicit Comm(AsyncIODispatcherType &);
			Comm(AsyncIODispatcherType &, const std::string &device);
			~Comm();

			// non-copyable
		private:
			Comm(const Comm &);
			Comm &operator=(const Comm &);


		public:
			// explicit转换
			operator HANDLE()					{ return comm_; }
			operator const HANDLE () const		{ return comm_; }

			// 显示获取
			HANDLE GetHandle()					{ return comm_; }
			const HANDLE GetHandle() const		{ return comm_; }

		public:
			// 打开目标文件
			void Open(const std::string &device);
			// 关闭
			void Close();

			// 是否打开
			bool IsOpen() const
			{ return comm_ != INVALID_HANDLE_VALUE; }

			//	取消
			bool Cancel();

			// 设置状态
			void SetState(const DCB &dcb);

			// 获取状态
			DCB GetState() const;

			// 设置超时
			void SetTimeOut();

			// 获取超时
			void GetTimeOut() const;
			

			// 设置属性
			template < typename OptionT > 
			void SetOption(const OptionT &option);

			// 获取属性
			template < typename OptionT >
			void GetOption(OptionT &option);
			
			// 不需设置回调接口,同步函数
		public:
			size_t Read(void *buf, size_t len);
			size_t Write(const void *buf, size_t len);

			// 异步调用接口
		public:
			void AsyncRead(void *buf, size_t len, const CallbackType &handler);
			void AsyncWrite(const void *buf, size_t len, const CallbackType &handler);
		};
	}



	namespace iocp
	{
		typedef comm::Comm Comm;

		// File 工厂
		template<>
		struct ObjectFactory< Comm >
		{
			typedef memory::FixedMemoryPool<true, sizeof(Comm)>		PoolType;
			typedef ObjectPool< PoolType >							ObjectPoolType;
		};
	}


	namespace comm
	{
		inline CommPtr MakeComm(Comm::AsyncIODispatcherType &io)
		{
			return CommPtr(ObjectAllocate<Comm>(io), &ObjectDeallocate<Comm>);
		}

		inline CommPtr MakeComm(Comm::AsyncIODispatcherType &io, const std::string &device)
		{
			return CommPtr(ObjectAllocate<Comm>(io, device), &ObjectDeallocate<Comm>);
		}
	}


	template < typename OptionT > 
	void Comm::SetOption(const OptionT &option)
	{
		if( !IsOpen() )
			throw std::logic_error("Comm not open");
	
		DCB dcb = GetState();
		option.Store(dcb);

		SetState(dcb);
	}

	// 获取属性
	template < typename OptionT >
	void Comm::GetOption(OptionT &option)
	{
		if( !IsOpen() )
			throw std::logic_error("Comm not open");
	
		DCB dcb = GetState();

		option.Load(dcb);
	}
}




#endif