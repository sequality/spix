/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "Screenshot.h"

#include <Spix/Scene/Scene.h>

namespace spix {
namespace cmd {

Screenshot::Screenshot(ItemPath targetItemPath, std::string filePath)
: m_itemPath {std::move(targetItemPath)}
, m_filePath {std::move(filePath)}
{
}

void Screenshot::execute(CommandEnvironment& env)
{
    auto item = env.scene().itemAtPath(m_itemPath);
    if (!item) {
        env.state().reportError("Screenshot: Item not found: " + m_itemPath.string());
        return;
    }
    if (!item->visibleOnScreen()) {
        env.state().reportError(
            "Screenshot: Item not visible on screen (off-viewport, hidden, or zero-size): " + m_itemPath.string());
        return;
    }

    env.scene().takeScreenshot(m_itemPath, m_filePath);
}

} // namespace cmd
} // namespace spix
