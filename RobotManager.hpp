#pragma once
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/asio/io_context.hpp>
#include "Thread/pool.hpp"
namespace robot{
template <typename T>
class RobotManager: public ThreadPool
{
public:
    static RobotManager* Instance();
    void CreateRobot(std::size_t id, const std::string& account, const std::string& token);
    void StartAllRobot();
private:
    RobotManager(std::size_t num);
    typedef boost::shared_ptr<T> ANY_PTR;
private:
    std::vector<ANY_PTR> robots_;
    static RobotManager* instance_;
};

}//end namespace robot
#include "RobotManager.ipp"