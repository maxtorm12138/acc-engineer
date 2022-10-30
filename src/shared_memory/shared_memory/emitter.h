#ifndef ACC_ENGINEER_SHARED_MEMORY_EMITTER_H
#define ACC_ENGINEER_SHARED_MEMORY_EMITTER_H

#include <QObject>
#include "defines.h"

namespace acc_engineer::shared_memory {

class emitter final : public QObject
{
    Q_OBJECT

public:
signals:
    void start_driving();
    void new_frame(frame frame);
    void stop_driving();
};
} // namespace acc_engineer::shared_memory

#endif
