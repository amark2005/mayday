# MAYDAY ✈️

A terminal-based ADS-B flight tracker. Type a callsign, get live telemetry — altitude, speed, mach, position, vertical rate, aircraft type, registration, and emergency status. Refreshes in the background without blocking the UI.

Built in C++ with ncurses.

> README written with AI assistance. Every line of code is handwritten.

---

## Features

- Live flight data via [adsb.lol](https://api.adsb.lol) public API
- Background fetch thread — UI never freezes
- Emergency status turns red automatically
- `~rx~` indicator shows when a fetch is in progress
- Callsign input on a welcome screen — no hardcoded flights
- Quits on `q`

---

## Dependencies

| Library | Purpose |
|---|---|
| ncurses | TUI rendering |
| OpenSSL | HTTPS for API calls |
| cpp-httplib | HTTP client (header-only, bundled at `src/httplib.h`) |
| nlohmann/json | JSON parsing (header-only) |


---

## Build


```bash
clang++ -O3 -march=native -flto -ffast-math -Wall -Werror main.cpp -o mayday -lssl -lcrypto -lncurses -lpthread
```

Or with GCC:

```bash
g++ -O3 -march=native -flto -ffast-math -Wall -Werror main.cpp -o mayday -lssl -lcrypto -lncurses -lpthread
```

Install dependencies on Arch:

```bash
sudo pacman -S openssl ncurses
```

On Debian/Ubuntu:

```bash
sudo apt install libssl-dev libncurses-dev
```

---

## Run

```bash
./mayday
```

Type the callsign (e.g. `IGO7453`, `AIC101`) and hit Enter. That's it.

---

## Shipping to other machines

MAYDAY is dynamically linked. To ship without asking users to install anything:

```bash
mkdir -p lib
cp /usr/lib/libncurses.so.6 /usr/lib/libssl.so.3 /usr/lib/libcrypto.so.3 lib/
```

Then ship with this launcher:

```bash
#!/bin/bash
DIR="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
exec "$DIR/mayday" "$@"
```

Layout:
```
mayday/
├── mayday
├── run.sh
└── lib/
    ├── libncurses.so.6
    ├── libssl.so.3
    └── libcrypto.so.3
```

---

## Project structure

```
mayday/
├── main.cpp        # TUI, input, draw loop
├── http.cpp        # Plane struct, API fetch, JSON parse
└── src/
    └── httplib.h   # cpp-httplib (header-only)
```

---
