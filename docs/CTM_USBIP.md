# CTM-USBIP (Windows Host)

CTM-USBIP is vendored as a git submodule at [`CTM-USBIP/`](../CTM-USBIP/). It receives HID reports from Aurora on the TV and presents a virtual USB gamepad on Windows via [usbip-win2](https://github.com/vadimgrn/usbip-win2).

## Prerequisites

- Windows 10/11 x64
- Visual Studio 2022 with **Desktop development with C++**
- usbip-win2 driver (bundled in CTM-Bridge-Setup or installed manually)
- **vcpkg** (one-time): clone into `CTM-USBIP/third_party/vcpkg` and install static Opus:

```powershell
cd CTM-USBIP
git clone --depth 1 https://github.com/microsoft/vcpkg third_party/vcpkg
.\third_party\vcpkg\bootstrap-vcpkg.bat -disableMetrics
.\third_party\vcpkg\vcpkg.exe install opus:x64-windows-static --x-install-root=third_party\vcpkg\installed
```

## Build (local)

From the repository root:

```powershell
.\CTM-USBIP\build.ps1 -Configuration Release
```

Output: `CTM-USBIP\out\x64\Release\ctm-usbip.exe` plus `maps\` and `profiles\`.

Optional helper that also copies artifacts to `dist/CTM-USBIP/`:

```powershell
.\scripts\windows\build_ctm_usbip.ps1 -Configuration Release
```

## Run the agent

```powershell
.\CTM-USBIP\out\x64\Release\ctm-usbip.exe agent 48054
```

Or install **CTM-Bridge-Setup.exe** from [CTM-USBIP releases](https://github.com/CTM-Bridge/CTM-USBIP/releases) for a Windows service on the same port.

## Flydigi Apex 4

- TV bridge kind: `flydigi` (composite passthrough + `CTMB_MSG_ENUM`)
- Host map: `maps/flydigi_apex4_identity.map`
- Fallback profile: `profiles/descriptors/flydigi_apex4_usb.profile`
- Notes: [CTM-USBIP/docs/flydigi_apex4_knowledge.md](../CTM-USBIP/docs/flydigi_apex4_knowledge.md)

## Submodule init

```bash
git submodule update --init CTM-USBIP
```

The webOS Docker build initializes submodules but does **not** compile CTM-USBIP (Windows-only).
