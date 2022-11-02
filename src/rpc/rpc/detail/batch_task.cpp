#include "batch_task.h"
namespace acc_engineer::rpc::detail {
void batch_task_base::cancel(net::cancellation_type type)
{
    for (auto iter = cancellation_signals_.begin(); iter != cancellation_signals_.end();)
    {
        if (auto signal = iter->lock(); signal != nullptr)
        {
            signal->emit(type);
            ++iter;
        }
        else
        {
            iter = cancellation_signals_.erase(iter);
        }
    }
}
} // namespace acc_engineer::rpc::detail