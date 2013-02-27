#ifndef __HUB_HPP
#define __HUB_HPP


#include <iocp\dispatcher.hpp>
#include <network\tcp.hpp>
#include <timer\timer.hpp>

#include <map>
#include <set>
#include <string>
#include <atltime.h>
#include <memory>
#include <iostream>

namespace pubsub
{
	typedef std::set<std::string> ConnectionSubscription;

	using namespace async;

	class Session;
	typedef std::shared_ptr<Session> SessionPtr;


	class Topic
	{
	private:
		std::string topic_;
		std::string content_;

		CTime lastPubTime_;

		std::set<SessionPtr> audiences_;
	
	private:
		std::string _MakeMessage()
		{
			return "pub " + topic_ + "\r\n" + content_ + "\r\n";
		}

	public:
		Topic(const std::string &topic)
			: topic_(topic)
		{
		}

		void Add(const SessionPtr & conn)
		{
			audiences_.insert(conn);
			if( lastPubTime_ != 0 )
			{
				//conn->AsyncWrite(iocp::Buffer(_MakeMessage()), 0);
			}
		}

		void Remove(const SessionPtr& conn)
		{
			audiences_.erase(conn);
		}

		void Publish(const std::string& content, CTime time)
		{
			content_ = content;
			lastPubTime_ = time;
			std::string message = _MakeMessage();
			for (auto it = audiences_.begin();
				it != audiences_.end();
				++it)
			{
				//(*it)->AsyncWrite(iocp::Buffer(message), 0);
			}
		}
	};


	class Session
		: public std::tr1::enable_shared_from_this<Session>
	{
		iocp::io_dispatcher &io_;
		network::tcp::socket sock_;
		ConnectionSubscription subs_;

	public:
		Session(iocp::io_dispatcher &io, const network::socket_handle_ptr &sock)
			: io_(io)
			, sock_(sock)
		{
			
		}

		void AddSubscription(const std::string &sub)
		{
			subs_.insert(sub);
		}

		void RemoveSubscription(const std::string &sub)
		{
			subs_.erase(sub);
		}
	};

	typedef std::shared_ptr<Session> SessionPtr;


	class PubSubServer
	{
		iocp::io_dispatcher &io_;
		network::tcp::accpetor acceptor_;
		std::map<std::string, Topic> topics_;
		timer::timer_handle timer_;
		
	public:
		PubSubServer(iocp::io_dispatcher &io, u_short port)
			: io_(io)
			, acceptor_(io_, network::tcp::v4(), port)
			, timer_(io)
		{
			timer_.async_wait(std::bind(&PubSubServer::_OnTimePublish, this), 3000);
		}

		void Start()
		{
			try
			{
				
				acceptor_.async_accept(0, std::tr1::bind(&PubSubServer::_OnConnect, this, iocp::_Error, iocp::_Socket));
			}
			catch(std::exception &e)
			{
				std::cerr << e.what() << std::endl;
			}
		}

	private:
		void _OnConnect(iocp::error_code err, const network::socket_handle_ptr &conn)
		{
			if( err != 0 )
				return;

			SessionPtr session(new Session(io_, conn));
			//session->Start();
		}

		void _OnMessage(iocp::error_code err, u_long size, const SessionPtr& conn, CTime receiveTime)
		{
			/*ParseResult result = kSuccess;
			while (result == kSuccess)
			{
				string cmd;
				string topic;
				string content;
				result = parseMessage(buf, &cmd, &topic, &content);
				if (result == kSuccess)
				{
					if (cmd == "pub")
					{
						doPublish(conn->name(), topic, content, receiveTime);
					}
					else if (cmd == "sub")
					{
						LOG_INFO << conn->name() << " subscribes " << topic;
						doSubscribe(conn, topic);
					}
					else if (cmd == "unsub")
					{
						_DoUnsubscribe(conn, topic);
					}
					else
					{
						conn->shutdown();
						result = kError;
					}
				}
				else if (result == kError)
				{
					conn->shutdown();
				}
			}*/
		}

		void _OnTimePublish()
		{
			CTime now = CTime::GetCurrentTime();
			_DoPublish("internal", "time", "%Y-%M-%D %H:%m:%S"/*(LPCTSTR)now.Format(L"%Y-%M-%D %H:%m:%S")*/, now);
		}

		void _DoSubscribe(const SessionPtr & conn, const std::string& topic)
		{
			conn->AddSubscription(topic);
			_GetTopic(topic).Add(conn);
		}

		void _DoUnsubscribe(const SessionPtr &conn, const std::string& topic)
		{
			//std::cout << conn->name() << " unsubscribes " << topic << std::endl;
			
			conn->RemoveSubscription(topic);
			_GetTopic(topic).Remove(conn);
		}

		void _DoPublish(const std::string& source, const std::string& topic, const std::string& content, CTime time)
		{
			_GetTopic(topic).Publish(content, time);
		}

		Topic& _GetTopic(const std::string& topic)
		{
			auto it = topics_.find(topic);
			if (it == topics_.end())
			{
				it = topics_.insert(std::make_pair(topic, Topic(topic))).first;
			}
			return it->second;
		}

		
	};
}




#endif