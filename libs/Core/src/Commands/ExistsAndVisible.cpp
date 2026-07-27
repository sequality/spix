/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "ExistsAndVisible.h"

#include <Spix/Scene/Scene.h>

namespace spix {
namespace cmd {

ExistsAndVisible::ExistsAndVisible(ItemPath path, std::promise<bool> promise)
: m_path(std::move(path))
, m_promise(std::move(promise))
{
}

void ExistsAndVisible::execute(CommandEnvironment& env)
{
    auto item = env.scene().itemAtPath(m_path);

    // Viewport-aware: an item that exists but is scrolled out of view, clipped
    // away, or zero-sized counts as not visible. For the raw visibility-property
    // value use getStringProperty(path, "visible").
    if (item) {
        m_promise.set_value(item->visibleOnScreen());
    } else {
        m_promise.set_value(false);
    }
}

} // namespace cmd
} // namespace spix
