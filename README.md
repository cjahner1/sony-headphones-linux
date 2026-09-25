# Sony Headphones for Linux

An intentionally native, lightweight Qt 6/C++20 companion for Sony headphones, designed first for **WF-1000XM6**.

## What works in this first foundation

- A polished Qt Quick desktop interface — no Electron or browser runtime.
- BlueZ system D-Bus discovery for paired Sony devices.
- A WF-1000XM6-first dashboard, listening-mode, volume, Speak-to-Chat, and DSEE controls.
- A pinned upstream [`SonyHeadphonesClient`](https://github.com/mos9527/SonyHeadphonesClient) submodule. Its MIT-licensed `libmdr` and `libmdr-bt` already implement Sony MDR over authenticated/encrypted RFCOMM and cover most XM6 features.

## Deliberate safety boundary

The UI will not send speculative proprietary commands. The next implementation milestone is a small Qt adapter around the upstream MDR transport, after compiling it and replay-testing its WF-1000XM6 packet captures. This prevents a polished control from pretending it changed an earbud setting when it did not.

## Build

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/sony-headphones-linux
```

Required packages: Qt 6.4+ (Core, Gui, QML, Quick, Quick Controls 2, DBus), CMake 3.22+, and a C++20 compiler. BlueZ is used at runtime.

## Parity roadmap

The upstream MDR implementation documents working WF-1000XM6 support for battery, volume, noise/ambient mode, sound pressure, voice guidance, media controls, multipoint, Speak-to-Chat, touch gestures, power-off, Capture Voice During Call, and DSEE. Equalizer is the first protocol feature to finish; firmware updates stay out of scope until their update flow is independently validated.
