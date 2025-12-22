// SPDX-License-Identifier: Apache-2.0
// Copyright Parallel Dynamic Ltd.

#ifndef WHITEBEET_SLAC_HPP
#define WHITEBEET_SLAC_HPP

#include "ld-ev.hpp"

// headers for provided interface implementations
#include <generated/interfaces/slac/Implementation.hpp>

// ev@4bf81b14-a215-475c-a1d3-0a484ae48918:v1
#include <thread>
#include <atomic>
#include <memory>
// ev@4bf81b14-a215-475c-a1d3-0a484ae48918:v1

namespace module {

struct Conf {
    std::string device;
    std::string whitebeet_mac;
    int slac_timeout_ms;
    bool publish_mac_on_match_cnf;
};

class WhiteBeetSlac : public Everest::ModuleBase {
public:
    WhiteBeetSlac() = delete;
    WhiteBeetSlac(const ModuleInfo& info, std::unique_ptr<slacImplBase> p_main, Conf& config) :
        ModuleBase(info), p_main(std::move(p_main)), config(config){};

    const std::unique_ptr<slacImplBase> p_main;
    const Conf& config;

    // ev@1fce4c5e-0ab8-41bb-90f7-14277703d2ac:v1
    void set_slac_enabled(bool enabled);
    // ev@1fce4c5e-0ab8-41bb-90f7-14277703d2ac:v1

protected:
    // ev@4714b2ab-a24f-4b95-ab81-36439e1478de:v1
    // ev@4714b2ab-a24f-4b95-ab81-36439e1478de:v1

private:
    friend class LdEverest;
    void init();
    void ready();

    // ev@211cfdbe-f69a-4cd6-a4ec-f8aaa3d1b6c8:v1
    std::unique_ptr<std::thread> slac_thread;
    std::atomic<bool> slac_enabled{false};
    std::atomic<bool> in_bcd_state{false};
    std::atomic<bool> terminate_requested{false};
    
    void slac_worker_thread();
    // ev@211cfdbe-f69a-4cd6-a4ec-f8aaa3d1b6c8:v1
};

// ev@087e516b-124c-48df-94fb-109508c7cda9:v1
// ev@087e516b-124c-48df-94fb-109508c7cda9:v1

} // namespace module

#endif // WHITEBEET_SLAC_HPP
