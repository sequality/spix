/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "WaitForItem.h"
#include <Spix/Scene/Scene.h>

namespace spix {
namespace cmd {

WaitForItem::WaitForItem(ItemPath path, std::chrono::milliseconds maxWaitTime, std::promise<bool> promise)
: m_maxWaitTime(std::move(maxWaitTime))
, m_path(std::move(path))
, m_promise(std::move(promise))
{
}

void WaitForItem::execute(CommandEnvironment& env)
{
    m_promise.set_value(m_itemFound);
}

bool WaitForItem::canExecuteNow(CommandEnvironment& env)
{
    if (!m_timerInitialized) {
        m_timerInitialized = true;
        m_startTime = std::chrono::steady_clock::now();
        return false;
    }

    // Done as soon as the item both exists and is visible.
    auto item = env.scene().itemAtPath(m_path);
    if (item && item->visible()) {
        m_itemFound = true;
        return true;
    }

    // Otherwise keep waiting (item missing, or present but not yet visible)
    // until the timeout elapses, then report failure.
    auto timeSinceStart = std::chrono::steady_clock::now() - m_startTime;
    if (timeSinceStart >= m_maxWaitTime) {
        m_itemFound = false;
        return true;
    }
    return false;
}

} // namespace cmd
} // namespace spix
