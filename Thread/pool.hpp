#pragma once
#include <boost/core/noncopyable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <vector>
#include <thread>
class ThreadPool: public boost::noncopyable
{
public:
    ThreadPool(std::size_t num);
    ~ThreadPool();
    void Run();
    void Wait();
protected:
    boost::asio::io_context& GetIoContext(std::size_t id);
private:
    typedef std::thread THREAD;
    typedef boost::shared_ptr<boost::asio::io_context> IO_CONTEXT;
    typedef boost::shared_ptr<boost::asio::io_context::work> WORK;
private:
    void join();
    void stop();
private:
    std::size_t thread_num_;
    std::vector<THREAD> threads_;
    std::vector<IO_CONTEXT> iocs_;
    std::vector<WORK> works_;
};
