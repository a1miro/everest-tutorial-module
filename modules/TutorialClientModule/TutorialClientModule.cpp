// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "TutorialClientModule.hpp"

#include <chrono>
#include <exception>

namespace module {

TutorialClientModule::~TutorialClientModule() {
    keep_sending.store(false);
    if (periodic_request_thread.joinable()) {
        periodic_request_thread.join();
    }
}

void TutorialClientModule::init() {
    invoke_init(*p_main);
}

void TutorialClientModule::ready() {
    invoke_ready(*p_main);

    if (config.request_period_seconds <= 0) {
        EVLOG_error << "TutorialClientModule(" << config.client_name
                    << ") invalid request_period_seconds=" << config.request_period_seconds
                    << ". Expected > 0, sending only one request.";
        send_request_once();
        return;
    }

    send_request_once();

    keep_sending.store(true);
    periodic_request_thread = std::thread([this]() {
        const auto interval = std::chrono::seconds(config.request_period_seconds);
        while (keep_sending.load()) {
            std::this_thread::sleep_for(interval);
            if (!keep_sending.load()) {
                break;
            }
            send_request_once();
        }
    });
}

void TutorialClientModule::send_request_once() {
    try {
        std::string payload = config.request_payload;
        const auto response = r_tutorial_interface->call_command_tutorial(payload);
        EVLOG_info << "TutorialClientModule(" << config.client_name << ") got response: " << response;
    } catch (const std::exception& e) {
        EVLOG_error << "TutorialClientModule(" << config.client_name << ") request failed: " << e.what();
    }
}

} // namespace module
