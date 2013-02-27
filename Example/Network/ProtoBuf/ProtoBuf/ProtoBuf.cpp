// ProtoBuf.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Codec.hpp"
#include "Dispatcher.hpp"
#include "Server.hpp"

#include <iostream>

using std::cout;
using std::endl;

#include "test.pb.h"

#pragma comment(lib, "libprotobuf")


void print(const std::string& buf)
{
	printf("encoded to %zd bytes\n", buf.c_str());
	for (size_t i = 0; i < buf.length(); ++i)
	{
		unsigned char ch = static_cast<unsigned char>(buf[i]);

		printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
	}
}

using namespace async;

void TestQuery()
{
	typedef ProtobufCodec<network::Tcp::Socket> DefaultProtoBuf;

	muduo::Query query;
	query.set_id(1);
	query.set_questioner("Chen Shuo");
	query.add_question("Running?");

	std::string buf;
	DefaultProtoBuf::FillBuffer(buf, query);
	print(buf);

	size_t len = 0;
	memcpy(&len, buf.data(), sizeof(len));

	DefaultProtoBuf::ErrorCode errorCode = DefaultProtoBuf::kNoError;
	MessagePtr message = DefaultProtoBuf::Parse(buf.data() + sizeof(size_t), len, &errorCode);
	assert(errorCode == DefaultProtoBuf::kNoError);
	assert(message != NULL);
	message->PrintDebugString();
	assert(message->DebugString() == query.DebugString());

	std::tr1::shared_ptr<muduo::Query> newQuery = std::tr1::static_pointer_cast<muduo::Query>(message);
	assert(newQuery != NULL);
}


void testAnswer()
{
	typedef ProtobufCodec<network::Tcp::Socket> DefaultProtoBuf;

	muduo::Answer answer;
	answer.set_id(1);
	answer.set_questioner("Chen Shuo");
	answer.set_answerer("blog.csdn.net/Solstice");
	answer.add_solution("Jump!");
	answer.add_solution("Win!");

	std::string buf;
	DefaultProtoBuf::FillBuffer(buf, answer);
	print(buf);

	size_t len = 0;
	memcpy(&len, buf.data(), sizeof(len));

	DefaultProtoBuf::ErrorCode errorCode = DefaultProtoBuf::kNoError;
	MessagePtr message = DefaultProtoBuf::Parse(buf.data() + sizeof(size_t), len, &errorCode);
	assert(errorCode == DefaultProtoBuf::kNoError);
	assert(message != NULL);
	message->PrintDebugString();
	assert(message->DebugString() == answer.DebugString());

	std::tr1::shared_ptr<muduo::Answer> newAnswer = std::tr1::static_pointer_cast<muduo::Answer>(message);
	assert(newAnswer != NULL);
}


void testEmpty()
{
	muduo::Empty empty;

	typedef ProtobufCodec<network::Tcp::Socket> DefaultProtoBuf;

	std::string buf;
	DefaultProtoBuf::FillBuffer(buf, empty);
	print(buf);

	size_t len = 0;
	memcpy(&len, buf.data(), sizeof(len));

	DefaultProtoBuf::ErrorCode errorCode = DefaultProtoBuf::kNoError;
	MessagePtr message = DefaultProtoBuf::Parse(buf.data() + sizeof(size_t), len, &errorCode);
	assert(message != NULL);
	message->PrintDebugString();
	assert(message->DebugString() == empty.DebugString());
}


void redoCheckSum(std::string& data, int len)
{
	int checkSum = 0;/*sockets::hostToNetwork32(static_cast<int32_t>(
		::adler32(1,
		reinterpret_cast<const Bytef*>(data.c_str()),
		static_cast<int>(len - 4))));*/
	data[len-4] = reinterpret_cast<const char*>(&checkSum)[0];
	data[len-3] = reinterpret_cast<const char*>(&checkSum)[1];
	data[len-2] = reinterpret_cast<const char*>(&checkSum)[2];
	data[len-1] = reinterpret_cast<const char*>(&checkSum)[3];
}


void testBadBuffer()
{
	typedef ProtobufCodec<network::Tcp::Socket> DefaultProtoBuf;

	muduo::Empty empty;
	empty.set_id(43);

	std::string buf;
	DefaultProtoBuf::FillBuffer(buf, empty);
	// print(buf);

	size_t len = 0;
	memcpy(&len, buf.data(), sizeof(len));

	{
		std::string data(buf.data(), len);
		DefaultProtoBuf::ErrorCode errorCode = DefaultProtoBuf::kNoError;
		MessagePtr message = DefaultProtoBuf::Parse(data.c_str() + sizeof(size_t), len-1, &errorCode);
		assert(message == NULL);
		assert(errorCode == DefaultProtoBuf::kCheckSumError);
	}

	{
		std::string data(buf.data(), len);
		DefaultProtoBuf::ErrorCode errorCode = DefaultProtoBuf::kNoError;
		data[len-1]++;
		MessagePtr message = DefaultProtoBuf::Parse(data.c_str() + sizeof(size_t), len, &errorCode);
		assert(message == NULL);
		assert(errorCode == DefaultProtoBuf::kCheckSumError);
	}

	{
		std::string data(buf.data(), len);
		DefaultProtoBuf::ErrorCode errorCode = DefaultProtoBuf::kNoError;
		data[0]++;
		MessagePtr message = DefaultProtoBuf::Parse(data.c_str() + sizeof(size_t), len, &errorCode);
		assert(message == NULL);
		assert(errorCode == DefaultProtoBuf::kCheckSumError);
	}

}


int g_count = 0;

void onMessage(const network::Tcp::Socket& conn, const MessagePtr& message) 
{
	g_count++;
}

void testOnMessage()
{
	typedef ProtobufCodec<network::Tcp::Socket> DefaultProtoBuf;

	muduo::Query query;
	query.set_id(1);
	query.set_questioner("Chen Shuo");
	query.add_question("Running?");

	std::string buf1;
	DefaultProtoBuf::FillBuffer(buf1, query);

	muduo::Empty empty;
	empty.set_id(43);
	empty.set_id(1982);

	std::string buf2;
	DefaultProtoBuf::FillBuffer(buf2, empty);

	size_t totalLen = buf1.size() + buf2.size();
	std::string all;
	all.append(buf1.data(), buf1.size());
	all.append(buf2.data(), buf2.size());
	assert(all.size() == totalLen);


	network::SocketPtr sock;
	network::Tcp::Socket conn(sock);
	DefaultProtoBuf codec(onMessage);
	for (size_t len = 0; len <= totalLen; ++len)
	{
		std::string input;
		input.append(all.data(), len);

		g_count = 0;
		codec.OnMessage(conn, iocp::Buffer(input));
		int expected = len < buf1.size() ? 0 : 1;
		if (len == totalLen) expected = 2;
		assert(g_count == expected);
		// printf("%2zd %d\n", len, g_count);

		input.append(all.data() + len, totalLen - len);
		codec.OnMessage(conn, iocp::Buffer(input));
		assert(g_count == 2);
	}
}




typedef std::tr1::shared_ptr<muduo::Query> QueryPtr;
typedef std::tr1::shared_ptr<muduo::Answer> AnswerPtr;

void onQuery(async::network::Tcp::Socket&, const QueryPtr& message)
{
	cout << "onQuery: " << message->GetTypeName() << endl;
}

void onAnswer(async::network::Tcp::Socket&, const AnswerPtr& message)
{
	cout << "onAnswer: " << message->GetTypeName() << endl;
}

void onUnknownMessageType(async::network::Tcp::Socket&, const MessagePtr& message)
{
	cout << "onUnknownMessageType: " << message->GetTypeName() << endl;
}


void TestDispatcher()
{

	dispatch::ProtobufDispatcher<network::Tcp::Socket> dispatcher(onUnknownMessageType);
	dispatcher.RegisterMessageCallback<muduo::Query>(onQuery);
	dispatcher.RegisterMessageCallback<muduo::Answer>(onAnswer);

	async::network::SocketPtr sock;
	async::network::Tcp::Socket conn(sock);

	QueryPtr query(new muduo::Query);
	AnswerPtr answer(new muduo::Answer);
	std::tr1::shared_ptr<muduo::Empty> empty(new muduo::Empty);

	dispatcher.OnProtobufMessage(conn, query);
	dispatcher.OnProtobufMessage(conn, answer);
	dispatcher.OnProtobufMessage(conn, empty);

	struct NotMessage
	{};

	std::tr1::shared_ptr<NotMessage> not(new NotMessage);
	//dispatcher.RegisterMessageCallback<NotMessage>(onQuery);
}



void OnQuery(Session &session, const MessagePtr& message)
{
	std::cout << "onQuery:\n" << message->GetTypeName() << message->DebugString();
	
	session.Send(*message);

	//conn->shutdown();
}

void OnAnswer(Session &session, const MessagePtr& message)
{
	std::cout << "onAnswer: " << message->GetTypeName();
	
	session.Send(*message);
}

void TestServer()
{
	async::iocp::IODispatcher io;
	Server svr(io, 5050);

	using namespace std::tr1::placeholders;
	svr.Register<muduo::Query>(std::tr1::bind(&OnQuery, _1, _2));
	svr.Register<muduo::Answer>(std::tr1::bind(&OnAnswer, _1, _2));

	svr.Start();
	

	system("pause");

	svr.Stop();
}

int _tmain(int argc, _TCHAR* argv[])
{	
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	TestQuery();

	testAnswer();

	testEmpty();

	testBadBuffer();

	//testOnMessage();
	
	TestDispatcher();

	TestServer();
	

	google::protobuf::ShutdownProtobufLibrary();

	system("pause");
	return 0;
}

