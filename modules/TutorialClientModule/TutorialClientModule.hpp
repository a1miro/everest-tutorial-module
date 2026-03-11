// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#ifndef TUTORIAL_CLIENT_MODULE_HPP
#define TUTORIAL_CLIENT_MODULE_HPP

//
// AUTO GENERATED - MARKED REGIONS WILL BE KEPT
// template version 2
//

#include "ld-ev.hpp"

// headers for provided interface implementations
#include <generated/interfaces/empty/Implementation.hpp>

// headers for required interface implementations
#include <generated/interfaces/interface_tutorial_module/Interface.hpp>

#include <atomic>
#include <thread>

namespace module {

struct Conf {
    std::string client_name;
    std::string request_payload;
    int request_period_seconds{5};
};

class TutorialClientModule : public Everest::ModuleBase {
public:
    TutorialClientModule() = delete;
    TutorialClientModule(const ModuleInfo& info, std::unique_ptr<emptyImplBase> p_main,
                        std::unique_ptr<interface_tutorial_moduleIntf> r_tutorial_interface, Conf& config) :
        ModuleBase(info), p_main(std::move(p_main)), r_tutorial_interface(std::move(r_tutorial_interface)),
        config(config) {};
    ~TutorialClientModule();

    const std::unique_ptr<emptyImplBase> p_main;
    const std::unique_ptr<interface_tutorial_moduleIntf> r_tutorial_interface;
    const Conf& config;

private:
    friend class LdEverest;
    void init();
    void ready();
    void send_request_once();

    std::atomic<bool> keep_sending{false};
    std::thread periodic_request_thread;
};

} // namespace module

#endif // TUTORIAL_CLIENT_MODULE_HPP
