#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include "parseHttp.h"
#include <fstream>

namespace Network
{
  
  namespace Private
  {
    
    class Connection
        : private boost::noncopyable
        , public boost::enable_shared_from_this<Connection>
    {
    public:
      Connection(boost::asio::io_service &ioService)
        : Strand(ioService)
        , Socket(ioService)
      {
      }
      boost::asio::ip::tcp::socket& GetSocket()
      {
        return Socket;
      }
      void Start()
      {
        Socket.async_read_some(boost::asio::buffer(Buffer),
                               Strand.wrap(
                                 boost::bind(&Connection::HandleRead, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred)
                                 ));
      }
      void HandleRead(boost::system::error_code const &error, std::size_t bytes)
      {
        if (error)
          return;
       
        std::vector<boost::asio::const_buffer> Buffers;
		
		
		//Raw request
		std::string rawHeader( Buffer.data() );
		parseHttpHeader(rawHeader);
		formAnswer();

		std::cout<<"Raw Request:\n"<<rawHeader<<std::endl;
		std::cout<<"Formed answer:\n"<<httpMessage.message;


        Buffers.push_back(boost::asio::const_buffer(httpMessage.message.c_str(), 
									httpMessage.message.size() ));

        boost::asio::async_write(Socket, Buffers,
                                 Strand.wrap(
                                   boost::bind(&Connection::HandleWrite, shared_from_this(),
                                               boost::asio::placeholders::error)
                                   ));
      }
      void HandleWrite(boost::system::error_code const &error)
      {
        if (error)
          return;
        
        boost::system::error_code Code;
        Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, Code);
      }
      
    private:
      boost::array<char, 4096> Buffer;
      boost::asio::io_service::strand Strand;
      boost::asio::ip::tcp::socket Socket;
    };
    
  }
  
  class EchoServer
      : private boost::noncopyable
  {
  public:
    EchoServer(std::string const& locAddr, std::string const& port, unsigned threadsCount)
      : Acceptor(IoService)
      , Threads(threadsCount)
    {
      boost::asio::ip::tcp::resolver Resolver(IoService);
      boost::asio::ip::tcp::resolver::query Query(locAddr, port);
      boost::asio::ip::tcp::endpoint Endpoint = *Resolver.resolve(Query);
      Acceptor.open(Endpoint.protocol());
      Acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      Acceptor.bind(Endpoint);
      Acceptor.listen();
      
      StartAccept();
      
      std::generate(Threads.begin(), Threads.end(),
                    boost::bind(
                      &boost::make_shared<boost::thread, boost::function<void ()> const &>,
                      boost::function<void ()>(boost::bind(&boost::asio::io_service::run, &IoService))
                      ));
    }
    ~EchoServer()
    {
      std::for_each(Threads.begin(), Threads.end(),
                    boost::bind(&boost::asio::io_service::stop, &IoService));
      std::for_each(Threads.begin(), Threads.end(),
                    boost::bind(&boost::thread::join, _1));
    }
    
  private:
    boost::asio::io_service IoService;
    boost::asio::ip::tcp::acceptor Acceptor;
    
    typedef boost::shared_ptr<Private::Connection> ConnectionPtr;
    
    ConnectionPtr NewConnection;
    
    typedef boost::shared_ptr<boost::thread> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPool;
    
    ThreadPool Threads;
    
    void StartAccept()
    {
      NewConnection = boost::make_shared<Private::Connection, boost::asio::io_service &>(IoService);
      Acceptor.async_accept(NewConnection->GetSocket(),
                            boost::bind(&EchoServer::HandleAccept, this,
                                        boost::asio::placeholders::error));
    }
    void HandleAccept(boost::system::error_code const &error)
    {
      if (!error)
        NewConnection->Start();
      StartAccept();
    }
  };
  
}

int main(int argc, char* argv[])
{
	std::string szHost;
	std::string szPort;
	std::string szDir;
	int flag;
	while( (flag = getopt(argc, argv, ":h:p:d:")) != EOF){
		switch(flag){
			case 'h':
				szHost = optarg;
				break;
			case 'p':
				szPort = optarg;
				break;
			case 'd':
				szDir = optarg;
				break;
			default:
				std::cout<<"Usage : final -h <ip> -p <port> -d <directory>"<<std::endl;
				break;		
		}	
	}
	std::cout<<szHost<<std::endl;
	std::cout<<szPort<<std::endl;
	std::cout<<szDir<<std::endl;
	
  try
  {
    Network::EchoServer Srv(szHost.c_str(), szPort.c_str(), 4);
    std::cin.get();
  }
  catch (std::exception const &e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
