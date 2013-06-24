#include <iostream>
#include <mutex>
#include <vector>
#include <cstdint>
#include <string>

#include <async_io/network.hpp>
#include "handler_allocator.hpp"


class stats_t
{
	std::mutex mutex_;
	size_t total_msgs_read_;
	size_t total_bytes_read_;

public:
	stats_t()
		: mutex_()
		, total_msgs_read_(0)
		, total_bytes_read_(0)
	{
	}

	void add(size_t msgs_read, size_t bytes_read)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		total_msgs_read_ += msgs_read;
		total_bytes_read_ += bytes_read;
	}

	void print(std::uint32_t timeout)
	{
		std::lock_guard<std::mutex>  lock(mutex_);
		std::cout << total_msgs_read_ << " total msg read" << std::endl;
		std::cout << total_bytes_read_ << " total bytes read" << std::endl;

		std::cout << static_cast<double>(total_bytes_read_) / static_cast<double>(total_msgs_read_)
			<< " average message size" << std::endl;
		std::cout << static_cast<double>(total_bytes_read_) / (timeout * 1024 * 1024)
			<< " MiB/s throughput" << std::endl;
	}
};

using namespace async;

class session_t
{
	stats_t &stats_;
	service::io_dispatcher_t &io_;

	network::client cli_;
	std::vector<char> read_buf_;
	std::vector<char> write_buf_;

	std::uint32_t read_bytes_;
	std::uint32_t write_bytes_;

	std::uint32_t read_msg_cnt_;

	handler_allocator read_allocator_;
	handler_allocator write_allocator_;

public:
	session_t(stats_t &stats, service::io_dispatcher_t &io, timer::win_timer_service_t &timer_svr, std::uint32_t block_size)
		: stats_(stats)
		, io_(io)
		, cli_(io_, timer_svr)
		, read_bytes_(0)
		, write_bytes_(0)
		, read_msg_cnt_(0)
	{
		read_buf_.resize(block_size);
		write_buf_.resize(block_size);

		for(std::uint32_t i = 0; i < block_size; ++i)
			write_buf_[i] = static_cast<char>(i % 128);
	}
	~session_t()
	{
		stats_.add(read_msg_cnt_, read_bytes_);
	}

public:
	void start(const std::string &ip, std::uint16_t port)
	{
		cli_.register_connect_handler([this](bool suc)
		{
			if( !suc )
				return;

			write();
			read();
		});
		cli_.register_disconnect_handler([]()
		{

		});
		cli_.register_error_handler([](const std::string &msg)
		{
			std::cout << msg << std::endl;
		});

		cli_.start(ip, port, std::chrono::seconds(3));
	}

	void stop()
	{
		//io_.post(std::bind(&network::client::stop, std::ref(cli_)));
		cli_.stop();
	}

	void read()
	{
		cli_.async_recv(read_buf_.data(), read_buf_.size(), 
			make_custom_handler(read_allocator_, [this](std::uint32_t len)
		{
			++read_msg_cnt_;
			read_bytes_ += len;

			std::swap(read_buf_, write_buf_);
			write();
		}));
	}

	void write()
	{
		cli_.async_send(write_buf_.data(), write_buf_.size(), 
			make_custom_handler(write_allocator_, [this](std::uint32_t len)
		{
			write_bytes_ += len;
			read();
		}));
	}

};

void client_start(char **argv)
{
	stats_t stats;

	std::string ip = argv[0];
	std::uint16_t port = std::atoi(argv[1]);
	std::uint32_t session_cnt = std::atoi(argv[2]);
	std::uint32_t thr_cnt = atoi(argv[3]);
	std::uint32_t block_size = atoi(argv[4]);
	std::uint32_t timeout = atoi(argv[5]);

	std::list<session_t *> sessions;

	service::io_dispatcher_t io([](const std::string &msg){ std::cout << msg << std::endl;}, thr_cnt);
	service::io_dispatcher_t timer_io([](const std::string &){}, 1);

	timer::win_timer_service_t timer_svr(timer_io);

	for(auto i = 0; i != session_cnt; ++i)
	{
		auto session = new session_t(stats, io, timer_svr, block_size);
		sessions.push_back(session);
	}

	timer::timer_handle timer(timer_svr, timeout, timeout, 
		[&sessions, &io]()
	{
		/*std::for_each(sessions.begin(), sessions.end(), 
		[](session_t *s)
		{
		io.post([](const std::error_code &, std::uint32_t)
		{
		s->stop();
		delete s;
		});

		});*/

		io.stop();

		sessions.clear();
		std::cout << "press any key exit..." << std::endl;
	});

	timer.async_wait();

	std::for_each(sessions.begin(), sessions.end(), 
		[ip, port, &io](session_t *session)
	{
		io.post(std::bind(&session_t::start, session, ip, port));
	});

	std::cin.get();
	timer.cancel();
	timer_svr.stop();
	stats.print(timeout);

}