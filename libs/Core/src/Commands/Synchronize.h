/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#pragma once

#include <Spix/Commands/Command.h>
#include <Spix/Data/ItemPath.h>

#include <future>

namespace spix {
namespace cmd {

class Synchronize : public Command {
public:
    Synchronize(std::promise<void> promise);

    void execute(CommandEnvironment& env) override;

private:
    std::promise<void> m_promise;
};

} // namespace cmd
} // namespace spix
