#include "shared_memory_emitter.h"

void acc_engineer::ui::shared_memory_emitter::consume(
    shared_memory::page_file_physics &physics_content, shared_memory::page_file_graphic &graphics_content, shared_memory::page_file_static &static_content)
{
    emit new_frame({.gas = physics_content.gas, .throttle = physics_content.brake});
}
