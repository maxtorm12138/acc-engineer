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

bool shared_memory::driving() const
{
    return graphic_content_->status == acc_status::live && physics_content_->is_engine_running == 1;
}

std::tuple<page_file_physics, page_file_graphic, page_file_static> shared_memory::frame() const
{
    return {*physics_content_, *graphic_content_, *static_content_};
}
} // namespace acc_engineer::shared_memory