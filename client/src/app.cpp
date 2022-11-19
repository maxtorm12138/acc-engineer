#include "app.h"

// spdlog
#include <spdlog/spdlog.h>

// boost
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>

// shared memory
#include <shared_memory/reader.h>

// windows
#include <timeapi.h>

namespace acc_engineer {

app::app(int argc, char *argv[])
    : QApplication(argc, argv)
    , running_(true)
    , io_context_(1)
{
    connect(this, &QGuiApplication::lastWindowClosed, this, &app::quit);

    net_thread_ = std::jthread([this]() { net_main(); });
    shared_memory_thread_ = std::jthread([this]() { shared_memory_main(); });
}

void app::net_main()
{
    SPDLOG_TRACE("net_main started");
    BOOST_SCOPE_EXIT_ALL(&)
    {
        SPDLOG_TRACE("net_main ended");
    };

    guard_.emplace(make_work_guard(io_context_));
    io_context_.run();
}

void app::shared_memory_main()
{
    using namespace std::chrono_literals;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;

    SPDLOG_TRACE("shared_memory_main started");
    BOOST_SCOPE_EXIT_ALL(&)
    {
        SPDLOG_TRACE("shared_memory_main ended");
    };

    timeBeginPeriod(1);
    BOOST_SCOPE_EXIT_ALL(&)
    {
        timeEndPeriod(1);
    };

    shared_memory::reader reader;

    nanoseconds current_timeslice = 10ms;
    nanoseconds next_timeslice = 10ms;

    auto sample_start_time = std::chrono::high_resolution_clock::now();
    while (running_)
    {
        BOOST_SCOPE_EXIT_ALL(&)
        {
            const auto logic_cost = std::chrono::high_resolution_clock::now() - sample_start_time;

            if (const auto expected_sleep_cost = current_timeslice - logic_cost - 1ms; expected_sleep_cost > 0us)
            {
                const DWORD sleep_ms = duration_cast<milliseconds>(expected_sleep_cost).count();
                Sleep(sleep_ms);
            }

            auto total_cost = std::chrono::high_resolution_clock::now() - sample_start_time;

            if (total_cost < current_timeslice)
            {
                const auto compensate = current_timeslice - total_cost;
                const auto start_compensate_time = std::chrono::high_resolution_clock::now();

                while (std::chrono::high_resolution_clock::now() - start_compensate_time < compensate) {}
            }

            total_cost = std::chrono::high_resolution_clock::now() - sample_start_time;

            current_timeslice = next_timeslice + (current_timeslice - total_cost);

            sample_start_time = std::chrono::high_resolution_clock::now();

            SPDLOG_DEBUG("total_cost: {}ns next_timeslice: {}ns", total_cost.count(), current_timeslice.count());
        };

        if (!reader.driving())
        {
            next_timeslice = 1s;
            continue;
        }

        next_timeslice = 10ms;
    }
}

void app::quit()
{
    SPDLOG_DEBUG("application quit");
    running_ = false;
    guard_->reset();
    QApplication::quit();
}

} // namespace acc_engineer