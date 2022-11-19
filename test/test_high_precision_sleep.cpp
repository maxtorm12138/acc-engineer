#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <chrono>
#include <timeapi.h>
namespace net = boost::asio;

void sleep_thread()
{
    using namespace std::chrono_literals;
    timeBeginPeriod(1);

    while (true)
    {
        auto start = std::chrono::high_resolution_clock::now();
        Sleep(5);
        auto end = std::chrono::high_resolution_clock::now();
        SPDLOG_INFO("elapsed {} us", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }
}

int main(int argc, char *argv[])
{
    using namespace std::chrono_literals;
    std::jthread thread(sleep_thread);
}
