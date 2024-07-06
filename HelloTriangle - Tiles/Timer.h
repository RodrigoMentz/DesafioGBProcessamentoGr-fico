
#include <chrono>
#include <thread>
#include <ctime>

class Timer
{
public:
    Timer() {}

    void start() { begin = std::chrono::system_clock::now(); }
    void finish() { end = std::chrono::system_clock::now(); }
    void wait(float FPS)
    {
        std::chrono::time_point<std::chrono::system_clock> now, last;
        std::chrono::duration<double> dt;
        last = std::chrono::system_clock::now();
        do {
			now = std::chrono::system_clock::now();
			dt =  end - now;
		} while (dt.count()*1000 < 1.0 / FPS);
    }

    double getElapsedTimeMs()
    {
        std::chrono::duration<double> elapsed_seconds = end - begin;
        return elapsed_seconds.count() * 1000;
    }

    double getElapsedTime()
    {
        std::chrono::duration<double> elapsed_seconds = end - begin;
        return elapsed_seconds.count();
    }
    double calcWaitingTime(int fps, double elapsedTime)
    {
        double wt = 1000 / (double)fps - elapsedTime;
        return wt;
    }

protected:
    // Using time point and system_clock
    std::chrono::time_point<std::chrono::system_clock> begin, end;
};
