#include <iostream>
#include <thread>
#include <chrono>
#include "Buffer.h"

using namespace std::chrono_literals;

void produce(std::shared_ptr<Buffer> pBuffer, int n)
{
	for (int i = 0; i < n; ++i) {
		//std::this_thread::sleep_for(2s);
		pBuffer->put(i);
		std::cout << "put " << i << std::endl;
	}
}

void consume(std::shared_ptr<Buffer> pBuffer, int n)
{
	int temp;
	for (int i = 0; i < n; ++i)
	{
		temp = pBuffer->get();
		std::cout << "get:" << temp << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::shared_ptr<Buffer> pBuffer(new Buffer) ;
	
	std::thread producer(produce, pBuffer,  100);
	std::thread consume(consume, pBuffer, 50);


	producer.join();
	consume.join();

	std::this_thread::sleep_for(2s);
	return 0;
}
