// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <ecst/config.hpp>
#include <ecst/aliases.hpp>
#include <ecst/settings.hpp>
#include <ecst/context/entity.hpp>
#include "./container.hpp"
#include "./data.hpp"

ECST_CONTEXT_STORAGE_ENTITY_NAMESPACE
{
    namespace impl
    {
        template <typename TSettings>
        auto dispatch_tw(TSettings s) noexcept
        {
            return static_if(settings::has_fixed_entity_storage<TSettings>)
                .then([](auto xs)
                    {
                        return mp::type_v<                               // .
                            impl::fixed_entity_storage<                  // .
                                context::entity::dispatch<decltype(xs)>, // .
                                settings::fixed_capacity(xs)>            // .
                            >;
                    })
                .else_([](auto xs)
                    {
                        return mp::type_v<                              // .
                            impl::dynamic_entity_storage<TSettings,     // .
                                context::entity::dispatch<decltype(xs)> // .
                                >                                       // .
                            >;
                    })(s);
        }
    }

    /// @brief Dispatches between a fixed or a dynamic entity storage, depending
    /// on the user settings.
    template <typename TSettings>
    using dispatch = mp::unwrap<decltype(impl::dispatch_tw(TSettings{}))>;
}
ECST_CONTEXT_STORAGE_ENTITY_NAMESPACE_END