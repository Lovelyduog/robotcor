#include "MessagerParser.hpp"
#include "WebRobot.hpp"
#include "libcommon/protocol/LxGame.pb.h"
#include "libcommon/protocol/LxGameEnums.h"
#include "libcommon/protocol/LxGameHelper.h"
#include "libcommon/ThreadLog/ThreadLog.h"


void MessagerParser::OnMessage(const LxGame::WrapPacket& packet)
{
    int64 cmd = packet.cmd();
    switch(cmd)
    {
    case DATA_UserLoginResp:
        robot_->handleLoginRes(packet);
        break;
    case FISH_FishEnterRoomResp:
        break;
    case USER_SyncUserInfoChange:
        break;
    case FISH_SyncCreateFish:
        break;
    }
}