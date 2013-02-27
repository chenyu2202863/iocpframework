// Timer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"




void AsyncWait()
{
	std::cout << "AsyncWait" << std::endl;

	static int count = 0;
}


void AsyncWait2()
{
	std::cout << "AsyncWait2" << std::endl;
}


void AsyncWait3(async::timer::TimerPtr &timer)
{
	static int count = 1000;

	count += 500;

	try
	{
		if( count == 5000 )
			timer->SetTimer(count);
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	std::cout << "AsyncWait3" << std::endl;
}

void AsyncWait4()
{
	std::cout << "AsyncWait4" << std::endl;
}

void SyncWait5()
{
	std::cout << "SyncWait5" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	{
		async::iocp::IODispatcher io;
		
		try
		{
			{
				std::auto_ptr<async::timer::Timer> timer;
				timer.reset(new async::timer::Timer(io, 2000, 0, std::tr1::bind(&AsyncWait)));
				timer->AsyncWait();
 
				timer->Cancel();
				timer->SetTimer(10000);

				async::timer::Timer timer2(io, 4000, 0, std::tr1::bind(&AsyncWait2));
				timer2.AsyncWait();

				system("pause");
			}
			

			{
				async::timer::TimerPtr timer3(new async::timer::Timer(io));
				timer3->AsyncWait(std::tr1::bind(&AsyncWait3, std::tr1::ref(timer3)), 2000);
			
				system("pause");
			}

			{
				async::timer::Timer timer4(io);
				timer4.SyncWait(std::tr1::bind(&SyncWait5), 5000, 0);
			}

			async::timer::Timer timer4(io);
			timer4.AsyncWait(std::tr1::bind(&AsyncWait4), 500, 0);

			system("pause");
		}
		catch(std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}

	}
	
	

	system("pause");
	return 0;
}

