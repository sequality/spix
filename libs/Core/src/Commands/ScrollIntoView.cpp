/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "ScrollIntoView.h"

#include <Spix/Scene/Scene.h>

namespace spix {
namespace cmd {

ScrollIntoView::ScrollIntoView(ItemPath path)
: m_path(std::move(path))
{
}

void ScrollIntoView::execute(CommandEnvironment& env)
{
    auto item = env.scene().itemAtPath(m_path);

    if (!item) {
        env.state().reportError("ScrollIntoView: Item not found: " + m_path.string());
        return;
    }

    if (!item->ensureVisibleInViewport()) {
        env.state().reportError(
            "ScrollIntoView: Could not bring item into view (no scrollable ancestor or still off-screen): "
            + m_path.string());
    }
}

} // namespace cmd
} // namespace spix
