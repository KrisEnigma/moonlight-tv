# Aurora

**Aurora** is a fork of [Moonlight TV](https://github.com/mariotaku/moonlight-tv), the community [Moonlight GameStream Client](https://moonlight-stream.org/) optimized for large screens. It runs on LG webOS TVs (C1–C5 and compatible) and Raspberry Pi with Raspbian.

> **Notice:** This is an unofficial fork. All rights to the original project belong to [mariotaku/moonlight-tv](https://github.com/mariotaku/moonlight-tv) and the Moonlight community. Aurora is provided without warranty.

## Why this fork?

It was created to push the limits of LG C1 (and similar OLEDs) in 4K 120fps HDR streaming. LG’s documentation suggests ~100 Mbps decode capability, but higher bitrates have worked on stable 5 GHz WiFi, with noticeable quality gains over the common 65 Mbps limit.

## Features

- High-performance streaming on webOS (4K 120fps HDR)
- UI optimized for large screens and remote control
- Up to 4 controllers
- Full keyboard overlay
- Compact real-time performance indicator
- **Max bitrate: 300 Mbps** (use sparingly; see warning below)
- **HDR10 (PQ)** over HEVC Main10 when the host sends HDR (HLG, HDR10+, and Dolby Vision are not used)

## ⚠️ Bitrate warning

The bitrate limit is set to **300 Mbps**. High bitrates stress the TV’s WiFi and may:

- Increase chipset temperature
- Raise latency on unstable networks
- Potentially accelerate hardware wear

**Use at your own risk.** Be sensible: start with moderate values (e.g. 80–150 Mbps) and only increase if the network and device are stable. See the reference table below.

## Recommended bitrate table (H.265, near-lossless quality)

| Resolution | 60 fps | 120 fps | 144 fps |
|------------|--------|---------|---------|
| 1080p     | 40–60 Mbps | 80–100 Mbps | 100–120 Mbps |
| 1440p     | 60–80 Mbps | 120–150 Mbps | 150–180 Mbps |
| 4K        | 100–130 Mbps | 180–230 Mbps | 220–280 Mbps |

*Values for H.265 (HEVC), HDR, near-visually-lossless quality. Above ~230 Mbps at 4K 120fps HDR, visual gains tend to be marginal.*

## Fork adjustments

- **300 Mbps** – Raised bitrate limit; use responsibly
- **Video pipeline** – Frame pacing and decoding path aligned with upstream [mariotaku/moonlight-tv](https://github.com/mariotaku/moonlight-tv) (same ss4s submodule baseline)
- **HDR** – Starfish is signaled as **HDR10** only (no HLG / HDR10+ / Dolby Vision paths)

## Documentation

- **[Build and installation (webOS)](docs/BUILD_WEBOS.md)** – Developer mode, build, and manual installation

## Tested (LG C1)

- 4K 120fps HDR, H.265, ~180 Mbps
- Total latency: ~22 ms (network 3–7 ms, decode 8–10 ms, host ~5 ms)
- Controllers tested:
  - Xbox Series S – rumble working
  - 8BitDo Ultimate (Bluetooth, D-Input) – low latency, rumble not supported by TV

## License and credits

- Base project: [mariotaku/moonlight-tv](https://github.com/mariotaku/moonlight-tv)
- Components: [moonlight-embedded](https://github.com/irtimmer/moonlight-embedded) (libgamestream and decoder)
