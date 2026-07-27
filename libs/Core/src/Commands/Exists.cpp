/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "Exists.h"

#include <Spix/Scene/Scene.h>

namespace spix {
namespace cmd {

Exists::Exists(ItemPath path, std::promise<bool> promise)
: m_path(std::move(path))
, m_promise(std::move(promise))
{
}

void Exists::execute(CommandEnvironment& env)
{
    // Pure existence check: does the path resolve to an item? No visibility or
    // geometry is considered.
    auto item = env.scene().itemAtPath(m_path);
    m_promise.set_value(item != nullptr);
}

} // namespace cmd
} // namespace spix
