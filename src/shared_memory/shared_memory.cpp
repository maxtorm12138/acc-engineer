#include "shared_memory.h"

namespace acc_engineer::shared_memory {
shared_memory::shared_memory()
    : physics_memory_(boost::interprocess::open_or_create, physics_page_file_path.data(), boost::interprocess::read_write, sizeof(page_file_physics))
    , physics_region_(physics_memory_, boost::interprocess::read_write, 0, sizeof(page_file_physics))
    , physics_content_(static_cast<page_file_physics *>(physics_region_.get_address()))
    , graphics_memory_(boost::interprocess::open_or_create, graphics_page_file_path.data(), boost::interprocess::read_write, sizeof(page_file_physics))
    , graphics_region_(graphics_memory_, boost::interprocess::read_write, 0, sizeof(page_file_graphic))
    , graphic_content_(static_cast<page_file_graphic *>(graphics_region_.get_address()))
    , static_memory_(boost::interprocess::open_or_create, static_page_file_path.data(), boost::interprocess::read_write, sizeof(page_file_static))
    , static_region_(static_memory_, boost::interprocess::read_write, 0, sizeof(page_file_static))
    , static_content_(static_cast<page_file_static *>(static_region_.get_address()))
{}

bool shared_memory::driving(const frame &frame)
{
    return frame.status == acc_status::live && frame.is_engine_running == 1;
}

frame shared_memory::snapshot() const
{
    auto physics_content = *physics_content_;
    auto graphic_content = *graphic_content_;
    auto static_content = *static_content_;

    frame frame{.gas = physics_content.gas,
        .brake = physics_content.brake,
        .fuel = physics_content.fuel,
        .steer_angle = physics_content.steer_angle,
        .speed_kmh = physics_content.speed_kmh,
        .gear = physics_content.gear,
        .status = graphic_content.status,
        .is_engine_running = physics_content.is_engine_running};

    return frame;
}
} // namespace acc_engineer::shared_memory