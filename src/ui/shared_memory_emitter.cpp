#include "shared_memory_emitter.h"

void acc_engineer::ui::shared_memory_emitter::consume(
    shared_memory::page_file_physics &physics_content, shared_memory::page_file_graphic &graphics_content, shared_memory::page_file_static &static_content)
{
    const frame frame{.percent_gas = static_cast<int>(std::floor(physics_content.gas * 100)),
        .percent_throttle = static_cast<int>(std::floor(physics_content.brake * 100)),
        .gear = physics_content.gear,
        .speed = static_cast<int>(physics_content.speed_kmh)};
    emit new_frame(frame);
}
