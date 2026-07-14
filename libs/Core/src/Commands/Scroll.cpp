/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "Scroll.h"

#include <Spix/Scene/Scene.h>

namespace spix {
namespace cmd {

Scroll::Scroll(ItemPosition path, int angle)
: m_position(std::move(path))
, m_angle(angle)
{
}

void Scroll::execute(CommandEnvironment& env)
{
    auto path = m_position.itemPath();
    auto item = env.scene().itemAtPath(path);

    if (!item) {
        env.state().reportError("Scroll: Item not found: " + path.string());
        return;
    }

    env.scene().events().scroll(item.get(), m_angle);
}

} // namespace cmd
} // namespace spix
