// SPDX-License-Identifier: Apache-2.0
// Copyright Parallel Dynamic Ltd.

#ifndef MAIN_SLAC_IMPL_HPP
#define MAIN_SLAC_IMPL_HPP

#include <generated/interfaces/slac/Implementation.hpp>

#include "../WhiteBeetSlac.hpp"

namespace module {
namespace main {

struct Conf {};

class slacImpl : public slacImplBase {
public:
    slacImpl() = delete;
    slacImpl(Everest::ModuleAdapter* ev, const Everest::PtrContainer<WhiteBeetSlac>& mod, Conf& config) :
        slacImplBase(ev, "main"), mod(mod), config(config){};

    // Command handlers
    virtual void handle_reset(bool& enable) override;
    virtual void handle_enter_bcd() override;
    virtual void handle_leave_bcd() override;
    virtual void handle_dlink_terminate() override;
    virtual void handle_dlink_error() override;
    virtual void handle_dlink_pause() override;

    // Variable publications
    void publish_state(const std::string& state);
    void publish_dlink_ready(bool ready);
    void publish_request_error_routine();
    void publish_ev_mac_address(const std::string& mac);

private:
    const Everest::PtrContainer<WhiteBeetSlac>& mod;
    const Conf& config;
};

} // namespace main
} // namespace module

#endif // MAIN_SLAC_IMPL_HPP
