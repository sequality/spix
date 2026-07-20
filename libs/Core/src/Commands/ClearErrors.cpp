/***
 * Copyright (C) Falko Axmann. All rights reserved.
 * Licensed under the MIT license.
 * See LICENSE.txt file in the project root for full license information.
 ****/

#include "ClearErrors.h"


namespace spix {
namespace cmd {

ClearErrors::ClearErrors()
{
}

void ClearErrors::execute(CommandEnvironment& env)
{
    env.state().clearErrors();
}

} // namespace cmd
} // namespace spix
