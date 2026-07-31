# ESP32 Web-Interface Controlled Relay Board

Control a 6-channel relay board from any browser on your network using an ESP32.
The ESP32 hosts a small HTTP server and serves a self-contained web page with an
ON/OFF button per relay. State is kept in memory and reflected on the page.

## Web interface

![Web interface](https://raw.githubusercontent.com/t0mer/esp32-web-interface-controlled-relay-board/main/assets/screenshots/web-interface.png)

Each card shows a device's current state and a button to toggle it. Buttons
submit a `POST` request that is only accepted when it originates from the
device's own page (same-origin check), so a relay can't be flipped by a link,
an `<img>` tag, or a cross-site form.

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

| Device | ESP32 GPIO |
|---|---|
| Device 1 | GPIO 4 |
| Device 2 | GPIO 5 |
| Device 3 | GPIO 18 |
| Device 4 | GPIO 19 |
| Device 5 | GPIO 21 |
| Device 6 | GPIO 22 |

Each pin is initialised `LOW` at boot; `ON` drives it `HIGH`. If your relay
module is **active-low** (energises when the input is `LOW`), the ON/OFF sense
will be inverted — either wire to the relay's NC contacts or swap `HIGH`/`LOW`
in `applyAction()`.

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

### 2. Configure your Wi-Fi credentials

Open `esp32_relay_http/esp32_relay_http.ino` and set your network name and
password:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 3. Flash the ESP32

1. Connect the ESP32 over USB and select its port under **Tools → Port**.
2. Click **Upload**.
3. Open **Tools → Serial Monitor** at **115200 baud**. After it connects to
   Wi-Fi, the ESP32 prints its IP address:

   ```
   WiFi connected.
   IP address:
   192.168.1.50
   ```

### 4. Open the interface

Browse to `http://<ESP32-IP>/` (e.g. `http://192.168.1.50/`) from any device on
the same network and use the ON/OFF buttons.

## HTTP API

State changes are performed with a same-origin `POST`; the root page is a `GET`.

| Method | Path | Action |
|---|---|---|
| `GET`  | `/`        | Return the control page. |
| `POST` | `/1/on`    | Turn Device 1 on. |
| `POST` | `/1/off`   | Turn Device 1 off. |
| …      | `/N/on`, `/N/off` | Same for devices `N` = 1–6. |

A `POST` is applied only if its `Origin` (or `Referer`) host matches the `Host`
header. `GET` requests never change relay state, so control from scripts/tools
must send a `POST` with a matching `Origin` header.

## Security notes

This firmware is intended for a **trusted local network**:

- **No authentication.** Anyone who can load the page on your LAN can control the
  relays. Do not expose the device to the internet.
- **Plain HTTP.** Traffic is unencrypted.
- **CSRF protection** is in place (same-origin `POST` only), which prevents other
  websites from toggling your relays, but it is not a substitute for auth.

## License

[Apache License 2.0](LICENSE).
