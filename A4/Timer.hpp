#pragma once

#include <iostream>
#include <chrono>

// Utility class for benchmarking (Courtesy of The Cherno)
class Timer {
public:
	Timer()
		: m_startTimePoint(std::chrono::high_resolution_clock::now())
	{}

	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		const auto endTimePoint = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint - m_startTimePoint).count();

		std::cout << std::endl << "Finished in:" << " " << ms << "ms" << " (";
			std::cout << double(ms) / 1000.0  << "s)" << std::endl;
	}

private:
	const std::chrono::high_resolution_clock::time_point m_startTimePoint;
};