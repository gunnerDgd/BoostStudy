#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <thread>

#define HELLO_MESSAGE "Hello Boost Asio\n"
std::vector<boost::asio::ip::tcp::socket*> ClientVector, CreatedClient;

void TcpClientReceive(boost::asio::mutable_buffer    * ClientBuffer,
					  boost::asio::ip::tcp::socket   & ClientSocket, 
					  const boost::system::error_code& ErrorCode   , 
					  std::size_t					   RecvSize)
{
	std::cout << "Received : " << reinterpret_cast<char*>(ClientBuffer->data()) << std::endl;
	ClientSocket.async_receive(*ClientBuffer, boost::bind(TcpClientReceive		  , 
														  ClientBuffer			  ,
														  boost::ref(ClientSocket), 
														  std::placeholders::_1   , 
														  std::placeholders::_2)) ;
}

void TcpAcceptor(boost::asio::ip::tcp::acceptor& Acceptor, const boost::system::error_code& ErrorCode, boost::asio::ip::tcp::socket ClientSocket)
{
	boost::asio::ip::tcp::socket* ClientTcp = new boost::asio::ip::tcp::socket(std::move(ClientSocket));
	ClientVector	   .push_back(ClientTcp);

	boost::asio::const_buffer    HelloMessage(HELLO_MESSAGE, strlen(HELLO_MESSAGE));
	ClientTcp			  ->send(HelloMessage);

	std::cout << "Sent\n";
	Acceptor.async_accept(boost::bind(TcpAcceptor, boost::ref(Acceptor), std::placeholders::_1, std::placeholders::_2));
}

int main()
{
	boost::asio::io_context		   TcpServerContext;
	boost::asio::ip::tcp::endpoint TcpServerEndpoint(boost::asio::ip::address::from_string("0.0.0.0"), 6500);
	boost::asio::ip::tcp::acceptor TcpServerAcceptor(TcpServerContext);
	
	TcpServerAcceptor.open  (boost::asio::ip::tcp::v4());
	TcpServerAcceptor.bind  (TcpServerEndpoint);
	
	TcpServerAcceptor.listen	  ();
	TcpServerAcceptor.async_accept(boost::bind(TcpAcceptor, boost::ref(TcpServerAcceptor), std::placeholders::_1, std::placeholders::_2));

	std::jthread TcpClientCreator([&TcpServerContext]()
		{
			while (true)
			{
				Sleep(1000);
				boost::asio::ip::tcp::endpoint ClientEndpoint(boost::asio::ip::address::from_string("127.0.0.1"), 6500);
				boost::asio::ip::tcp::socket  *ClientSocket  = new boost::asio::ip::tcp::socket(TcpServerContext);
				boost::asio::mutable_buffer   *ClientBuffer  = new boost::asio::mutable_buffer(new char[255], 255);

				ClientSocket->connect  (ClientEndpoint);
				CreatedClient.push_back(ClientSocket);

				ClientSocket->async_receive(*ClientBuffer, boost::bind(TcpClientReceive,
											 ClientBuffer,
											 boost::ref(*ClientSocket),
											 std::placeholders::_1,
											 std::placeholders::_2));
			}
		});

	while (true)
		TcpServerContext.run();
}