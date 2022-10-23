#include <iostream>
#include <thread>

#include <QApplication>
#include <QString>

#include <spdlog/spdlog.h>

#include "shared_memory/reader.h"

#include "ui/main_window.h"

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

void shared_memory_main()
{
    try
    {
        using namespace std::chrono_literals;
        acc_engineer::shared_memory::reader reader;
        std::chrono::milliseconds sleep_ms(10);
        while (running)
        {
            std::this_thread::sleep_for(sleep_ms);
            auto [physics_content, graphic_content, static_content] = reader.read();
            if (graphic_content.session != acc_engineer::shared_memory::acc_session_type::unknown)
            {
                sleep_ms = 10ms;
            }
            else
            {
                spdlog::debug("user not driving sleep 2s");
                sleep_ms = 2s;
            }
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "shared memory exception: " << ex.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::trace);
    QApplication app(argc, argv);
    std::thread net_main_thread(net_main, argc, argv);
    std::thread shared_memory_thread(shared_memory_main);

    acc_engineer::ui::main_window main_window;
    main_window.show();

    const int ret = app.exec(); // NOLINT(readability-static-accessed-through-instance)
    running = false;
    net_main_thread.join();
    shared_memory_thread.join();
    return ret;
}
