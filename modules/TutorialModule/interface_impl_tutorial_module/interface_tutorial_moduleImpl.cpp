// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest

#include "interface_tutorial_moduleImpl.hpp"

namespace module {
namespace interface_impl_tutorial_module {

void interface_tutorial_moduleImpl::init() {
}

void interface_tutorial_moduleImpl::ready() {
}

std::string interface_tutorial_moduleImpl::handle_command_tutorial(std::string& payload) {
    // your code for cmd command_tutorial goes here
    return "everest";
}

} // namespace interface_impl_tutorial_module
} // namespace module
