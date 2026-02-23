// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#ifndef TUTORIAL_CLIENT_MODULE_MAIN_EMPTY_IMPL_HPP
#define TUTORIAL_CLIENT_MODULE_MAIN_EMPTY_IMPL_HPP

//
// AUTO GENERATED - MARKED REGIONS WILL BE KEPT
// template version 3
//

#include <generated/interfaces/empty/Implementation.hpp>

#include "../TutorialClientModule.hpp"

namespace module {
namespace main {

struct Conf {};

class emptyImpl : public emptyImplBase {
public:
    emptyImpl() = delete;
    emptyImpl(Everest::ModuleAdapter* ev, const Everest::PtrContainer<TutorialClientModule>& mod, Conf& config) :
        emptyImplBase(ev, "main"), mod(mod), config(config) {};

protected:
    // no commands defined for this interface

private:
    const Everest::PtrContainer<TutorialClientModule>& mod;
    const Conf& config;

    virtual void init() override;
    virtual void ready() override;
};

} // namespace main
} // namespace module

#endif // TUTORIAL_CLIENT_MODULE_MAIN_EMPTY_IMPL_HPP
