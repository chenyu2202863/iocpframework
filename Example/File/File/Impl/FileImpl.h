#ifndef __FILE_IMPL_HPP
#define __FILE_IMPL_HPP

#include <iostream>
#include <algorithm>

#include <iocp/dispatcher.hpp> // for io_dispatcher
#include <file/basic_file.hpp> // for basic_file	
#include <iocp/buffer.hpp>


using namespace async;	// 纯粹少敲代码,别鄙视


// 圆整为扇区大小倍数
template<typename U>
inline U round(const U &m)
{
	return (m + 512 - 1) & ~(512 - 1);
}



class file_async_rw
{
private:
	//std::tr1::array<char, 8192> buf
	iocp::auto_buffer_ptr buf_;
	u_int64 offset_;

	filesystem::basic_file readFile_;
	filesystem::basic_file writeFile_;

	u_int64 fileSize_;

public:
	file_async_rw(async::iocp::io_dispatcher &io, LPCTSTR lpszReadFile, LPCTSTR lpszWriteFile)
		: readFile_(io, lpszReadFile, filesystem::overlapped_read())
		, writeFile_(io, lpszWriteFile, filesystem::overlapped_write())
		, offset_(0)
		, buf_(async::iocp::make_buffer(8192))
	{
		//std::uninitialized_fill(buf_.begin(), buf_.end(), 0);
		HANDLE file = ::CreateFile(lpszReadFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		assert(file != INVALID_HANDLE_VALUE);

		LARGE_INTEGER size = {0};
		if( !::GetFileSizeEx(file, &size) )
			throw iocp::win32_exception("GetFileSizeEx");
		fileSize_ = size.QuadPart;

		::CloseHandle(file);
	}

public:
	void start()
	{
		try
		{
			size_t left = (size_t)min(fileSize_ - offset_, 8192);
			if( left == 0 )
			{
				stop();
				return;
			}

			iocp::async_read(readFile_, iocp::buffer(buf_->data(), round(left)), offset_, iocp::transfer_at_leat(left), 
				std::bind(&file_async_rw::_on_read, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void stop()
	{
		readFile_.close();

		writeFile_.close();
	}

private:
	void _on_read(iocp::error_code error, u_long size)
	{
		if( error != 0 )
		{
			assert(0);
			return;
		}

		try
		{
			iocp::async_write(writeFile_, iocp::buffer(buf_->data(), size), offset_, iocp::transfer_at_leat(size), 
				std::bind(&file_async_rw::_on_write, this, iocp::_Error, iocp::_Size));
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void _on_write(iocp::error_code error, u_long size)
	{
		if( error != 0 )
		{
			assert(0);
			return;
		}

		offset_ += size;
		start();
	}
};







#endif