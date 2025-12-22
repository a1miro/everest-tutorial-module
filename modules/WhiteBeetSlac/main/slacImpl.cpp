// SPDX-License-Identifier: Apache-2.0
// Copyright Parallel Dynamic Ltd.

#include "slacImpl.hpp"

namespace module {
namespace main {

void slacImpl::handle_reset(bool& enable) {
    EVLOG_info << "SLAC reset requested (enable: " << enable << ")";
    mod->set_slac_enabled(enable);
}

void slacImpl::handle_enter_bcd() {
    EVLOG_info << "Entering BCD state (vehicle connected)";
    mod->in_bcd_state = true;
}

void slacImpl::handle_leave_bcd() {
    EVLOG_info << "Leaving BCD state (vehicle disconnected)";
    mod->in_bcd_state = false;
}

void slacImpl::handle_dlink_terminate() {
    EVLOG_info << "Data link terminate requested";
    // This will be handled by the worker thread when in_bcd_state becomes false
    publish_state("UNMATCHED");
    publish_dlink_ready(false);
}

void slacImpl::handle_dlink_error() {
    EVLOG_info << "Data link error - restarting matching process";
    publish_state("UNMATCHED");
    publish_dlink_ready(false);
    // The worker thread will restart matching if still in BCD state
}

void slacImpl::handle_dlink_pause() {
    EVLOG_info << "Data link pause requested (power saving mode)";
    // WhiteBeet doesn't have a specific pause mode, so we just log it
}

void slacImpl::publish_state(const std::string& state) {
    slacImplBase::publish_state(state);
}

void slacImpl::publish_dlink_ready(bool ready) {
    slacImplBase::publish_dlink_ready(ready);
}

void slacImpl::publish_request_error_routine() {
    slacImplBase::publish_request_error_routine();
}

void slacImpl::publish_ev_mac_address(const std::string& mac) {
    slacImplBase::publish_ev_mac_address(mac);
}

} // namespace main
} // namespace module
