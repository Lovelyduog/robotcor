#pragma once
#include <string>
#include <boost/asio/io_context.hpp>
#include "libcommon/protocol/LxGame.pb.h"
namespace conn{

class ConnectBase
{
public:
    virtual void Connect() = 0;
    virtual void Stop() = 0;
    virtual void Restart() = 0;
    virtual void SendMessage(const LxGame::WrapPacket& packet) = 0;
    virtual ~ConnectBase() = default;
public:
    ConnectBase(boost::asio::io_context& ioc,const std::string& host, const std::string& port)
    :ioc_(ioc),host_(host),port_(port){};
protected:
protected:
    boost::asio::io_context& ioc_;
    std::string host_;
    std::string port_;
};
}//end namespace conn