#include "MessagerParser.hpp"
#include "behave/behave.hpp"
#include "libcommon/protocol/LxGame.pb.h"
#include "libcommon/protocol/LxGameEnums.h"
#include "libcommon/protocol/LxGameHelper.h"
#include "libcommon/ThreadLog/ThreadLog.h"
void MessagerParser::OnMessage(const LxGame::WrapPacket& packet)
{
    int64 cmd = packet.cmd();
    switch(cmd)
    {
    case EnumNetworkCmd::DATA_UserLoginResp:
        robot::handleLoginRes(*robot_, packet);
        break;
    case EnumNetworkCmd::FISH_FishEnterRoomResp:
        break;
    case EnumNetworkCmd::USER_SyncUserInfoChange:
        break;
    case EnumNetworkCmd::FISH_SyncCreateFish:
        break;
    }
}