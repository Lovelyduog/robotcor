#pragma once
#include "libcommon/protocol/LxGame.pb.h"

namespace robot{
    class WebRobot;
}

class MessagerParser
{
public:
    MessagerParser(robot::WebRobot* owner){robot_ = owner;};
    void OnMessage(const LxGame::WrapPacket& packet);
private:
    
private:
    robot::WebRobot* robot_;
};