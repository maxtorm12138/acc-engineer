#ifndef ACC_ENGINEER_SHARED_MEMORY_SHARED_MEMORY_H
#define ACC_ENGINEER_SHARED_MEMORY_SHARED_MEMORY_H

#include <boost/noncopyable.hpp>
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "defines.h"

#include "proto/struct.pb.h"

namespace acc_engineer::shared_memory {
class reader : public boost::noncopyable
{
public:
    reader();

    [[nodiscard]] frame snapshot();

    [[nodiscard]] float mfd_fuel_to_add() const noexcept;

    [[nodiscard]] float fuel_tank_capacity() const noexcept;

    [[nodiscard]] structure::WheelInfo mfd_tyre_pressures() const noexcept;

    [[nodiscard]] int current_tyre_set() const noexcept;

    [[nodiscard]] int mfd_tyre_set() const noexcept;

    [[nodiscard]] bool driving() const noexcept;

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
    uint64_t tick_;
};

} // namespace acc_engineer::shared_memory

#endif
