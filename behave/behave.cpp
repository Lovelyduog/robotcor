#include "behave.hpp"
#include "../WebRobot.hpp"
#include "../libcommon/protocol/LxGame.pb.h"
#include "../libcommon/protocol/LxGameEnums.h"
#include "../libcommon/protocol/LxGameHelper.h"
#include "../libcommon/ThreadLog/ThreadLog.h"

namespace robot{
void WebRobot::Login()
{
    LxGame::WrapPacket packet;
    LxGame::UserLoginReq req;
    req.set_account(account_);
    req.set_token(token_);
    req.set_login_type(1);
    LxGameHelper::MakeUserLoginReq(packet, req);
    SendMessage(packet);
}

void handleLoginRes(WebRobot& robot, const LxGame::WrapPacket& packet)
{
        LxGame::UserLoginResp user_info;
        if(!user_info.ParseFromString(packet.data()) )
        {
            LOGINFO("failed to login");
            assert(false);
            return;
        }

        int32 error_code = packet.errorcode();

        do{
            if(!error_code)
            {
                LOGINFO("login ok. %s", user_info.DebugString().data());
                robot.UpdateRoomFlag(true);
                break;
            }
            LOGERROR("login error is [%d]", error_code);
            robot.UpdateRoomFlag(false);
        }while(0);
}
void WebRobot::EnterRoom( int32 room_index)
{
    if(room_index > 0)
    {
        LOGINFO("LogicRobot::EnterRoom");
        LxGame::WrapPacket packet;
        LxGame::FishEnterRoomReq req;
        req.set_room_index(room_index);
        LxGameHelper::MakeFishEnterRoomReq(packet, req);
        SendMessage(packet);
    }
}

void handleEnterRoomRes(WebRobot& robot,const LxGame::WrapPacket& packet)
{
    LOGINFO("handleEnterRoomRes::");
    LxGame::FishEnterRoomResp resp;
    if(!resp.ParseFromString(packet.data()))
    {
        LOGINFO("failed to enter room");
        return;
    }

    int32 error_code = packet.errorcode();
    if (error_code)
    {
            LOGERROR("failed to enter room,error_code is[%d]", error_code);
    }
    //处理除了系统错误外的error_code
    do{
        if(!error_code)
        {
            // LOGINFO("enter room ok. %s", _room_config_infos.DebugString().data());
            // _bullet_id = 1;
        }

    }while(0);
}
}//end namespace robot