#pragma once
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <list>
#include <cstdlib>
#include <semaphore.h>
#include "connect.hpp"

class MessagerParser;
namespace robot
{
    class WebRobot;
}

namespace conn{

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


class WebConnect: public ConnectBase
{
public:
    WebConnect(robot::WebRobot& robot,boost::asio::io_context& ioc,const std::string& host, const std::string& port, const std::string& target);
    void Connect();
    void Stop();
    void Restart();
    bool IsOpen(){return websocket_.is_open();};
    void SendMessage(const LxGame::WrapPacket& packet);
    void SetParser(MessagerParser* parser){parser_ = parser;};
private:
    void sendMessage(const LxGame::WrapPacket& packet);
    void sendMessageImp(const LxGame::WrapPacket& packet);
    void doSend();
    void on_send(boost::beast::error_code ec, std::size_t bytes_transferred);
private:
    std::string target_;
    websocket::stream<beast::tcp_stream> websocket_;
    std::list<LxGame::WrapPacket> send_list_;
    MessagerParser* parser_;
    robot::WebRobot& robot_;
};
}//end namespace conn