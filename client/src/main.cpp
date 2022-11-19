#include <spdlog/spdlog.h>

#include "app.h"

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::trace);

    acc_engineer::app app(argc, argv);

    return app.exec();
}
