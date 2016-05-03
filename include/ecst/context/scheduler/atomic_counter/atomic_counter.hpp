// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <ecst/config.hpp>
#include <ecst/aliases.hpp>
#include <ecst/utils.hpp>
#include <ecst/thread_pool.hpp>
#include <ecst/mp.hpp>
#include <ecst/signature_list.hpp>
#include <ecst/settings.hpp>
#include <ecst/context/scheduler/atomic_counter/task_group.hpp>
#include <ecst/context/scheduler/atomic_counter/utils.hpp>

ECST_SCHEDULER_NAMESPACE
{
    namespace impl
    {
        /// @brief Namespace alias for the `atomic_counter` scheduler.
        namespace sac = ecst::scheduler::atomic_counter;
    }

    /// @brief System execution scheduler based on a runtime atomic counter.
    template <typename TSettings>
    class s_atomic_counter
    {
    private:
        static constexpr auto ssl =
            settings::system_signature_list(TSettings{});

        impl::sac::impl::task_group_type<decltype(ssl)> _task_group;

        /// @brief Resets all dependency atomic counters.
        void reset() noexcept
        {
            impl::sac::reset_task_group_from_ssl(ssl, _task_group);
        }

        template <typename TContext, typename TBlocker, typename TF>
        void start_execution(TContext& ctx, TBlocker& b, TF&& f)
        {
            signature_list::system::for_indepedent_ids(ssl, // .
                [this, &ctx, &b, &f](auto s_id)
                {
                    ctx.post_in_thread_pool([this, s_id, &ctx, &b, &f]
                        {
                            this->_task_group.start_from_task_id(
                                b, s_id, ctx, f);
                        });
                });
        }

    public:
        s_atomic_counter() = default;
        ECST_DEFINE_DEFAULT_MOVE_ONLY_OPERATIONS(s_atomic_counter);

        template <typename TSystemStorage>
        void initialize(TSystemStorage&)
        {
            reset();
        }

        template <typename TContext, typename TF>
        void execute(TContext& ctx, TF&& f)
        {
            reset();

            // Aggregates the required synchronization objects.
            counter_blocker b{mp::list::size(ssl)};

            // Starts every independent task and waits until the remaining tasks
            // counter reaches zero. We forward `f` into the lambda here, then
            // refer to it everywhere else.
            execute_and_wait_until_counter_zero(b,
                [ this, &ctx, &b, f = FWD(f) ]
                {
                    this->start_execution(ctx, b, f);
                });
        }
    };
}
ECST_SCHEDULER_NAMESPACE_END