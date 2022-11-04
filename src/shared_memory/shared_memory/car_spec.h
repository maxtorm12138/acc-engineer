#ifndef ACC_ENGINEER_SHARED_MEMORY_CAR_SPEC_H
#define ACC_ENGINEER_SHARED_MEMORY_CAR_SPEC_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/key.hpp>

namespace acc_engineer::shared_memory {

struct car_spec
{
    std::string_view kunos_id;
    std::string_view name;
    int max_rpm;
    int brake_bias_offset;
};

struct by_kunuos_id
{};

struct by_name
{};

// GT3 2018
constexpr car_spec amr_v12_vantage_gt3{};
constexpr car_spec audi_r8_lms{};
constexpr car_spec bentley_continental_gt3_2016{};
constexpr car_spec bentley_continental_gt3_2018{};
constexpr car_spec bmw_m6_gt3{};
constexpr car_spec jaguar_g3{};
constexpr car_spec ferrari_488_gt3{};
constexpr car_spec honda_nsx_gt3{};
constexpr car_spec lamborghini_gallardo_rex{};
constexpr car_spec lamborghini_huracan_gt3{};
constexpr car_spec lamborghini_huracan_st{};
constexpr car_spec lexus_rc_f_gt3{};
constexpr car_spec mclaren_650s_gt3{};
constexpr car_spec mercedes_amg_gt3{};
constexpr car_spec nissan_gt_r_gt3_2017{};
constexpr car_spec nissan_gt_r_gt3_2018{};
constexpr car_spec porsche_991_gt3_r{};
constexpr car_spec porsche_991ii_gt3_cup{};

// GT3 2019
constexpr car_spec amr_v8_vantage_gt3{};
constexpr car_spec audi_r8_lms_evo{};
constexpr car_spec honda_nsx_gt3_evo{};
constexpr car_spec lamborghini_huracan_gt3_evo{};
constexpr car_spec mclaren_720s_gt3{};
constexpr car_spec porsche_991ii_gt3_r{};

// GT4
constexpr car_spec alpine_a110_gt4{};
constexpr car_spec amr_v8_vantage_gt4{};
constexpr car_spec audi_r8_gt4{};
constexpr car_spec bmw_m4_gt4{};
constexpr car_spec chevrolet_camaro_gt4r{};
constexpr car_spec ginetta_g55_gt4{};
constexpr car_spec ktm_xbow_gt4{};
constexpr car_spec maserati_mc_gt4{};
constexpr car_spec mclaren_570s_gt4{};
constexpr car_spec mercedes_amg_gt4{};
constexpr car_spec porsche_718_cayman_gt4_mr{};

// GT3 2020
constexpr car_spec ferrari_488_gt3_evo{};
constexpr car_spec mercedes_amg_gt3_evo{};

// GT3 2021
constexpr car_spec bmw_m4_gt3{};

// Challengers Pack DLC 2022
constexpr car_spec audi_r8_lms_evo_ii{};
constexpr car_spec bmw_m2_cs_racing{};
constexpr car_spec ferrari_488_challenge_evo{};
constexpr car_spec lamborghini_huracan_st_evo2{};
constexpr car_spec porsche_992_gt3_cup{};

template<typename ByTag, typename Key>
const car_spec &find_spec(Key &find_key)
{
    using namespace boost::multi_index;
    // clang-format off
    using spec_container_type =
        multi_index_container<
			car_spec,
			indexed_by<
				hashed_unique<tag<by_kunuos_id>, key<&car_spec::kunos_id>>,
				hashed_unique<tag<by_name>, key<&car_spec::name>>
			>
	>;
    // clang-format on

    static spec_container_type specs = [] {
        spec_container_type container;
        container.emplace(amr_v12_vantage_gt3);
        container.emplace(audi_r8_lms);
        container.emplace(bentley_continental_gt3_2016);
        container.emplace(bentley_continental_gt3_2018);
        container.emplace(bmw_m6_gt3);
        container.emplace(jaguar_g3);
        container.emplace(ferrari_488_gt3);
        container.emplace(honda_nsx_gt3);
        container.emplace(lamborghini_gallardo_rex);
        container.emplace(lamborghini_huracan_gt3);
        container.emplace(lamborghini_huracan_st);
        container.emplace(lexus_rc_f_gt3);
        container.emplace(mclaren_650s_gt3);
        container.emplace(nissan_gt_r_gt3_2017);
        container.emplace(nissan_gt_r_gt3_2018);
        container.emplace(porsche_991_gt3_r);
        container.emplace(porsche_991ii_gt3_cup);
        container.emplace(amr_v8_vantage_gt3);
        container.emplace(audi_r8_lms_evo);
        container.emplace(honda_nsx_gt3_evo);
        container.emplace(lamborghini_huracan_gt3_evo);
        container.emplace(mclaren_720s_gt3);
        container.emplace(porsche_991ii_gt3_r);
        container.emplace(ferrari_488_gt3_evo);
        container.emplace(mercedes_amg_gt3_evo);
        container.emplace(bmw_m4_gt3);
        container.emplace(audi_r8_lms_evo_ii);
        container.emplace(bmw_m2_cs_racing);
        container.emplace(ferrari_488_challenge_evo);
        container.emplace(lamborghini_huracan_st_evo2);
        container.emplace(porsche_992_gt3_cup);
        return container;
    }();

    auto &view = specs.find<ByTag>();
    auto it_spec = view.find(find_key);
    return it_spec.value();
};

} // namespace acc_engineer::shared_memory

#endif
