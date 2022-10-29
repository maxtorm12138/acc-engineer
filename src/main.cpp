#include <iostream>
#include <thread>

#include <QApplication>
#include <QString>

#include <spdlog/spdlog.h>

#include "common/defines.h"
#include "shared_memory/shared_memory.h"
#include "shared_memory/shared_memory_emitter.h"
#include "ui/main_window.h"
#include "ui/driver_input.h"

std::atomic running{true};

int net_main(int argc, char *argv[])
{
    using namespace std::chrono_literals;
    while (running)
    {
        std::this_thread::sleep_for(2s);
    }
    return 0;
}

int shared_memory_main(std::shared_ptr<acc_engineer::shared_memory::shared_memory> memory, std::shared_ptr<acc_engineer::shared_memory::emitter> emitter)
{
    try
    {
        using namespace std::chrono_literals;
        std::chrono::milliseconds sleep_ms(10);
        while (running)
        {
            std::this_thread::sleep_for(sleep_ms);
            auto frame = memory->snapshot();

            if (!acc_engineer::shared_memory::shared_memory::driving(frame))
            {
                sleep_ms = std::chrono::milliseconds(acc_engineer::not_driving_sleep_ms);
                SPDLOG_DEBUG("user not driving, sleep {}ms", acc_engineer::not_driving_sleep_ms);
                continue;
            }

            sleep_ms = std::chrono::milliseconds(acc_engineer::driving_sleep_ms);
            SPDLOG_DEBUG("user driving, sleep {}ms", acc_engineer::driving_sleep_ms);

            emit emitter->new_frame(frame);
        }
    }
    catch (const std::exception &ex)
    {
        SPDLOG_ERROR("shared memory main exception: {}", ex.what());
    }
    return 0;
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::trace);
    QApplication app(argc, argv);

    auto shared_memory = std::make_shared<acc_engineer::shared_memory::shared_memory>();
    auto shared_memory_emitter = std::make_shared<acc_engineer::shared_memory::emitter>();

    auto main_window = new acc_engineer::ui::main_window;
    auto driver_input = new acc_engineer::ui::driver_input(main_window);

    std::thread shared_memory_thread(shared_memory_main, shared_memory, shared_memory_emitter);
    QObject::connect(shared_memory_emitter.get(), &acc_engineer::shared_memory::emitter::new_frame, driver_input, &acc_engineer::ui::driver_input::handle_new_frame);
    //  std::thread net_main_thread(net_main, argc, argv);

    main_window->show();
    driver_input->show();

    const int ret = app.exec(); // NOLINT(readability-static-accessed-through-instance)
    running = false;
    // net_main_thread.join();

    shared_memory_thread.join();
    return ret;
}
