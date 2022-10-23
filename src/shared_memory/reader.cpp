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
{}

page_file_physics reader::read_physics() const
{
    return *physics_content_;
}

page_file_graphic reader::read_graphic() const
{
    return *graphic_content_;
}

page_file_static reader::read_static() const
{
    return *static_content_;
}

std::tuple<page_file_physics, page_file_graphic, page_file_static> reader::read() const
{
    return {read_physics(), read_graphic(), read_static()};
}
} // namespace acc_engineer::shared_memory