#include <string>
#include <boost/asio/spawn.hpp>
#include "WebRobot.hpp"
#include "MessagerParser.hpp"
#include "connect/websocekt_connect.hpp"
#include "libcommon/protocol/LxGame.pb.h"
#include "libcommon/protocol/LxGameHelper.h"
#include "ThreadLog/ThreadLog.h"


namespace robot{

WebRobot::WebRobot(boost::asio::io_context& ioc,const std::string& host, const std::string& port, const std::string& target)
:RobotBase(),ioc_(ioc),heart_beat_timer_(ioc),shot_fish_timer_(ioc)
{
    parser_ = new MessagerParser(this);
    connecter_ = new conn::WebConnect(*this ,ioc, host, port, target);
    b_inRoom_ = false;
    bullet_id_ = 0;
    status_= STATUS_UNLOGINED;
    room_id_ = 0;
    client_id_ = 0;
}

WebRobot::~WebRobot()
{
    delete parser_;
    delete connecter_;
}

std::map<int , DATA_ROOMS> WebRobot::s_people_number_2_room_Config;
std::map<int, Equips> WebRobot::s_equips_Config;
void WebRobot::LoadDataInLocal()
{
    // 加载一次
    if(s_people_number_2_room_Config.empty())
    {
        JDATA->GamePlayPtr()->ForEach([&](tagJsonGamePlay* data) {
            if(data->_ID >= 101 && data->_ID <= 105)
            {
                s_people_number_2_room_Config[data->_MaxPlayer][data->_ID] = data;
            }
        });
    }

    if(s_equips_Config.empty())
    {
        Equips _equips_task1(30, 0 , 0);
        Equips _equips_task2(60, 0 , 0);
        Equips _equips_task3(60, 6 , 0);
        Equips _equips_task4(60, 6 , 9);

        s_equips_Config[1] = _equips_task1;
        s_equips_Config[2] = _equips_task2;
        s_equips_Config[3] = _equips_task3;
        s_equips_Config[4] = _equips_task4;
    }
    int32 num = GlobalUtils::GetRandNumber(1, 4);
    if(!_equips_task.isInitted())
    {
        _equips_task = s_equips_Config[num];
    }
}

void WebRobot::Init()
{
    if(parser_ == nullptr)
    {
        assert(false);
        return;
    }
    connecter_->SetParser(parser_);
}

void WebRobot::Run()
{
    connecter_->SetParser(parser_);
    connecter_->Connect();
}

void WebRobot::StartHeartBeat()
{
    boost::asio::spawn(ioc_, [&](boost::asio::yield_context yield){
        for(;;)
        {
            heart_beat_timer_.expires_from_now(boost::posix_time::seconds(20));
            heart_beat_timer_.async_wait(yield);
            LxGame::UserHeartBeatReq req;
            LxGame::WrapPacket packet;
            LxGameHelper::MakeUserHeartBeatReq(packet);
            SendMessage(packet);
        }
    });

    boost::asio::spawn(ioc_, [&](boost::asio::yield_context yield){
        for( ; ; ) {
                shot_fish_timer_.expires_from_now(boost::posix_time::milliseconds(500));
                shot_fish_timer_.async_wait(yield);
                if(!connecter_->IsOpen()) {
                    break;
                }
                if(b_inRoom_) {
                    LxGame::InputManualFire msg;
                    msg.set_bulletid(bullet_id_);
                    msg.set_bulletangle(30);
                    msg.set_bullettype(0);
                    m_bullets.insert(bullet_id_);
                    LxGame::WrapPacket packet;
                    LxGameHelper::MakeInputManualFire(packet, msg);
                    SendMessage(packet);
                    bullet_id_++;

                    if( m_bullets.size() > 2 ) {
                        int32 fishId = 0;
                        int64 nowTick = GlobalUtils::GetTickCount();
                        while( !fishes_.empty() ) {
                            auto itFish = fishes_.begin();
                            if( itFish->second <= nowTick ) {
                                fishes_.erase(itFish);
                            }
                            else {
                                fishId = itFish->first;
                                break;
                            }
                        }
                        if( fishId != 0 ) {
                            InputHitFish msg;
                            int32 bulletId = *m_bullets.begin();
                            m_bullets.erase(bulletId);
                            msg.set_bulletid(bulletId);
                            msg.set_fishid(fishId);
                            WrapPacket packet;
                            LxGameHelper::MakeInputHitFish(packet, msg);
                            SendMessage(packet);
                        }
                    }
                }
            }
    });
}

void WebRobot::SendMessage(const LxGame::WrapPacket& packet)
{
    connecter_->SendMessage(packet);
}

void WebRobot::UpdateRoomFlag(bool status)
{
    b_inRoom_ = status;
}

void WebRobot::UpdateStatus(STATUS status)
{
    status_ = status;
}

}//end namespace robot