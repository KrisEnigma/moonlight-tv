# Aurora

Unofficial fork of [Moonlight TV](https://github.com/mariotaku/moonlight-tv) for **LG webOS** (C1–C5 and compatible sets), focused on high-quality streaming on OLED TVs with a remote- and gamepad-friendly UI.

> Rights to the original project belong to [mariotaku/moonlight-tv](https://github.com/mariotaku/moonlight-tv) and the Moonlight community. Provided without warranty.

## Highlights

- **AMOLED layout** — pure black background (`#000000`), dark surfaces, and violet accent; launcher, game grid, and settings popups share the same theme.
- **3.6K resolution (3584×2016)** — option between 2K and 4K; ~90% of 4K pixel area with less load on the TV decoder and lower input lag than native 4K on recent models.
- **HDR10 (PQ)** over HEVC Main10 or AV1 Main10 (when supported).
- Up to **300 Mbps** on the bitrate slider (UI maximum); practical guidance below.

## Recommended settings (LG OLED)

| Setting | Suggestion |
|---------|------------|
| **Resolution** | **3.6K** (3584×2016) — best quality/performance balance for most titles |
| **FPS** | 60 or 120 depending on game and network |
| **Codec** | HEVC (H.265); AV1 if host and decoder expose it |
| **Bitrate** | **Up to ~270 Mbps** on stable 5 GHz Wi‑Fi — reduces micro-stutters and throughput drops on heavy streams (HDR, 120 Hz). Increase gradually; visual gains above that are usually small |

On an unstable network, start at **120–180 Mbps** for 3.6K HDR before going higher.

## Status overlay

### How to open

- **Magic Remote:** **red (RED)** button or **EXIT** key (traditional remote).
- **Gamepad:** press **LB + RB + Back + Start** together, then **release** (opens the streaming menu).
- **Physical keyboard on the host (via stream):** `Ctrl + Alt + Shift + S`.
- During streaming, the app also hints at holding **BACK** (behavior may vary by remote model).

From the menu: **Full keyboard**, virtual mouse, suspend, and quit. The compact stats bar can stay pinned on start (Settings → Basic).

### Gamepad hotkeys (during streaming)

Hold the buttons together, then **release all at once** to trigger (same pattern as opening the menu).

| Combo | Action |
|-------|--------|
| **LB + RB + Back + Start** | Open streaming menu (overlay) |
| **RB + RS** | Open **Full keyboard** directly |
| **LB + RS** | Toggle **Virtual mouse** |
| **LB + LS** | Toggle **pinned performance stats** |

**Xbox layout:** **LB** / **RB** = left / right bumper; **LS** / **RS** = press the left / right analog stick; **Back** = View; **Start** = Menu.

**Virtual mouse** must be enabled first in **Settings → Input → Virtual mouse**. When active:

- **Right stick** — move the cursor
- **Left stick** — scroll the page (vertical and horizontal, with acceleration)
- **LT / RT** — left / right mouse button

These combos work without opening the overlay. Controllers **bridged via HID Passthrough** do not use Moonlight emulation; other paired controllers and these hotkeys still work normally.

### Compact mode (single line)

Example: `3584×2016 HDR H.265 FPS 120 N 2/1ms H 4ms S 1ms D 8ms TL 15ms FD 0.00% 245.0 Mbps`

| Field | Meaning |
|-------|---------|
| **Resolution / HDR / Codec** | Current stream (SDR or HDR, H.264/H.265/AV1) |
| **FPS** | Render rate on the panel (capped to display refresh) |
| **N** | Network RTT (average / variance in ms) |
| **H** | Average host capture latency (ms) |
| **S** | Submit time to the decoder (ms) |
| **D** | Average TV decoder latency only (ms) |
| **TL** | Estimated total latency (N + H + S + D) |
| **FD** | % of frames dropped on the network |
| **Mbps** | Measured video throughput |

**Color dot:** green ≤25 ms, yellow ≤30 ms, red >30 ms (TL).

### Full mode

Same metrics in separate rows: video, audio, RTT, network/render FPS, frame drop, bitrate, host and decoder latency.

## Full keyboard (soft keyboard)

Windows 11 / Xbox OSK layout with alphabetic and `&123` symbol layers, plus F1–F12. Aurora dark theme. Sticky modifiers (Alt then Tab = Alt+Tab).

### How to open

1. **Gamepad:** hold **RB + RS**, then release both — opens the keyboard without the overlay.
2. Open the **streaming overlay** (see above) and select **Full keyboard**.
3. **Magic Remote:** press the **blue (BLUE)** button during streaming.

Keys are sent directly to the PC (**Ctrl**, **Alt**, **Shift**, **Win** are *sticky*: e.g. Alt then Tab = Alt+Tab). **B / Back** closes the keyboard (on webOS remotes it does not send Escape to the host in that case).

### Known issues (to be fixed)

- The remote may send **both key and gamepad events for one press**, toggling input mode while the keyboard is open.
- **Modifier combos** (e.g. Ctrl+Q) can sometimes leave a modifier stuck on the host and affect gamepad input on Windows (Game Bar, etc.) — there is mitigation, but it is not 100% reliable in all cases.
- **TV remote D-pad** on the keyboard: only navigates keys (arrows + OK); other remote keys may close the keyboard or reach the host.
- While the keyboard is open, avoid switching quickly between TV remote and gamepad.

## Virtual mouse

Move the PC cursor from a gamepad while streaming (useful for desktop apps and launchers).

### How to enable

Turn on **Settings → Input → Virtual mouse**, then start or reconnect the stream.

### How to toggle during streaming

- **Gamepad:** hold **LB + RS**, then release both.
- Or open the **streaming overlay** and select **Virtual Mouse**.

When active, use the right stick and triggers as described in [Gamepad hotkeys](#gamepad-hotkeys-during-streaming) above. Press **LB + RS** again to turn it off.

## HID Passthrough (experimental)

Bridge selected controllers to your PC as native HID devices via [CTM-USBIP](https://github.com/CTM-Bridge/CTM-USBIP), while other controllers keep standard Moonlight emulation. Enable in **Settings → Input**, then manage devices from the streaming overlay (**HID Devices**).

- **Auto-Plugin** (per controller): when checked, the controller is bridged automatically at stream start and excluded from Moonlight; when unchecked, use **Plug in** / **Plug out** during streaming.
- You can mix modes: one controller via HID Passthrough and another via Moonlight on the same session.

Full setup (PC agent, supported pads, developer mode): **[webOS build guide — HID Passthrough](docs/BUILD_WEBOS.md#5-hid-passthrough-experimental)**.

### DualSense (DS5) — controller audio

> **Use TV / HDMI audio for game sound.** Controller speaker and 3.5 mm jack over HID Passthrough are experimental and often sound worse than HDMI.
>
> webOS Bluetooth pacing limits DS5 output reports (~100 Hz → ~62 Hz with jitter). Routing game audio to the controller can force **SBC over A2DP** instead of keeping audio on the TV. In the HID panel, leave **Audio output** on **Auto (game decides)** unless you specifically need sound on the pad.
>
> Optional [raw-ACL daemon](https://github.com/sh00bx/webos-ds5-raw-acl) on rooted/dev-mode TVs can improve haptics and controller audio — see the build guide.

## Build and installation

- [webOS Homebrew Channel](https://github.com/webosbrew/webos-homebrew-channel) — add this repo link: `https://raw.githubusercontent.com/GuiDev1994/aurora-tv/main/repo.json`
- [Device Manager app](https://github.com/webosbrew/dev-manager-desktop) — install the latest .ipk directly from [Releases](https://github.com/GuiDev1994/aurora-tv/releases)
- [webOS TV CLI tools](https://webostv.developer.lge.com/develop/tools/cli-installation) — ares-install com.aurora.gamestream_*_arm.ipk (see [webOS build guide](docs/BUILD_WEBOS.md) for CLI setup)

## Credits

- Base: [mariotaku/moonlight-tv](https://github.com/mariotaku/moonlight-tv)
- Components: [moonlight-embedded](https://github.com/irtimmer/moonlight-embedded), [moonlight-common-c](https://github.com/moonlight-stream/moonlight-common-c)
