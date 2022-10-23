#ifndef ACC_ENGINEER_SHARED_MEMORY_READER_H
#define ACC_ENGINEER_SHARED_MEMORY_READER_H

#include <boost/noncopyable.hpp>
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/managed_windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "defines.h"
namespace acc_engineer::shared_memory {
class reader : public boost::noncopyable
{
public:
    reader();

    [[nodiscard]] page_file_physics read_physics() const;
    [[nodiscard]] page_file_graphic read_graphic() const;
    [[nodiscard]] page_file_static read_static() const;

    [[nodiscard]] std::tuple<page_file_physics, page_file_graphic, page_file_static> read() const;

private:
    boost::interprocess::windows_shared_memory physics_memory_;
    boost::interprocess::mapped_region physics_region_;
    page_file_physics *physics_content_;
    boost::interprocess::windows_shared_memory graphics_memory_;
    boost::interprocess::mapped_region graphics_region_;
    page_file_graphic *graphic_content_;
    boost::interprocess::windows_shared_memory static_memory_;
    boost::interprocess::mapped_region static_region_;
    page_file_static *static_content_;
};

} // namespace acc_engineer::shared_memory

#endif
