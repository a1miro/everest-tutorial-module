WhiteBeet SLAC Module
=====================

This module implements the ISO15118-3 SLAC interface for the WHITE-beet-EI hardware using the FreeV2G library.

Unlike the standard EvseSlac module which sends raw HomePlug AV packets to the PLC modem, this module uses the WhiteBeet's high-level API to control SLAC matching. The WhiteBeet device contains a Qualcomm QCA7005 PLC chip that handles SLAC internally.

Features
--------

- Direct integration with WhiteBeet hardware via FreeV2G Python library
- Automatic Control Pilot (CP) initialization and management
- SLAC state machine fully managed by WhiteBeet firmware
- Compatible with EVerest's standard SLAC interface

Configuration
-------------

- **device**: Network interface for communicating with WhiteBeet (default: eth0)
- **whitebeet_mac**: MAC address of the WhiteBeet device
- **slac_timeout_ms**: Timeout for SLAC matching in milliseconds (default: 50000)
- **publish_mac_on_match_cnf**: Whether to publish EV MAC on successful match (default: true)

Prerequisites
-------------

- FreeV2G library installed at /opt/FreeV2G
- Python 3 development libraries
- WhiteBeet hardware connected via Ethernet

Usage
-----

Replace the standard EvseSlac module in your EVerest configuration with WhiteBeetSlac:

```yaml
evse:
  module: EvseManager
  config_module:
    # ... other config
  connections:
    slac:
      - module_id: whitebeet_slac
        implementation_id: main

whitebeet_slac:
  module: WhiteBeetSlac
  config_module:
    device: eth0
    whitebeet_mac: "c4:93:00:34:a4:e4"
```

How It Works
------------

1. **Initialization**: Module connects to WhiteBeet via Ethernet and initializes Control Pilot in EVSE mode
2. **Ready State**: SLAC module is started on the QCA7005 chip and ready to respond to requests
3. **BCD State**: When CP enters state B/C/D (vehicle connected), duty cycle is set to 5% and SLAC matching begins
4. **Matching**: WhiteBeet handles the entire SLAC protocol internally and reports success/failure
5. **Matched**: Once matched, the data link is established and higher layers can proceed with ISO15118
6. **Disconnection**: When leaving BCD state, the SLAC link is terminated

Architecture Difference
-----------------------

**Standard EvseSlac:**
```
EVerest → Raw SLAC packets → TAP/Ethernet → PLC Modem → Powerline
```

**WhiteBeetSlac:**
```
EVerest → WhiteBeet API → Framing Protocol → WhiteBeet Firmware → QCA7005 → Powerline
```

The WhiteBeet approach is simpler and more reliable because:
- SLAC state machine runs in tested firmware
- No need for raw packet manipulation
- Built-in error handling and retry logic
- Works with WhiteBeet's integrated design

Notes
-----

- This module requires root privileges if accessing network interfaces directly
- Make sure FreeV2G Python modules are accessible (sys.path includes /opt/FreeV2G)
- The WhiteBeet must be properly powered and connected before starting EVerest
