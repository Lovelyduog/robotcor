namespace robot{
template<typename T>
RobotManager<T>* RobotManager<T>::instance_ = nullptr;

template<typename T>
RobotManager<T>::RobotManager(std::size_t num):ThreadPool(num)
{

}


template<typename T>
void RobotManager<T>::CreateRobot(std::size_t id, const std::string& account, const std::string& token)
{
    ANY_PTR robot(new T(GetIoContext(id), "192.168.0.5", "8080", "/"));
    robot->SetAccount(account);
    robot->SetToken(token);
    robots_.push_back(robot);
}

template<typename T>
RobotManager<T>* RobotManager<T>::Instance()
{
    if(!instance_)
    {
        instance_ = new RobotManager<T>(5);
    }
    return instance_;
}

template<typename T>
void RobotManager<T>::StartAllRobot()
{
    for(int i = 0; i < robots_.size(); ++i)
    {
        robots_[i]->Run();
    }
}
}//end namespace robot