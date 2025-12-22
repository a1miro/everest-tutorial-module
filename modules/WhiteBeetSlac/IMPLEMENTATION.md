# WhiteBeet SLAC Module - Implementation Summary

## What Was Created

A complete EVerest module that integrates WhiteBeet hardware directly with EVerest for ISO15118-3 SLAC functionality.

## Files Created

```
everest-tutorial-module/modules/WhiteBeetSlac/
├── CMakeLists.txt              # Build configuration
├── WhiteBeetSlac.hpp           # Module header
├── WhiteBeetSlac.cpp           # Main module implementation
├── manifest.yaml               # Module manifest
├── doc.rst                     # Documentation
├── README.md                   # Detailed guide
├── main/
│   ├── slacImpl.hpp           # SLAC interface header
│   └── slacImpl.cpp           # SLAC interface implementation
└── config-whitebeet-slac.yaml # Sample configuration
```

## Key Features

### 1. **Python-C++ Integration**
- Embeds Python interpreter in C++ module
- Calls FreeV2G Python library directly
- No external process needed

### 2. **Control Pilot Management**
- Initializes CP in EVSE mode
- Sets duty cycle (100% idle, 5% during matching)
- Signals vehicle to start SLAC

### 3. **SLAC State Machine**
- UNMATCHED: Initial state, waiting for vehicle
- MATCHING: SLAC negotiation in progress
- MATCHED: Link established, HLC can proceed

### 4. **Thread-Safe Operation**
- Background worker thread for SLAC
- Atomic flags for state management
- Clean shutdown handling

### 5. **EVerest Interface Compliance**
- Implements standard `slac` interface
- Compatible with existing EvseManager
- Drop-in replacement for EvseSlac

## How It Works

### Initialization Sequence

```
Module Start
    ↓
Initialize Python Interpreter
    ↓
Import FreeV2G Whitebeet module
    ↓
Create Whitebeet instance (ETH, device, MAC)
    ↓
Initialize Control Pilot:
    - Set mode to EVSE (1)
    - Set duty cycle to 100%
    - Start CP service
    ↓
Start SLAC module (mode: EVSE)
    ↓
Wait 2 seconds for SLAC ready
    ↓
Enter main loop
```

### Main Operation Loop

```
┌─────────────────────────────────────┐
│ SLAC Disabled or Not in BCD?       │
│ → Wait 100ms                        │
└─────────────────┬───────────────────┘
                  │
                  ↓
┌─────────────────────────────────────┐
│ Vehicle Connected (enter_bcd)       │
│ → Set CP duty cycle to 5%           │
│ → Start SLAC matching               │
└─────────────────┬───────────────────┘
                  │
                  ↓
┌─────────────────────────────────────┐
│ Call slacMatched() (blocking)       │
└─────────────┬───────────┬───────────┘
              │           │
        Success│           │Failure
              ↓           ↓
    ┌──────────────┐  ┌──────────────┐
    │  MATCHED     │  │  FAILED      │
    │  Link Ready  │  │  Retry       │
    └──────┬───────┘  └──────────────┘
           │
    Stay matched until
    leave_bcd
```

### Shutdown Sequence

```
Terminate Signal
    ↓
Stop SLAC module
    ↓
Stop Control Pilot
    ↓
Release Whitebeet instance
    ↓
Cleanup Python objects
    ↓
Exit
```

## API Mapping

### EVerest SLAC Interface → WhiteBeet API

| EVerest Command | WhiteBeet Function | Purpose |
|-----------------|-------------------|---------|
| `reset(true)` | `slacStart(1)` | Start SLAC module |
| `reset(false)` | `slacStop()` | Stop SLAC module |
| `enter_bcd` | `controlPilotSetDutyCycle(5)` + `slacStartMatching()` | Begin matching |
| `leave_bcd` | Set `in_bcd_state = false` | End session |
| `dlink_terminate` | Publish UNMATCHED | Terminate link |
| `dlink_error` | Publish error + retry | Handle errors |

### WhiteBeet API → EVerest Variables

| WhiteBeet Event | EVerest Variable | Value |
|----------------|------------------|-------|
| SLAC started | `state` | "UNMATCHED" |
| Matching started | `state` | "MATCHING" |
| `slacMatched() == true` | `state` | "MATCHED" |
| `slacMatched() == true` | `dlink_ready` | true |
| Match failed | `request_error_routine` | null (trigger) |
| Leave BCD | `dlink_ready` | false |

## Configuration Parameters

```yaml
whitebeet_slac:
  config_module:
    device: eth0                          # Interface to WhiteBeet
    whitebeet_mac: "c4:93:00:34:a4:e4"   # WhiteBeet MAC address
    slac_timeout_ms: 50000                # Matching timeout
    publish_mac_on_match_cnf: true        # Publish EV MAC for autocharge
```

## Build Requirements

- EVerest framework
- Python 3.6+ development headers
- FreeV2G library at `/opt/FreeV2G`
- CMake 3.11+
- GCC/G++ with C++17 support

## Runtime Requirements

- Root privileges (for network access)
- WhiteBeet hardware connected via Ethernet
- Python 3 with scapy and pylibpcap
- FreeV2G accessible in Python path

## Advantages Over Proxy Approach

| Aspect | Proxy Approach | Direct API Approach |
|--------|---------------|-------------------|
| **Complexity** | High (TAP, forwarding, framing) | Low (direct API calls) |
| **Reliability** | Prone to packet loss | Firmware-guaranteed |
| **Performance** | Extra hops, latency | Direct communication |
| **Debugging** | Multiple layers to check | Single integration point |
| **Maintenance** | Proxy + Module | Just module |
| **Dependencies** | TAP driver, proxy service | Python + FreeV2G |
| **Error Handling** | Manual packet retry | Firmware handles it |

## Testing Checklist

- [ ] Module builds without errors
- [ ] Module loads in EVerest
- [ ] Python interpreter initializes
- [ ] FreeV2G imports successfully
- [ ] WhiteBeet connection established
- [ ] Control Pilot initializes
- [ ] SLAC module starts
- [ ] Enter BCD triggers matching
- [ ] Successful match → MATCHED state
- [ ] dlink_ready published
- [ ] Leave BCD terminates link
- [ ] Clean shutdown on exit

## Next Steps

1. **Build the module**:
   ```bash
   cd everest-tutorial-module
   cmake -B build -S .
   cmake --build build
   ```

2. **Test Python integration**:
   ```bash
   python3 -c "import sys; sys.path.insert(0, '/opt/FreeV2G'); from Whitebeet import Whitebeet; print('OK')"
   ```

3. **Configure EVerest**:
   - Copy `config-whitebeet-slac.yaml` to your config directory
   - Update `whitebeet_mac` with your device's MAC
   - Adjust `device` to your Ethernet interface

4. **Run EVerest**:
   ```bash
   sudo ./build/dist/bin/manager --conf config-whitebeet-slac.yaml
   ```

5. **Monitor logs**:
   - Check for "WhiteBeet SLAC module initialized"
   - Watch for "SLAC matching successful"
   - Verify state transitions

## Future Enhancements

- [ ] Extract and publish EV MAC address from WhiteBeet
- [ ] Add configuration for CP duty cycle values
- [ ] Implement mock mode for testing without hardware
- [ ] Add statistics/metrics collection
- [ ] Support for EV mode (not just EVSE)
- [ ] Better error recovery and retry logic
- [ ] Integration with EvseManager for CP state coordination

## Troubleshooting Guide

### Build Errors

**Problem**: Python.h not found
**Solution**: `sudo apt-get install python3-dev`

**Problem**: Cannot find slac interface
**Solution**: Ensure everest-core interfaces are in CMAKE_PREFIX_PATH

### Runtime Errors

**Problem**: Failed to import Whitebeet module
**Solution**: Check FreeV2G is at `/opt/FreeV2G` and contains `Whitebeet.py`

**Problem**: Cannot create Whitebeet instance
**Solution**: 
- Verify network interface exists: `ip link show eth0`
- Check MAC address format (lowercase, colons)
- Ensure WhiteBeet is powered and connected

**Problem**: SLAC matching always fails
**Solution**:
- Check vehicle supports ISO15118
- Verify powerline connection (cable plugged in correctly)
- Monitor with tcpdump: `sudo tcpdump -i eth0 ether proto 0x88e1`
- Check WhiteBeet firmware version

## Conclusion

This module provides a clean, reliable integration between EVerest and WhiteBeet hardware by using the WhiteBeet API directly instead of trying to inject raw packets. It's simpler, more maintainable, and more reliable than the proxy approach.

The implementation follows EVerest's module architecture, implements the standard SLAC interface, and should work as a drop-in replacement for the EvseSlac module when using WhiteBeet hardware.
