#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <thread>

void TestEnqueueThread(boost::asio::io_context::executor_type& IoExecutor)
{
	using namespace std::chrono_literals;

	while (true)
	{
		std::this_thread::sleep_for(1000ms);
		boost::asio::post(IoExecutor, []() {std::cout << "Worker Run\n"; });
	}
}

void TestWorkerThread(boost::asio::io_context& IoContext)
{
	while (true)
		IoContext.run();
}

int main()
{
	boost::asio::io_context IoContext;
	auto					IoExecutor = IoContext.get_executor();

	std::jthread IoEnqueueThread(TestEnqueueThread, std::ref(IoExecutor)),
				 IoWorkerThread (TestWorkerThread, std::ref(IoContext));
}