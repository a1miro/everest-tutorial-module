// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "TutorialModule.hpp"

namespace module {

void TutorialModule::init() {
    invoke_init(*p_interface_impl_tutorial_module);
}

void TutorialModule::ready() {
    invoke_ready(*p_interface_impl_tutorial_module);
}

} // namespace module
