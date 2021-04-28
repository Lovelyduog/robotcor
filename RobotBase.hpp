#pragma once
namespace robot{
// 抽象基类为了可扩展性
class RobotBase
{
public:
    RobotBase() = default;
    virtual ~RobotBase(){};
    virtual void Run() = 0;
protected:

};

}//end namespace robot