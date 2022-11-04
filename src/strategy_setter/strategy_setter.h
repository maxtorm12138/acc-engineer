#ifndef ACC_ENGINEER_STRATEGY_SETTER_STRATEGY_SETTER_H
#define ACC_ENGINEER_STRATEGY_SETTER_STRATEGY_SETTER_H
#include <tuple>
namespace acc_engineer::strategy_setter {

template<typename Key>
std::tuple<Key, int> press_count(std::tuple<Key, Key> keys, float current, float target)
{
    int press = std::rint((target - current) * 10.f);
    if (press < 0)
    {
        return {std::get<0>(keys), -press};
    }
    else
    {
        return {std::get<1>(keys), -press};
    }
}

template<typename Key>
std::tuple<Key, int> press_count(std::tuple<Key, Key> keys, int current, int target)
{
    int press = target - current;
    if (press < 0)
    {
        return {std::get<0>(keys), -press};
    }
    else
    {
        return {std::get<1>(keys), -press};
    }
}

} // namespace acc_engineer::strategy_setter

#endif