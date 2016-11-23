#ifndef INCLUDE_THREADSAFE_H
#define INLCUDE_THREADSAFE_H


#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <string>

template<typename QueueType>
class ThreadSafeQueue{

private:
    // variables for multithread
    mutable std::mutex mutex_;
    mutable std::condition_variable condition_empty_;       // Condition variable for queue empty
	mutable std::condition_variable condition_full_;        // Condition variable for queue full
    mutable std::atomic_bool shutdown_;                             // atomic variable for thread shutdown  

    // variable for raw buffer data:
    std::queue<QueueType> queue_;

public:
    ThreadSafeQueue():shutdown_(false) {}

    virtual ~ThreadSafeQueue():{ 
        shutdown_ = true; 
        NotifyAll();
    }

    virtual void NotifyAll()const {
        condition_empty_.notify_all();
        condition_full_.notify_all();
    }

    virtual void Shutdown() const {
        shutdown_ = true;
        NotifyAll();
    }

    virtual void Resume() const {
        shutdown_ = false;
        NotifyAll();
    }

    // Simply push value into the queue, no block and notify one of the thread asking for data..
    void PushNonBlocking(const QueueType* value){
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(value);
        }
        condition_empty_.notify_one();
    }

    // return the size of queue_..
    size_t Size() const final {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            size_t size = queue_.size();
        }
        return size;
    }

    // check whether the queue is empty..
    bool Empty() const final {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool empty = queue_.empty();
        }
        return empty;
    }

    // return false if the queue is show down. 
    // If the queue is full, block and wait for enough space to pushing...
    bool PushBlockingIfFull(const QueueType* value, size_t max_queue_size){
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Note shutdown will also wake up the waiting thread, and return false after that.
            condition_full_.wait(lock, [&]{ return ((queue_.size() != max_queue_size) || shutdown_); });
            if(shutdown_)
                return false;
            queue_.push(value);
        }
        condition_empty_.notify_one();
        return true;
    }

    // Almost never return false. 
    bool PushNonBlockDroppingIfFull(const QueueType* value, size_t max_queue_size){
        bool result = false;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(queue_.size() >= max_queue_size)
                queue_.pop();
            queue_.push(value);
        }
        condition_empty_.notify_one();
        result = true;
        return result;
    }

    // return false if queue is shutdown.
    // if queue is empty, wait for data and return the front.
    bool PopBlocking(QueueType* value){
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Note shutdown will also wake up the waiting thread, and return false after that.
            condition_empty_.wait(lock, [&]{ return (!queue_.empty() || shutdown); } );
            if(shutdown_)
                return false;
            QueueType _value = queue_.front();
            queue_.pop();
        }
        condition_full_.notify_one();
        return true;
    }

    // return false if queue is empty, and the value is not altered..
    bool PopNonBlocking(QueueType* value){
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (queue_.empty())
                return false;
            *value = queue_.front();
            queue_.pop();
        }
        condition_full_.notify_one();
        return true;
    }

};

#endif // ThreadSafeQueue.h