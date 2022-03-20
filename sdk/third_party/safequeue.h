/**********************************************************************
*
* Copyright (C)2015-2022 Maxwell Analytica. All rights reserved.
*
* @file safequeue.h
* @brief
* @details
* @author Maxwell
* @version 1.0.0
* @date 2018-12-09 22:41:27.492
*
**********************************************************************/
#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <queue>
#include <mutex>
#include <memory>
#include <string>
#include <atomic>
#include <condition_variable>

namespace ma
{
    class Semaphore
    {
    public:
        Semaphore()
        {
            count = 0;
        }
        void signal()
        {
            std::unique_lock<std::mutex> lock(mtx);
            ++count;
            cv.notify_one();
        }
        void wait()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [=] {
                return count > 0;
                });
            --count;
        }

    private:
        long count;
        std::mutex mtx;
        std::condition_variable cv;
    };

    template<typename T>
    class Safequeue
    {
    public:
        Safequeue()
        {
            terminate = false;
        }
        ~Safequeue(void)
        {

        }
        bool waitPop(T& value)
        {
            std::unique_lock<mutex> lock(mtx);
            data_cond.wait(lock, [this]{ return ((!data_queue.empty()) || terminate); });
            if (!data_queue.empty())
            {
                value = std::move(*data_queue.front());
                data_queue.pop();
                return true;
            }
            return false;
        }
        bool tryPop(T& value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (data_queue.empty())
                return false;
            value = move(*data_queue.front());
            data_queue.pop();
            return true;
        }
        std::shared_ptr<T> waitPop()
        {
            std::unique_lock<std::mutex> lock(mtx);
            data_cond.wait(lock, [this] { return ((!data_queue.empty()) || terminate); });
            if (!data_queue.empty())
            {
                std::shared_ptr<T> res = data_queue.front();
                data_queue.pop();
                return res;
            }
            return nullptr;
        }
        std::shared_ptr<T> tryPop()
        {
            std::lock_guard<std::mutex> lock(mtx);
            if(data_queue.empty())
            {
                return nullptr;
            }
            std::shared_ptr<T> res = data_queue.front();
            data_queue.pop();
            return res;
        }
        void push(T new_value)
        {
            if (terminate)
                return;
            std::shared_ptr<T> data(make_shared<T>(move(new_value)));
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(data);
            data_cond.notify_one();
        }
        bool empty()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return data_queue.empty();
        }
        int size()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return data_queue.size();
        }
        void exit()
        {
            std::lock_guard<std::mutex> lock(mtx);
            terminate = true;
            data_cond.notify_all();
        }
        bool isExit()
        {
            return terminate;
        }
        void clear()
        {
            if (data_queue.size() == 0)
                return;
            std::lock_guard<mutex> lock(mtx);
            std::queue<std::shared_ptr<T>> empty;
            std::swap(empty, data_queue);
        }
    private:
        std::mutex mtx;
        std::queue<std::shared_ptr<T>> data_queue;
        std::condition_variable data_cond;
        std::atomic<bool> terminate;
    };
}

#endif // SAFEQUEUE_H
