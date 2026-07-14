/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "Synchronize.h"

namespace spix {
namespace cmd {

Synchronize::Synchronize(std::promise<void> promise)
: m_promise(std::move(promise)) { }

void Synchronize::execute(CommandEnvironment& env)
{
    m_promise.set_value();
}

} // namespace cmd
} // namespace spix
