#include <string>
#include <boost/asio/io_context.hpp>
#include "WebRobot.hpp"
#include "RobotManager.hpp"
#include "ThreadLog/ThreadLog.h"

std::string generate_string(const std::string& prefix, int num)
{
    std::ostringstream str;
    str << prefix << num;
    return str.str();
}


int main()
{
    // boost::asio::io_context ioc;
    // boost::asio::io_context::work work(ioc);
    robot::RobotManager<robot::WebRobot>* robot_mgr = robot::RobotManager<robot::WebRobot>::Instance();
    if(!robot_mgr)
    {
        return -1;
    }
    robot_mgr->Run();
    for(int i = 0; i < 10; ++i)
    {
        robot_mgr->CreateRobot(i, generate_string("robot",i), generate_string("robot",i));
    }

    robot_mgr->StartAllRobot();
    robot_mgr->Wait();
    // ioc.run();
    return 0;
}
