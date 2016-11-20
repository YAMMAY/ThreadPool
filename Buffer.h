#include <stack>
#include <mutex>
#include <condition_variable>

class Buffer
{
private:
	std::mutex BufferMutex_;
	std::stack<int> Buffer_;
	std::condition_variable get_condition;
	std::condition_variable put_condition;
	int maxSize_;

public:
	Buffer() = default;
	~Buffer() = default;
	int get();   // return the top of buffer stack;
	void put(int x); // push the value into stack;
};

int Buffer::get()
{
	int temp;
	{
		std::unique_lock<std::mutex> getMutex(BufferMutex_);
		get_condition.wait(getMutex, [&] {return !Buffer_.empty(); });
		temp = Buffer_.top();
		Buffer_.pop();		// remove the element
		std::cout << "get " << temp << std::endl;
	}
	put_condition.notify_one();
	return temp;
}

void Buffer::put(int x)
{
	{
		std::unique_lock<std::mutex> putMutex(BufferMutex_);
		put_condition.wait(putMutex, [&] {return Buffer_.size() != maxSize_; });
		Buffer_.push(x);
		std::cout << "put " << x<< std::endl;
	}
	get_condition.notify_one();

}