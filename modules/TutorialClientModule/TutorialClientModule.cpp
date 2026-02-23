// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "TutorialClientModule.hpp"

namespace module {

void TutorialClientModule::init() {
    invoke_init(*p_main);
}

void TutorialClientModule::ready() {
    invoke_ready(*p_main);

    std::string payload = config.request_payload;
    const auto response = r_tutorial_interface->call_command_tutorial(payload);
    EVLOG_info << "TutorialClientModule(" << config.client_name << ") got response: " << response;
}

} // namespace module
