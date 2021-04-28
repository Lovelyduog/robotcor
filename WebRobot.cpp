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
}

WebRobot::~WebRobot()
{
    delete parser_;
    delete connecter_;
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
        // for( ; ; ) {
        //         shot_fish_timer_.expires_from_now(boost::posix_time::milliseconds(500));
        //         shot_fish_timer_.async_wait(yield);
        //         if(!connecter_->IsOpen()) {
        //             break;
        //         }
        //         if(b_inRoom_) {
        //             LxGame::InputManualFire msg;
        //             msg.set_bulletid(m_nBulletId);
        //             msg.set_bulletangle(30);
        //             msg.set_bullettype(0);
        //             m_bullets.insert(m_nBulletId);
        //             LxGame::WrapPacket packet;
        //             LxGame::LxGameHelper::MakeInputManualFire(packet, msg);
        //             SendMessage(packet);
        //             m_nBulletId++;

        //             if( m_bullets.size() > 2 ) {
        //                 int32 fishId = 0;
        //                 int64 nowTick = GlobalUtils::GetTickCount();
        //                 while( !m_mapFish.empty() ) {
        //                     auto itFish = m_mapFish.begin();
        //                     if( itFish->second <= nowTick ) {
        //                         m_mapFish.erase(itFish);
        //                     }
        //                     else {
        //                         fishId = itFish->first;
        //                         break;
        //                     }
        //                 }
        //                 if( fishId != 0 ) {
        //                     InputHitFish msg;
        //                     int32 bulletId = *m_bullets.begin();
        //                     m_bullets.erase(bulletId);
        //                     msg.set_bulletid(bulletId);
        //                     msg.set_fishid(fishId);
        //                     WrapPacket packet;
        //                     LxGameHelper::MakeInputHitFish(packet, msg);
        //                     SendMessage(packet);
        //                 }
        //             }
        //         }
        //     }
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

}//end namespace robot