#include <iostream>
#include <thread>
#include "Buffer.h"

void produce(std::shared_ptr<Buffer> pBuffer, int n)
{
	for (int i = 0; i < n; ++i) {
		pBuffer->put(i);
		//std::cout << "put " << i << std::endl;
	}
}

void consume(std::shared_ptr<Buffer> pBuffer, int n)
{
	int temp;
	for (int i = 0; i < n; ++i)
	{
		temp = pBuffer->get();
		//std::cout << "get:" << temp << std::endl;
	}
}

int main(int argc, char* argv[])
{
	//Buffer global_buffer;
	std::shared_ptr<Buffer> pBuffer(new Buffer) ;
	
	std::thread producer(produce, pBuffer,  100);
	std::thread consume(consume, pBuffer, 50);


	producer.join();
	consume.join();

	return 0;
}
