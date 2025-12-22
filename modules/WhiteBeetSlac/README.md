# WhiteBeet SLAC Module for EVerest

This module provides direct integration between EVerest and the WHITE-beet-EI hardware for SLAC (Signal Level Attenuation Characterization) functionality in ISO15118-3 charging.

## Overview

The WhiteBeet SLAC module replaces the standard `EvseSlac` module, which sends raw HomePlug AV packets. Instead, it uses the WhiteBeet's high-level API (via FreeV2G library) to control SLAC matching. This is necessary because the WhiteBeet's QCA7005 PLC chip handles SLAC internally through firmware.

## Architecture

```
┌─────────────┐
│  EVerest    │
│ EvseManager │
└──────┬──────┘
       │ SLAC interface
       ↓
┌─────────────────┐
│ WhiteBeetSlac   │
│ (This Module)   │
└──────┬──────────┘
       │ Python/C++ API
       ↓
┌─────────────────┐      Ethernet      ┌──────────────┐
│   FreeV2G       │◄────────────────────►│  WhiteBeet   │
│ (Python Lib)    │   Framing Protocol  │  Hardware    │
└─────────────────┘                     └──────┬───────┘
                                               │
                                               ↓
                                        ┌──────────────┐
                                        │   QCA7005    │
                                        │  PLC Modem   │
                                        └──────┬───────┘
                                               │
                                               ↓
                                          Powerline
```

## Prerequisites

1. **FreeV2G Library**: Install at `/opt/FreeV2G`
   ```bash
   sudo git clone https://github.com/Sevenstax/FreeV2G.git /opt/FreeV2G
   ```

2. **Python 3 Development Libraries**:
   ```bash
   sudo apt-get install python3-dev python3-pip
   sudo pip3 install scapy pylibpcap
   ```

3. **WhiteBeet Hardware**: 
   - Connected via Ethernet (e.g., eth0)
   - Powered on and accessible
   - Note the MAC address (check label or use `arp-scan`)

## Building

1. **Configure the workspace**:
   ```bash
   cd /home/amironenko/projects/imx93evk-rolec/workspace/everest-tutorial-module
   cmake -B build -S . -DCMAKE_INSTALL_PREFIX=./dist
   ```

2. **Build the module**:
   ```bash
   cmake --build build --target install
   ```

3. **Verify installation**:
   ```bash
   ls dist/modules/WhiteBeetSlac/
   ```

## Configuration

Edit your EVerest configuration YAML to use WhiteBeetSlac instead of EvseSlac:

```yaml
evse_manager:
  connections:
    slac:
      - module_id: whitebeet_slac
        implementation_id: main

whitebeet_slac:
  module: WhiteBeetSlac
  config_module:
    device: eth0                          # Ethernet interface connected to WhiteBeet
    whitebeet_mac: "c4:93:00:34:a4:e4"   # Your WhiteBeet's MAC address
    slac_timeout_ms: 50000                # SLAC timeout (50 seconds)
    publish_mac_on_match_cnf: true
```

See `config-whitebeet-slac.yaml` for a complete example.

## Running

1. **Set environment variables** (if not using standard paths):
   ```bash
   export PYTHONPATH=/opt/FreeV2G:$PYTHONPATH
   ```

2. **Run EVerest** (as root for network access):
   ```bash
   sudo ./dist/bin/manager --conf config-whitebeet-slac.yaml
   ```

## How It Works

1. **Initialization Phase**:
   - Module connects to WhiteBeet via Ethernet
   - Initializes Control Pilot (CP) in EVSE mode
   - Sets CP duty cycle to 100%
   - Starts SLAC module on QCA7005 chip

2. **Ready State**:
   - Module publishes `state: UNMATCHED`
   - Waits for Control Pilot to enter state B/C/D

3. **Matching Phase** (when vehicle connects):
   - CP enters state B (vehicle connected)
   - Module receives `enter_bcd` command
   - Sets CP duty cycle to 5% (signals EV to start SLAC)
   - Calls `slacStartMatching()` on WhiteBeet
   - Waits for `slacMatched()` callback

4. **Matched State**:
   - Publishes `state: MATCHED`
   - Publishes `dlink_ready: true`
   - ISO15118 high-level communication can proceed
   - Optional: Publishes EV MAC address for autocharge

5. **Disconnection**:
   - CP leaves state B/C/D
   - Module receives `leave_bcd` command
   - Publishes `dlink_ready: false` and `state: UNMATCHED`

## Troubleshooting

### Module fails to start
- Check FreeV2G is installed: `ls /opt/FreeV2G/Whitebeet.py`
- Verify Python can import: `python3 -c "import sys; sys.path.insert(0, '/opt/FreeV2G'); from Whitebeet import Whitebeet"`
- Check permissions: EVerest must run as root

### Cannot connect to WhiteBeet
- Verify network interface: `ip link show eth0`
- Check WhiteBeet is reachable: `ping <whitebeet_ip>`
- Verify MAC address is correct
- Check Ethernet cable connection

### SLAC matching fails
- Check CP state transitions in logs
- Verify WhiteBeet firmware is up to date
- Ensure vehicle supports ISO15118
- Check powerline connection quality
- Enable verbose logging in EVerest

### Python errors
- Install dependencies: `pip3 install scapy pylibpcap`
- Check Python version: `python3 --version` (should be 3.6+)

## Differences from Standard EvseSlac

| Feature | EvseSlac | WhiteBeetSlac |
|---------|----------|---------------|
| Packet handling | Raw HomePlug AV | WhiteBeet API |
| SLAC state machine | In module | In firmware |
| PLC modem | Generic | QCA7005 only |
| Dependencies | None | FreeV2G, Python3 |
| Complexity | High | Low |
| Reliability | Depends on packet timing | Firmware-handled |

## Development

### Testing without hardware

For development without WhiteBeet hardware, you can mock the Python calls:

```bash
export WHITEBEET_MOCK=1  # Future enhancement
```

### Adding features

Key files to modify:
- `WhiteBeetSlac.cpp`: Main worker thread and Python integration
- `main/slacImpl.cpp`: Command handlers for SLAC interface
- `manifest.yaml`: Configuration options

### Debugging

Enable detailed logging:
```bash
EVLOG_LEVEL=debug sudo ./dist/bin/manager --conf config-whitebeet-slac.yaml
```

Monitor WhiteBeet communication:
```bash
sudo tcpdump -i eth0 -X ether proto 0x6003
```

## License

Apache 2.0 - See LICENSE file

## Authors

- Andrei Mironenko, Parallel Dynamic Ltd.

## Support

For issues specific to:
- **WhiteBeet hardware**: Contact Sevenstax
- **FreeV2G library**: https://github.com/Sevenstax/FreeV2G
- **EVerest framework**: https://everest.github.io/
- **This module**: Create an issue in your repository
