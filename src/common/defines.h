#ifndef ACC_ENGINEER_COMMON_DEFINES_H
#define ACC_ENGINEER_COMMON_DEFINES_H

namespace acc_engineer {
constexpr uint64_t samples_per_second = 100;
constexpr uint64_t not_driving_sleep_ms = 2000;
constexpr uint64_t driving_sleep_ms = 1000 / samples_per_second;
constexpr uint64_t driver_input_history_seconds = 10;
constexpr uint64_t max_speed = 300;
constexpr uint64_t driver_input_dots = driver_input_history_seconds * samples_per_second;
} // namespace acc_engineer

#endif // !ACC_ENGINEER_COMMON_DEFINES_H
