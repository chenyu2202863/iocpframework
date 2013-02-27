#ifndef __PIPE_HPP
#define __PIPE_HPP



#include "../IOCP/Dispatcher.hpp"


namespace async
{
	namespace pipe
	{
		using namespace iocp;

		class Pipe;
		typedef std::tr1::shared_ptr<Pipe> PipePtr;

		// --------------------------------
		// class Pipe

		class Pipe
		{
		public:
			typedef iocp::IODispatcher	DispatcherType;

		private:
			// File Handle
			HANDLE pipe_;
			// IO服务
			DispatcherType &io_;

		public:
			explicit Pipe(DispatcherType &);
			Pipe(DispatcherType &, HANDLE);
			Pipe(DispatcherType &);
			~Pipe();

			// non-copyable
		private:
			Pipe(const Pipe &);
			Pipe &operator=(const Pipe &);

		public:
			// explicit转换
			operator HANDLE()					{ return pipe_; }
			operator const HANDLE () const		{ return pipe_; }

			// 显示获取
			HANDLE GetHandle()					{ return pipe_; }
			const HANDLE GetHandle() const		{ return pipe_; }

		public:
			// 创建
			void Create();

			// 打开目标文件
			void Open();
			// 关闭
			void Close();

			// 是否打开
			bool IsOpen() const
			{ return pipe_ != INVALID_HANDLE_VALUE; }

			// 连接
			void Connect();




			// 不需设置回调接口,同步函数
		public:
			size_t Read(const void *buf, size_t len);
			size_t Write(const void *buf, size_t len);

			void AsyncRead();
			void AsyncWrite();
		};
	}
}


#endif