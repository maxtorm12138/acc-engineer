#include "shared_memory_emitter.h"

void acc_engineer::ui::shared_memory_emitter::consume(
    shared_memory::page_file_physics &physics_content, shared_memory::page_file_graphic &graphics_content, shared_memory::page_file_static &static_content)
{

    const frame frame{.percent_gas = static_cast<int>(std::floor(physics_content.gas * 100)),
        .percent_throttle = static_cast<int>(std::floor(physics_content.brake * 100)),
        .gear = physics_content.gear,
        .gear_display =  format_gear(physics_content.gear),
        .speed = static_cast<int>(physics_content.speed_kmh)};
    emit new_frame(frame);
}

QString acc_engineer::ui::shared_memory_emitter::format_gear(int gear)
{
	switch (gear)
	{
	case 0:
        return tr("R");
	case 1:
        return tr("N");
	default :
		return QString("%1").arg(gear - 1);
	}
}
