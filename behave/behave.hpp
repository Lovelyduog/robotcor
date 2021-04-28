#pragma once
// #include <google/protobuf/stubs/common.h>
#include <vector>
#include "../libcommon/protocol/LxGame.pb.h"

//namespace introduced for LxGamehelper complete
using namespace std;
using namespace google::protobuf;
namespace robot{

class WebRobot;

void handleLoginRes(WebRobot& robot, const LxGame::WrapPacket& packet);
void handleEnterRoomRes(WebRobot& robot,const LxGame::WrapPacket& packet);
// ordinary reward

// growth behave

// game behave

}//end namespace robot
