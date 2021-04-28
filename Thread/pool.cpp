#include "pool.hpp"

// create io
ThreadPool::ThreadPool(std::size_t num)
{
    for(std::size_t i = 0; i < num; ++i)
    {
        IO_CONTEXT ioc(new boost::asio::io_context());
        WORK work(new boost::asio::io_context::work(*ioc));
        iocs_.push_back(ioc);
        works_.push_back(work);
    }

    thread_num_ = iocs_.size();
}
ThreadPool::~ThreadPool()
{
    
}
// create thread 
void ThreadPool::Run()
{
    if(iocs_.size() != works_.size())
    {
        return;
    }

    for(std::size_t i = 0; i < iocs_.size(); ++i)
    {
        threads_.emplace_back([this,i](){
            iocs_[i]->run();
        });
    }
}


void ThreadPool::Wait()
{
    join();
}

void ThreadPool::join()
{
    for(std::vector<THREAD>::iterator it = threads_.begin(); it != threads_.end(); ++it)
    {
        (*it).join();
    }
}

void ThreadPool::stop()
{

}

boost::asio::io_context& ThreadPool::GetIoContext(std::size_t id)
{
    return *iocs_[id%iocs_.size()];
}
