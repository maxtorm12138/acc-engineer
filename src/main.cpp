#include <iostream>
#include <thread>

#include <QApplication>
#include <QString>

#include <spdlog/spdlog.h>

#include "shared_memory/shared_memory.h"
#include "ui/shared_memory_emitter.h"
#include "ui/main_window.h"
#include "ui/driver_input.h"
#include "ui/gas_chart.h"

std::atomic running{true};

template<size_t N>
void print_content(const std ::string_view name, wchar_t content[N])
{
    const auto transcode = QString::fromWCharArray(content, N);
    std::cout << name << ": " << transcode.toStdString() << "\n";
}

void print_content(const std::string_view name, float content)
{
    std::cout << name << ": " << content << "\n";
}

void print_content(const std::string_view name, int content)
{
    std::cout << name << ": " << content << "\n";
}

int net_main(int argc, char *argv[])
{
    using namespace std::chrono_literals;
    while (running)
    {
        std::this_thread::sleep_for(2s);
    }
    return 0;
}

int shared_memory_main(std::shared_ptr<acc_engineer::shared_memory::shared_memory> memory, std::shared_ptr<acc_engineer::ui::shared_memory_emitter> emitter)
{
    try
    {
        using namespace std::chrono_literals;
        std::chrono::milliseconds sleep_ms(10);
        while (running)
        {
            std::this_thread::sleep_for(sleep_ms);
            if (!memory->driving())
            {
                sleep_ms = 2s;
                SPDLOG_DEBUG("user not driving, sleep 2s");
                continue;
            }
            sleep_ms = 15ms;
            auto [physics_content, graphic_content, static_content] = memory->frame();
            emitter->consume(physics_content, graphic_content, static_content);
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
    auto shared_memory_emitter = std::make_shared<acc_engineer::ui::shared_memory_emitter>();

    auto main_window = new acc_engineer::ui::main_window;
    auto gas_chart = new acc_engineer::ui::gas_chart;
    auto driver_input = new acc_engineer::ui::driver_input(gas_chart);

    std::thread shared_memory_thread(shared_memory_main, shared_memory, shared_memory_emitter);
    QObject::connect(shared_memory_emitter.get(), &acc_engineer::ui::shared_memory_emitter::new_frame, driver_input, &acc_engineer::ui::driver_input::handle_new_frame);
    QObject::connect(shared_memory_emitter.get(), &acc_engineer::ui::shared_memory_emitter::new_frame, gas_chart, &acc_engineer::ui::gas_chart::handle_new_frame);
    // std::thread net_main_thread(net_main, argc, argv);

    main_window->show();
    driver_input->show();

    const int ret = app.exec(); // NOLINT(readability-static-accessed-through-instance)
    running = false;
    // net_main_thread.join();

    shared_memory_thread.join();
    return ret;
}
