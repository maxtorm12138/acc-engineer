#ifndef ACC_ENGINEER_SHARED_MEMORY_DEFINES_H
#define ACC_ENGINEER_SHARED_MEMORY_DEFINES_H

#include <cstdint>
#include <string_view>

#include "car_spec.h"

namespace acc_engineer::shared_memory {

constexpr std::string_view physics_page_file_path = "Local\\acpmf_physics";
constexpr std::string_view graphics_page_file_path = "Local\\acpmf_graphics";
constexpr std::string_view static_page_file_path = "Local\\acpmf_static";

enum class driver_status
{
    not_driving = 0,
    driving = 1
};

enum class penalty_shortcut : int
{
    none = 0,
    drive_through_cutting,
    stop_and_go_10_cutting,
    stop_and_go_20_cutting,
    stop_and_go_30_cutting,
    disqualified_cutting,
    remove_best_lap_time_cutting,

    drive_through_pit_speeding,
    stop_and_go_10_pit_speeding,
    stop_and_go_20_pit_speeding,
    stop_and_go_30_pit_speeding,
    disqualified_pit_speeding,
    remove_best_lap_time_pit_speeding,

    disqualified_ignored_mandatory_pit,

    post_race_time,
    disqualified_trolling,
    disqualified_pit_entry,
    disqualified_pit_exit,
    disqualified_wrong_way,

    drive_through_ignored_driver_stint,
    disqualified_ignored_driver_stint,

    disqualified_exceeded_driver_stint_limit,
};

enum class acc_status : int
{
    off = 0,
    replay = 1,
    live = 2,
    pause = 3,
};

enum class acc_session_type : int
{
    unknown = -1,
    practice = 0,
    qualify = 1,
    race = 2,
    hotlap = 3,
    time_attack = 4,
    drift = 5,
    drag = 6,
    hotstint = 7,
    hotlapsuperpole = 8
};

enum class acc_flag_type : int
{
    no_flag = 0,
    blue_flag = 1,
    yellow_flag = 2,
    black_flag = 3,
    white_flag = 4,
    checkered_flag = 5,
    penalty_flag = 6,
};

enum class acc_track_grip_status : int
{
    green = 0,
    fast = 1,
    optimum = 2,
    greasy = 3,
    damp = 4,
    wet = 5,
    flooded = 6,
};

enum class acc_rain_intensity : int
{
    no_rain = 0,
    drizzle = 1,
    light_rain = 2,
    medium_rain = 3,
    heavy_rain = 4,
    thunderstorm = 5,
};

#pragma pack(push)
#pragma pack(4)

struct frame
{
    uint64_t tick;
    float gas;
    float brake;
    float fuel;
    float steer_angle;
    float speed_kmh;
    int gear;
    acc_status status;
    int abs;
    int tc;
    int tc2;
    float abs_in_action;
    float tc_in_action;
    int engine_map;
};

struct page_file_physics
{
    int packet_id = 0;
    float gas = 0;
    float brake = 0;
    float fuel = 0;
    int gear = 0;
    int rpms = 0;
    float steer_angle = 0;
    float speed_kmh = 0;
    float velocity[3]{};
    float acc_g[3]{};
    float wheel_slip[4]{};
    float unused_wheel_load[4]{};
    float wheels_pressure[4]{};
    float wheel_angular_speed[4]{};
    float unused_tyre_wear[4]{};
    float unused_tyre_dirty_level[4]{};
    float tyre_core_temperature[4]{};
    float unused_camber_rad[4]{};
    float suspension_travel[4]{};
    float unused_drs = 0;
    float tc = 0;
    float heading = 0;
    float pitch = 0;
    float roll = 0;
    float unused_cg_height{};
    float car_damage[5]{};
    int unused_number_of_tyres_out = 0;
    int pit_limiter_on = 0;
    float abs = 0;
    float unused_kers_charge = 0;
    float unused_kers_input = 0;
    int auto_shifter_on = 0;
    float unused_ride_height[2]{};
    float turbo_boost = 0;
    float unused_ballast = 0;
    float unused_air_density = 0;
    float air_temp = 0;
    float road_temp = 0;
    float local_angular_vel[3]{};
    float final_ff = 0;
    float unused_performance_meter = 0;

    int unused_engine_brake = 0;
    int unused_ers_recovery_level = 0;
    int unused_ers_power_level = 0;
    int unused_ers_heat_charging = 0;
    int unused_ers_is_charging = 0;
    float unused_kers_current_kj = 0;

    int unused_drs_available = 0;
    int unused_drs_enabled = 0;

    float brake_temp[4]{};
    float clutch = 0;

    float unused_tyre_temp_i[4]{};
    float unused_tyre_temp_m[4]{};
    float unused_tyre_temp_o[4]{};

    int is_ai_controlled{};

    float tyre_contact_point[4][3]{};
    float tyre_contact_normal[4][3]{};
    float tyre_contact_heading[4][3]{};

    float brake_bias = 0;

    float local_velocity[3]{};

    int unused_p2p_activations = 0;
    int unused_p2p_status = 0;

    int unused_current_max_rpm = 0;

    float unused_mz[4]{};
    float unused_fx[4]{};
    float unused_fy[4]{};
    float slip_ratio[4]{};
    float slip_angle[4]{};

    int unused_tc_in_action = 0;
    int unused_abs_in_action = 0;
    float unused_suspension_damage[4]{};
    float unused_tyre_temp[4]{};
    float water_temp = 0;
    float brake_pressure[4]{};
    int front_brake_compound = 0;
    int rear_brake_compound = 0;
    float pad_life[4]{};
    float disc_life[4]{};
    int ignition_on = 0;
    int starter_engine_on = 0;
    int is_engine_running = 0;
    float kerb_vibration = 0;
    float slip_vibrations = 0;
    float g_vibrations = 0;
    float abs_vibrations = 0;
};

struct page_file_graphic
{
    int packet_id = 0;
    acc_status status = acc_status::off;
    acc_session_type session = acc_session_type::practice;
    wchar_t current_time[15]{};
    wchar_t last_time[15]{};
    wchar_t best_time[15]{};
    wchar_t split[15]{};
    int completed_laps = 0;
    int position = 0;
    int i_current_time = 0;
    int i_last_time = 0;
    int i_best_time = 0;
    float session_time_left = 0;
    float distance_traveled = 0;
    int is_in_pit = 0;
    int current_sector_index = 0;
    int last_sector_time = 0;
    int number_of_laps = 0;
    wchar_t tyre_compound[33]{};
    float unused_replay_time_multiplier = 0;
    float normalized_car_position = 0;

    int active_cars = 0;
    float car_coordinates[60][3]{};
    int car_id[60]{};
    int player_car_id = 0;
    float penalty_time = 0;
    acc_flag_type flag = acc_flag_type::no_flag;
    penalty_shortcut penalty = penalty_shortcut::none;
    int ideal_line_on = 0;
    int is_in_pit_lane = 0;

    float surface_grip = 0;
    int mandatory_pit_done = 0;

    float wind_speed = 0;
    float wind_direction = 0;

    int is_setup_menu_visible = 0;

    int main_display_index = 0;
    int secondary_display_index = 0;
    int tc = 0;
    int tc_cut = 0;
    int engine_map = 0;
    int abs = 0;
    int fuel_x_lap = 0;
    int rain_lights = 0;
    int flashing_lights = 0;
    int lights_stage = 0;
    float exhaust_temperature = 0.0f;
    int wiper_lv = 0;
    int driver_stint_total_time_left = 0;
    int driver_stint_time_left = 0;
    int rain_tyres = 0;
    int session_index = 0;
    float used_fuel = 0;
    wchar_t delta_lap_time[15]{};
    int i_delta_lap_time = 0;
    wchar_t estimated_lap_time[15]{};
    int i_estimated_lap_time{};
    int is_delta_positive{};
    int i_split{};
    int is_valid_lap{};
    float fuel_estimated_laps = 0;
    wchar_t track_status[33]{};
    int missing_mandatory_pits{};
    float clock{};
    int direction_lights_left{};
    int direction_lights_right{};
    int global_yellow{};
    int global_yellow1{};
    int global_yellow2{};
    int global_yellow3{};
    int global_white{};
    int global_green{};
    int global_chequered{};
    int global_red{};
    int mfd_tyre_set{};
    float mfd_fuel_to_add{};
    float mfd_tyre_pressure_lf{};
    float mfd_tyre_pressure_rf{};
    float mfd_tyre_pressure_lr{};
    float mfd_tyre_pressure_rr{};
    acc_track_grip_status track_grip_status = acc_track_grip_status::optimum;
    acc_rain_intensity rain_intensity = acc_rain_intensity::no_rain;
    acc_rain_intensity rain_intensity_in10_min = acc_rain_intensity::no_rain;
    acc_rain_intensity rain_intensity_in30_min = acc_rain_intensity::no_rain;
    int current_tyre_set{};
    int strategy_tyre_set{};
    int gap_ahead{};
    int gap_behind{};
};

struct page_file_static
{
    wchar_t sm_version[15]{};
    wchar_t ac_version[15]{};

    // session static info
    int number_of_sessions = 0;
    int num_cars = 0;
    wchar_t car_model[33]{};
    wchar_t track[33]{};
    wchar_t player_name[33]{};
    wchar_t player_surname[33]{};
    wchar_t player_nick[33]{};
    int sector_count = 0;

    // car static info
    float unused_max_torque = 0;
    float unused_max_power = 0;
    int max_rpm = 0;
    float max_fuel = 0;
    float unused_suspension_max_travel[4]{};
    float unused_tyre_radius[4]{};
    float unused_max_turbo_boost = 0;

    float deprecated_1 = -273;
    float deprecated_2 = -273;

    int penalties_enabled = 0;

    float aid_fuel_rate = 0;
    float aid_tire_rate = 0;
    float aid_mechanical_damage = 0;
    int aid_allow_tyre_blankets = 0;
    float aid_stability = 0;
    int aid_auto_clutch = 0;
    int aid_auto_blip = 0;

    int unused_has_drs = 0;
    int unused_has_ers = 0;
    int unused_has_kers = 0;
    float unused_kers_max_j = 0;
    int unused_engine_brake_settings_count = 0;
    int unused_ers_power_controller_count = 0;
    float unused_track_spline_length = 0;
    wchar_t unused_track_configuration[33]{};
    float unused_ers_max_j = 0;

    int unused_is_timed_race = 0;
    int unused_has_extra_lap = 0;

    wchar_t unused_car_skin[33]{};
    int unused_reversed_grid_positions = 0;
    int pit_window_start = 0;
    int pit_window_end = 0;
    int is_online = 0;
};

#pragma pack(pop)

} // namespace acc_engineer::shared_memory
#endif
