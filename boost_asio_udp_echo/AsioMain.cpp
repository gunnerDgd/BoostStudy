#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ip/udp.hpp>

#include <thread>
#include <iostream>

#include <string_view>

const char* TestMessage = "Hello World";

void UdpRecvExec(boost::asio::ip::udp::socket&    UdpSocket,
				 boost::asio::mutable_buffer&     UdpSocketBuffer,
				 const boost::system::error_code& UdpErrorCode, std::size_t UdpRecvByte)
{
	UdpSocket.receive(UdpSocketBuffer);

	std::cout << std::string_view((char*)UdpSocketBuffer.data(), UdpRecvByte) << std::endl;
	
	UdpSocket.async_receive(UdpSocketBuffer,
							boost::bind(UdpRecvExec		   , 
										std::ref(UdpSocket), 
										std::ref(UdpSocketBuffer), 
										boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void UdpSendExec(boost::asio::ip::udp::socket& UdpSocket, boost::asio::ip::udp::endpoint& UdpEndpoint)
{
	using namespace std::chrono_literals;
	boost::asio::const_buffer UdpSendBuffer(TestMessage, 11);

	while (true)
	{
		std::this_thread::sleep_for(1000ms);
		UdpSocket		 .send_to  (UdpSendBuffer, UdpEndpoint);
	}
}

void UdpExecLoop(boost::asio::io_context& UdpIoContext)
{
	while (true)
		UdpIoContext.run();
}

int main()
{
	boost::asio::io_context        UdpSocketContext;
	boost::asio::mutable_buffer	   UdpSocketBuffer (new char[255], 255);

	boost::asio::ip::udp::endpoint UdpEndpoint(boost::asio::ip::address::from_string("127.0.0.1"), 6500);
	boost::asio::ip::udp::socket   UdpSocket  (UdpSocketContext, UdpEndpoint);
	
	UdpSocket.async_receive(UdpSocketBuffer,
							boost::bind(UdpRecvExec,
										std::ref(UdpSocket),
										std::ref(UdpSocketBuffer),
										boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	std::jthread				   UdpSendThread(UdpSendExec, std::ref(UdpSocket), std::ref(UdpEndpoint)), 
								   UdpExecThread(UdpExecLoop, std::ref(UdpSocketContext));
}