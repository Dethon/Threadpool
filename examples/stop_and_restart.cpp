#include "../ThreadPool.hpp"
#include <iostream>
#include <string>
#include <chrono>

using namespace std::chrono_literals;
int task(int a)
{
	std::cout << std::to_string(a) + " thread start\n";
	std::this_thread::sleep_for(1s);
	std::cout << std::to_string(a) + " thread end\n";
	return 0;
}

int main()
{
	auto pool = ThreadPool{ std::thread::hardware_concurrency() };
	for (auto i = 0u; i < 20u; i++)
	{
		pool.enqueue(&task, i);
	}

	std::this_thread::sleep_for(500ms);
	pool.stop();
	std::this_thread::sleep_for(5s);
	pool.start(1u);
	std::this_thread::sleep_for(5s);
	pool.stop();
	std::this_thread::sleep_for(5s);
	pool.start(std::thread::hardware_concurrency());	
	
	// ThreadPool's destructor uses softStop, so all queued tasks are executed before exiting the program.
	return 0;
}
