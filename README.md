# ESP32 Web-Interface Controlled Relay Board

Control a 6-channel relay board from any browser on your network using an ESP32.
The ESP32 hosts a small HTTP server and serves a self-contained web page with an
ON/OFF button per relay. State is kept in memory and reflected on the page.

It is a single Arduino sketch with no external libraries, aimed at hobbyists who
want simple local on/off control of lights, pumps, or other loads without a
cloud service or a home-automation hub.

## Table of contents

- [Features](#features)
- [Web interface](#web-interface)
- [How it works](#how-it-works)
- [Hardware](#hardware)
- [Software setup](#software-setup)
- [HTTP API](#http-api)
- [Home Assistant](#home-assistant)
- [Security notes](#security-notes)
- [Troubleshooting](#troubleshooting)
- [License](#license)

## Features

- Controls **6 relays** on fixed ESP32 GPIO pins.
- Built-in, mobile-friendly web page (no internet access or external assets
  needed) showing each device's state and an ON/OFF button.
- Simple HTTP endpoints (`POST /<n>/on`, `POST /<n>/off`) usable from scripts
  or home-automation tools.
- CSRF protection: state changes are accepted only as same-origin `POST`
  requests.
- Hardened request parsing: 2-second idle timeout per client and a 256-byte cap
  per request/header line to protect the heap.
- Serial logging (115200 baud) of connection status, the IP address, incoming
  requests, and relay changes.
- Uses only the `WiFi` library bundled with the ESP32 Arduino core; no extra
  libraries to install.

## Web interface

![Web interface](https://raw.githubusercontent.com/t0mer/esp32-web-interface-controlled-relay-board/main/assets/screenshots/web-interface.png)

Each card shows a device's current state and a button that switches it to the
opposite state (a device that is on shows an **OFF** button, and vice versa).
Buttons submit a `POST` request that is only accepted when it originates from
the device's own page (same-origin check), so a relay can't be flipped by a
link, an `<img>` tag, or a cross-site form.

## How it works

1. At boot, all six relay pins are set as outputs and driven `LOW` (off).
2. The ESP32 joins your Wi-Fi network (DHCP) and starts a raw `WiFiServer` on
   **port 80**.
3. For each connection, the sketch reads the request line and the `Host`,
   `Origin`, and `Referer` headers.
4. If the request is a same-origin `POST` to `/<n>/on` or `/<n>/off`, the
   matching GPIO is switched and its state is updated.
5. Every complete request, whatever the method or path, is answered with the full HTML
   control page reflecting the current states, and the connection is closed.

One client is served at a time. Relay states live in RAM only, so after a reboot
or power loss every relay starts **off**.

## Hardware

| Item | Notes |
|---|---|
| ESP32 dev board | Any common ESP32 (e.g. ESP32-WROOM DevKit). |
| 6-channel relay module | 3.3 V-logic-compatible trigger inputs recommended. |
| Power supply | Size for the relay coils; don't power the relay board from the ESP32's 3.3 V rail. |
| Jumper wires | ESP32 GPIO → relay `IN` pins, plus GND (and relay VCC from a suitable supply). |

> ⚠️ Relays commonly switch **mains voltage**. Mains wiring is dangerous and, in
> many places, must be done by a qualified electrician. Work unplugged and at
> your own risk.

### GPIO → relay mapping

The sketch drives these pins (defined at the top of
`esp32_relay_http/esp32_relay_http.ino`):

| Device | ESP32 GPIO | Constant |
|---|---|---|
| Device 1 | GPIO 4 | `Device1` |
| Device 2 | GPIO 5 | `Device2` |
| Device 3 | GPIO 18 | `Device3` |
| Device 4 | GPIO 19 | `Device4` |
| Device 5 | GPIO 21 | `Device5` |
| Device 6 | GPIO 22 | `Device6` |

To use different pins, change these constants before flashing.

Each pin is initialised `LOW` at boot; `ON` drives it `HIGH`. If your relay
module is **active-low** (energises when the input is `LOW`), the ON/OFF sense
will be inverted — either wire to the relay's NC contacts or swap `HIGH`/`LOW`
in `applyAction()` (and in the `digitalWrite(..., LOW)` calls in `setup()`).

Wire the common ground of the relay board to the ESP32 `GND`.

## Software setup

### 1. Install the Arduino IDE and ESP32 support

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. In **File → Preferences → Additional Boards Manager URLs**, add:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. In **Tools → Board → Boards Manager**, search for **esp32** and install the
   *esp32 by Espressif Systems* package.
4. Select your board under **Tools → Board → ESP32 Arduino** (e.g.
   *ESP32 Dev Module*).

No additional libraries are required; `WiFi.h` ships with the ESP32 core.

### 2. Configure your Wi-Fi credentials

Open `esp32_relay_http/esp32_relay_http.ino` and replace the masked
placeholders with your network name and password:

```cpp
const char* ssid     = "**************";  // Network SSID (name)
const char* password = "**************";  // Network password
```

The ESP32 must join a **2.4 GHz** network (the ESP32 radio does not support
5 GHz). Avoid committing your real credentials if you fork the repository.

Other values you may want to adjust in the same file:

| Setting | Default | Purpose |
|---|---|---|
| `WiFiServer server(80)` | `80` | HTTP port. |
| `Device1` … `Device6` | see [mapping](#gpio--relay-mapping) | Relay GPIO pins. |
| `timeoutTime` | `2000` ms | Drop a client that stops sending for this long. |
| `MAX_LINE_LENGTH` | `256` bytes | Maximum buffered length of each request or header line. |

### 3. Flash the ESP32

1. Connect the ESP32 over USB and select its port under **Tools → Port**.
2. Click **Upload**.
3. Open **Tools → Serial Monitor** at **115200 baud**. After it connects to
   Wi-Fi, the ESP32 prints its IP address:

   ```
   Connecting to <your SSID>
   .....
   WiFi connected.
   IP address:
   192.168.1.50
   ```

### 4. Open the interface

Browse to `http://<ESP32-IP>/` (e.g. `http://192.168.1.50/`) from any device on
the same network and use the ON/OFF buttons.

The IP address is assigned by your router via DHCP. Consider a DHCP reservation
so the address doesn't change. The sketch doesn't advertise an mDNS
(`.local`) hostname.

## HTTP API

State changes are performed with a same-origin `POST`; the root page is a `GET`.

| Method | Path | Action |
|---|---|---|
| `GET`  | `/`        | Return the control page. |
| `POST` | `/1/on`    | Turn Device 1 on. |
| `POST` | `/1/off`   | Turn Device 1 off. |
| …      | `/N/on`, `/N/off` | Same for devices `N` = 1–6. |

- A `POST` is applied only if its `Origin` (or, if absent, `Referer`) host
  matches the `Host` header. A `POST` carrying neither header is rejected.
- `GET` requests never change relay state, so control from scripts/tools must
  send a `POST` with a matching `Origin` header.
- Every response, including rejected or unknown requests, is
  `200 OK` with the HTML control page (`Content-type:text/html`). There is no
  JSON status endpoint; the current states appear in the page as
  `Device N - State on|off`.
- The request body is ignored.

Examples with `curl` (replace the IP with your ESP32's address):

```bash
# Turn Device 1 on
curl -X POST -H "Origin: http://192.168.1.50" http://192.168.1.50/1/on

# Turn Device 3 off
curl -X POST -H "Origin: http://192.168.1.50" http://192.168.1.50/3/off

# Read the current states
curl -s http://192.168.1.50/ | grep -o 'Device [0-9] - State [a-z]*'
```

If you use a non-default port, include it in the `Origin` value
(e.g. `http://192.168.1.50:8080`), because it must match the `Host` header.

## Home Assistant

The endpoints can be called from Home Assistant with the
[`rest_command`](https://www.home-assistant.io/integrations/rest_command/)
integration. Add to `configuration.yaml`:

```yaml
rest_command:
  relay_1_on:
    url: "http://192.168.1.50/1/on"
    method: post
    headers:
      Origin: "http://192.168.1.50"
  relay_1_off:
    url: "http://192.168.1.50/1/off"
    method: post
    headers:
      Origin: "http://192.168.1.50"
```

Call `rest_command.relay_1_on` / `rest_command.relay_1_off` from automations,
scripts, or dashboard buttons. Because the device has no machine-readable state
endpoint, Home Assistant can't reliably read back the relay state.

## Security notes

This firmware is intended for a **trusted local network**:

- **No authentication.** Anyone who can reach the device on your LAN can control
  the relays. Do not expose the device to the internet.
- **Plain HTTP.** Traffic is unencrypted.
- **CSRF protection** is in place (same-origin `POST` only), which blocks ordinary
  cross-site requests from other websites in a browser (it does not stop DNS-rebinding attacks), but it is not a
  substitute for auth: any non-browser client can set a matching `Origin`
  header.
- **Wi-Fi credentials are compiled into the firmware** as plain strings. Keep
  them out of public forks.

## Troubleshooting

| Symptom | Likely cause / fix |
|---|---|
| Serial Monitor prints dots forever after `Connecting to …` | The ESP32 can't join the network. Check the SSID/password and make sure it is a 2.4 GHz network. The sketch waits indefinitely while the ESP32 core keeps retrying. |
| Serial Monitor shows garbage | Set the baud rate to **115200**. |
| Relays are ON when the page says off (and vice versa) | Your relay module is active-low. See [GPIO → relay mapping](#gpio--relay-mapping). |
| All relays are off after a power cut | Expected: state is kept in RAM only and every relay starts off at boot. |
| A script/`curl` `POST` doesn't switch the relay; Serial shows `Rejected cross-origin POST` | Send an `Origin` header whose host (and port) matches the URL you are calling. |
| The page is unreachable after a router restart | The DHCP address may have changed. Check the Serial Monitor or your router, and consider a DHCP reservation. |
| Compiler warning that `NetworkServer::available()` (`WiFiServer::available()`) is deprecated ("Renamed to accept()") | Seen with ESP32 Arduino core 3.x. It is only a warning; the sketch still builds and runs. |

## License

[Apache License 2.0](LICENSE).
