#ifndef ACC_ENGINEER_SHARED_MEMORY_EMITTER_H
#define ACC_ENGINEER_SHARED_MEMORY_EMITTER_H

#include <QObject>
#include "shared_memory/defines.h"

namespace acc_engineer::ui {

struct frame
{
    int percent_gas;
    int percent_throttle;
    int gear;
    int speed;
};

class shared_memory_emitter final : public QObject
{
    Q_OBJECT
public:
    void consume(shared_memory::page_file_physics &physics_content, shared_memory::page_file_graphic &graphics_content, shared_memory::page_file_static &static_content);
signals:
    void new_frame(frame frame);
};
} // namespace acc_engineer::ui

#endif
