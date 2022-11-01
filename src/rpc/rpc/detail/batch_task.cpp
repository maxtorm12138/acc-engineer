#include "batch_task.h"
namespace acc_engineer::rpc::detail {
void batch_task_base::cancel(net::cancellation_type type)
{
    for (auto &weak_signal : cancellation_signals_)
    {
        if (auto signal = weak_signal.lock(); signal != nullptr)
        {
            signal->emit(type);
        }
    }
}
} // namespace acc_engineer::rpc::detail