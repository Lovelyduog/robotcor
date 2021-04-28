#pragma once
#include <string>
#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "RobotBase.hpp"
#include "libcommon/protocol/LxGame.pb.h"

namespace conn{
    class WebConnect;
}
class MessagerParser;
using namespace std;
using namespace google::protobuf;

namespace robot{

class WebRobot : public RobotBase
{
public:
    WebRobot(boost::asio::io_context& ioc,const std::string& host, const std::string& port, const std::string& target);
    ~WebRobot();
    void Init();
    void Run();
    void StartHeartBeat();
    void SendMessage(const LxGame::WrapPacket& packet);
    void UpdateRoomFlag(bool status);
    void SetAccount(const std::string& account){account_ = account;};
    void SetToken(const std::string& token){ token_ = token;};
public:
    void Login();
    void EnterRoom(int32 room_index);
private:
    MessagerParser* parser_;
    conn::WebConnect* connecter_;
    boost::asio::io_context& ioc_;
    boost::asio::deadline_timer heart_beat_timer_;
    boost::asio::deadline_timer shot_fish_timer_;
    std::string account_;
    std::string token_;
    bool b_inRoom_;
    
};

}//end namespace robot