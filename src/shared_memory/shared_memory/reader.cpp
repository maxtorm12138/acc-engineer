#include "reader.h"

namespace acc_engineer::shared_memory {
reader::reader()
    : physics_memory_(boost::interprocess::open_or_create, physics_page_file_path.data(), boost::interprocess::read_write, sizeof(page_file_physics))
    , physics_region_(physics_memory_, boost::interprocess::read_write, 0, sizeof(page_file_physics))
    , physics_content_(static_cast<page_file_physics *>(physics_region_.get_address()))
    , graphics_memory_(boost::interprocess::open_or_create, graphics_page_file_path.data(), boost::interprocess::read_write, sizeof(page_file_physics))
    , graphics_region_(graphics_memory_, boost::interprocess::read_write, 0, sizeof(page_file_graphic))
    , graphic_content_(static_cast<page_file_graphic *>(graphics_region_.get_address()))
    , static_memory_(boost::interprocess::open_or_create, static_page_file_path.data(), boost::interprocess::read_write, sizeof(page_file_static))
    , static_region_(static_memory_, boost::interprocess::read_write, 0, sizeof(page_file_static))
    , static_content_(static_cast<page_file_static *>(static_region_.get_address()))
    , tick_(0)
{}

frame reader::snapshot()
{
    auto physics_content = *physics_content_;
    auto graphic_content = *graphic_content_;
    auto static_content = *static_content_;

    frame frame{.tick = tick_++,
        .gas = physics_content.gas,
        .brake = physics_content.brake,
        .fuel = physics_content.fuel,
        .steer_angle = physics_content.steer_angle,
        .speed_kmh = physics_content.speed_kmh,
        .gear = physics_content.gear,
        .status = graphic_content.status,
        .abs = graphic_content.abs,
        .tc = graphic_content.tc,
		.tc2 = graphic_content.tc_cut,
        .abs_in_action = physics_content.abs,
        .tc_in_action = physics_content.tc,
        .engine_map = graphic_content.engine_map
    };

    return frame;
}
} // namespace acc_engineer::shared_memory