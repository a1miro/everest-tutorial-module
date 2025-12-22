// SPDX-License-Identifier: Apache-2.0
// Copyright Parallel Dynamic Ltd.

#include "WhiteBeetSlac.hpp"

#include <Python.h>
#include <chrono>
#include <thread>

namespace module {

void WhiteBeetSlac::init() {
    invoke_init(*p_main);
    
    // Initialize Python interpreter
    Py_Initialize();
    
    // Add FreeV2G to Python path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.insert(0, '/opt/FreeV2G')");
    
    EVLOG_info << "WhiteBeet SLAC module initialized";
}

void WhiteBeetSlac::ready() {
    invoke_ready(*p_main);
    
    // Start SLAC worker thread
    slac_thread = std::make_unique<std::thread>(&WhiteBeetSlac::slac_worker_thread, this);
    
    EVLOG_info << "WhiteBeet SLAC module ready";
}

void WhiteBeetSlac::set_slac_enabled(bool enabled) {
    slac_enabled = enabled;
    
    if (enabled) {
        EVLOG_info << "SLAC enabled - starting matching process";
        p_main->publish_state("MATCHING");
    } else {
        EVLOG_info << "SLAC disabled";
        p_main->publish_state("UNMATCHED");
        p_main->publish_dlink_ready(false);
    }
}

void WhiteBeetSlac::slac_worker_thread() {
    EVLOG_info << "SLAC worker thread started";
    
    // Import Python modules
    PyObject* pName = PyUnicode_DecodeFSDefault("Whitebeet");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    
    if (pModule == nullptr) {
        PyErr_Print();
        EVLOG_error << "Failed to import Whitebeet module from FreeV2G";
        return;
    }
    
    // Get Whitebeet class
    PyObject* pClass = PyObject_GetAttrString(pModule, "Whitebeet");
    if (pClass == nullptr || !PyCallable_Check(pClass)) {
        PyErr_Print();
        EVLOG_error << "Cannot find Whitebeet class";
        Py_DECREF(pModule);
        return;
    }
    
    // Create Whitebeet instance
    PyObject* pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString("ETH"));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(config.device.c_str()));
    PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(config.whitebeet_mac.c_str()));
    
    PyObject* pWhitebeet = PyObject_CallObject(pClass, pArgs);
    Py_DECREF(pArgs);
    Py_DECREF(pClass);
    
    if (pWhitebeet == nullptr) {
        PyErr_Print();
        EVLOG_error << "Failed to create Whitebeet instance";
        Py_DECREF(pModule);
        return;
    }
    
    EVLOG_info << "WhiteBeet connection established on " << config.device 
               << " (MAC: " << config.whitebeet_mac << ")";
    
    // Initialize Control Pilot
    EVLOG_info << "Initializing Control Pilot for EVSE mode...";
    
    // Set CP mode to EVSE (1)
    PyObject* pCpSetMode = PyObject_CallMethod(pWhitebeet, "controlPilotSetMode", "i", 1);
    if (pCpSetMode) Py_DECREF(pCpSetMode);
    
    // Set CP duty cycle to 100%
    PyObject* pCpSetDc = PyObject_CallMethod(pWhitebeet, "controlPilotSetDutyCycle", "i", 100);
    if (pCpSetDc) Py_DECREF(pCpSetDc);
    
    // Start CP service
    PyObject* pCpStart = PyObject_CallMethod(pWhitebeet, "controlPilotStart", nullptr);
    if (pCpStart) Py_DECREF(pCpStart);
    
    EVLOG_info << "Control Pilot initialized";
    
    // Start SLAC in EVSE mode
    EVLOG_info << "Starting SLAC module in EVSE mode...";
    PyObject* pSlacStart = PyObject_CallMethod(pWhitebeet, "slacStart", "i", 1);
    if (pSlacStart) Py_DECREF(pSlacStart);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EVLOG_info << "SLAC module ready";
    
    // Main SLAC loop
    while (!terminate_requested) {
        if (!slac_enabled || !in_bcd_state) {
            // Not enabled or not in charging state - just wait
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // We're in BCD state and SLAC is enabled - start matching
        EVLOG_info << "Entering BCD state - starting SLAC matching";
        p_main->publish_state("MATCHING");
        
        // Set duty cycle to 5% to signal EV to start SLAC
        PyObject* pCpSetDc5 = PyObject_CallMethod(pWhitebeet, "controlPilotSetDutyCycle", "i", 5);
        if (pCpSetDc5) Py_DECREF(pCpSetDc5);
        
        // Start SLAC matching
        PyObject* pSlacMatch = PyObject_CallMethod(pWhitebeet, "slacStartMatching", nullptr);
        if (pSlacMatch) Py_DECREF(pSlacMatch);
        
        // Wait for SLAC to complete (with timeout)
        PyObject* pSlacMatched = PyObject_CallMethod(pWhitebeet, "slacMatched", nullptr);
        
        if (pSlacMatched != nullptr && PyObject_IsTrue(pSlacMatched)) {
            EVLOG_info << "SLAC matching successful!";
            Py_DECREF(pSlacMatched);
            
            p_main->publish_state("MATCHED");
            p_main->publish_dlink_ready(true);
            
            // Get EV MAC address if requested
            if (config.publish_mac_on_match_cnf) {
                // TODO: Extract MAC address from WhiteBeet
                // For now, publish a placeholder
                // p_main->publish_ev_mac_address("00:00:00:00:00:00");
            }
            
            // Stay matched until we leave BCD state
            while (in_bcd_state && !terminate_requested) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            // Left BCD state - terminate link
            EVLOG_info << "Left BCD state - terminating SLAC link";
            p_main->publish_state("UNMATCHED");
            p_main->publish_dlink_ready(false);
            
        } else {
            EVLOG_warning << "SLAC matching failed or timed out";
            if (pSlacMatched) Py_DECREF(pSlacMatched);
            
            p_main->publish_state("UNMATCHED");
            p_main->publish_request_error_routine();
            
            // Wait a bit before retry
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
    
    // Cleanup
    EVLOG_info << "SLAC worker thread stopping...";
    
    // Stop SLAC
    PyObject* pSlacStop = PyObject_CallMethod(pWhitebeet, "slacStop", nullptr);
    if (pSlacStop) Py_DECREF(pSlacStop);
    
    // Stop CP
    PyObject* pCpStop = PyObject_CallMethod(pWhitebeet, "controlPilotStop", nullptr);
    if (pCpStop) Py_DECREF(pCpStop);
    
    Py_DECREF(pWhitebeet);
    Py_DECREF(pModule);
    
    EVLOG_info << "SLAC worker thread stopped";
}

} // namespace module
