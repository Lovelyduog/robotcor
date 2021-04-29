#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/bind/bind.hpp>
#include "websocekt_connect.hpp"
#include <functional>
#include "../WebRobot.hpp"
#include "libcommon/ThreadLog/ThreadLog.h"
#include "MessagerParser.hpp"

void other_work(boost::asio::yield_context yield_context)
{

}

namespace conn{

WebConnect::WebConnect(robot::WebRobot& robot,boost::asio::io_context& ioc,const std::string& host, 
    const std::string& port, const std::string& target)
    :ConnectBase(ioc, host, port),
     target_(target), websocket_(ioc),robot_(robot)
{
    parser_ = nullptr;
}

// keep reading  after building connect
void WebConnect::Connect()
{
    boost::asio::spawn(ioc_, [&](net::yield_context yield){
        for(;;)
        {
            tcp::resolver resolver_(ioc_);
            beast::error_code ec;
            auto result = resolver_.async_resolve(host_, port_, yield[ec]);
            if(ec)
            {
                continue;
            }
            beast::get_lowest_layer(websocket_).expires_after(std::chrono::seconds(30));
            auto ep = beast::get_lowest_layer(websocket_).async_connect(result, yield[ec]);
            if (ec) {
                continue;
            }

            host_ += ':' + std::to_string(ep.port());
            beast::get_lowest_layer(websocket_).expires_never();
            websocket_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
            websocket_.async_handshake(host_, target_.c_str(), yield[ec]);
            if (ec){
                websocket_.close(websocket::close_code::normal);
                continue;
            }
            robot_.Login();
            while(true)
            {
                beast::flat_buffer read_buffer;
                websocket_.async_read(read_buffer, yield[ec]);
                if(ec)
                {
                    websocket_.close(websocket::close_code::normal);
                    break;
                }

                LxGame::WrapPacket pack;
                if(!pack.ParseFromArray((void *)read_buffer.data().data(), read_buffer.size()))
                {
                    break;
                }

                if(parser_ == nullptr)
                {
                    LOGERROR("fatal error, parser is null, stop connect!!!!!");
                    return;
                }
                parser_->OnMessage(pack);
                read_buffer.consume(read_buffer.size());
                read_buffer.clear();
            }
        }
    });
}

void WebConnect::Stop()
{

}

void WebConnect::Restart()
{

}

void WebConnect::SendMessage(const LxGame::WrapPacket& packet)
{
    sendMessage(packet);
}

void WebConnect::sendMessage(const LxGame::WrapPacket& packet)
{
    ioc_.post(boost::bind(&WebConnect::sendMessageImp, this, packet));
}

void WebConnect::sendMessageImp(const LxGame::WrapPacket& packet)
{
	bool write_in_progress = !send_list_.empty();
	send_list_.push_back(packet);
	if (!write_in_progress) {
		doSend();
	}
}

void WebConnect::doSend()
{
	if( !websocket_.is_open() ) {
		return;
	}
	if( !send_list_.empty() ) {
		std::string mess_send;
		send_list_.front().SerializeToString(&mess_send);
		websocket_.binary(true);
		websocket_.async_write(
			net::buffer(mess_send),
			beast::bind_front_handler(
				&WebConnect::on_send,
				this));
	}
}

void WebConnect::on_send(boost::beast::error_code ec, std::size_t bytes_transferred)
{
	if (!ec)
	{
		send_list_.pop_front();
		if( !send_list_.empty() ) {
			doSend();
		}
	}
	else
	{
        LOGERROR("send result error, close the websocket");
		websocket_.close(websocket::close_code::normal);
	}
}
}//end namespace conn