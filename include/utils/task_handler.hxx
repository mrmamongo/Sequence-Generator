//
// Created by MrMam on 14.09.2022.
//

#ifndef SEQ_GENERATOR_TASK_HANDLER_HXX
#define SEQ_GENERATOR_TASK_HANDLER_HXX

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class task_handler {
public:
    task_handler(size_t);
    template<class F, class... Args>
    auto execute(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
    ~task_handler();
    void stop() {
        _stop.store(true);
    }
private:
    std::vector< std::thread > workers;
    std::queue< std::function<void()> > tasks;

    /// Синхронизация
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic_bool _stop;
};

/// \brief Конструктор, инициализирует пул потоков
/// \param threads Количество потоков
inline task_handler::task_handler(size_t threads)
        :   _stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
                [this]
                {
                    for(;;)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                                 [this]{ return this->_stop || !this->tasks.empty(); });
                            if(this->_stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        task();
                    }
                }
        );
}

/// \brief Добавление задачи в очередь
/// \tparam F - тип функции
/// \tparam Args - типы аргументов
/// \param f
/// \param args
/// \return
template<class F, class... Args>
auto task_handler::execute(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(_stop)
            throw std::runtime_error("execute on stopped task_handler");
        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

/// \brief Деструктор, ожидает завершения всех потоков
inline task_handler::~task_handler()
{
    for(std::thread &worker: workers)
        worker.join();
}

#endif //SEQ_GENERATOR_TASK_HANDLER_HXX
