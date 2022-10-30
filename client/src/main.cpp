#include <iostream>
#include <thread>

#include <QApplication>
#include <QString>

#include <spdlog/spdlog.h>

#include "common/defines.h"
#include "shared_memory/reader.h"
#include "shared_memory/emitter.h"

#include "app.h"

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

int shared_memory_main(std::shared_ptr<acc_engineer::shared_memory::reader> memory, std::shared_ptr<acc_engineer::shared_memory::emitter> emitter)
{
    try
    {
        using namespace std::chrono_literals;
        std::chrono::milliseconds sleep_ms(10);
        while (running)
        {
            std::this_thread::sleep_for(sleep_ms);
            auto frame = memory->snapshot();

            if (frame.status != acc_engineer::shared_memory::acc_status::live)
            {
                sleep_ms = std::chrono::milliseconds(acc_engineer::not_driving_sleep_ms);
                SPDLOG_TRACE("user not driving, sleep {}ms", acc_engineer::not_driving_sleep_ms);
                continue;
            }

            sleep_ms = std::chrono::milliseconds(acc_engineer::driving_sleep_ms);
            SPDLOG_TRACE("user driving, sleep {}ms", acc_engineer::driving_sleep_ms);

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
    QObject::connect(&app, &QGuiApplication::lastWindowClosed, &app, &QApplication::quit);

    acc_engineer::app my_app(&app);
    my_app.start();

    return app.exec();
}
