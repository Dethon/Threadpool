#include "../ThreadPool.hpp"
#include <iostream>
#include <string>

int task(int a, float c)
{
	std::cout << std::to_string(a) + " --- " + std::to_string(c) + "\n";
	return 0;
}

int main()
{
	auto pool = ThreadPool{ std::thread::hardware_concurrency() };
	auto futures = std::vector<std::future<int>>{};
	for (auto i = 0u; i < 20u; i++)
	{
		futures.push_back(pool.enqueue(&task, i, i/100.0f));
	}
	for (const auto &fut : futures)
	{
		fut.wait();
	}

	return 0;
}
