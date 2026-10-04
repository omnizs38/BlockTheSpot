<div align="center">

# BlockTheSpot

**Spotify ad blocking for Windows, with safer hooks and compatibility checks.**

[![Latest release](https://img.shields.io/github/v/release/omnizs38/BlockTheSpot?label=latest%20build&color=1DB954)](https://github.com/omnizs38/BlockTheSpot/releases/latest)
[![Build and Verify](https://github.com/omnizs38/BlockTheSpot/actions/workflows/ci.yml/badge.svg)](https://github.com/omnizs38/BlockTheSpot/actions/workflows/ci.yml)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue)

</div>

A maintained fork of [BlockTheSpot](https://github.com/mrpond/BlockTheSpot), built on [Nuzair46's continuation](https://github.com/Nuzair46/BlockTheSpot). It patches your existing standalone Spotify client rather than downloading a replacement from a third-party mirror.

- **More than one interception path.** Hooks CEF's ordinary and delayed imports by name, plus dynamic `GetProcAddress` lookups, including Windows API-set imports used by newer Spotify versions.
- **Layered ad blocking.** Blocks audio-ad, ad-logic, Gabo and sponsored-playlist requests, forces Spotify's own `adsEnabled` UI state off, and removes the leaderboard banner. URL matching is ASCII case-insensitive so casing changes do not bypass a rule.
- **Defensive patching.** Checks CEF object sizes, function-pointer ranges and memory protections. JavaScript patches need unique matches and bounded writes; paired edits are validated before either is applied.
- **Safer installation and loading.** Downloads are pinned to one release and checked against GitHub's SHA-256 digests before Spotify is stopped. Failed replacements roll back, backups are signature/version checked, and DLL/config paths follow the installation rather than the working directory.
- **Checks that exercise the hooks.** Branch CI and the daily watcher test installation failures, native hooks and loader argument forwarding, validate JS patches, then install/reinstall, launch and uninstall the official client on a disposable Windows runner.
- **Honest failure reporting.** Failed verification blocks publication and the watcher opens an issue. Startup diagnostics distinguish prepared handlers, installed imports and observed callbacks.

**Limits:** CEF offsets are known defaults with a version-override mechanism, not automatic ABI discovery. Guards cannot prove that a valid function pointer still identifies the expected method. The logged-out startup smoke test does **not** establish ad-free playback or signed-in UI behavior. No fork can guarantee compatibility with every future Spotify update. See [compatibility and testing](docs/compatibility.md).

## Install

Requires the **standalone Windows x64 client**, not the Microsoft Store version. In PowerShell:

```powershell
iwr -useb https://raw.githubusercontent.com/omnizs38/BlockTheSpot/master/install.ps1 | iex
```

[Read the installer first](install.ps1). It stages one release, verifies hashes and x64 DLL headers, then stops only this Spotify installation. It checks that the original `chrome_elf.dll` is signed by Spotify and matches the installed Chromium version before replacing anything. Each replacement is atomic; ordinary failures restore the prior files. Existing settings are saved as `config.ini.previous` rather than silently discarded.

A Spotify update can replace the loader; re-run the installer if necessary. A missing or stale genuine backup requires repairing Spotify first. Hash verification protects against incomplete/mixed downloads; it is not independent authentication of a compromised GitHub release.

### Manual install

1. Close Spotify and open `%AppData%\Spotify`.
2. Rename the **genuine** `chrome_elf.dll` to `chrome_elf_required.dll`. Do not overwrite a genuine backup with an already-installed BlockTheSpot loader.
3. Download `chrome_elf.dll`, `blockthespot.dll` and `config.ini` from the [latest release](https://github.com/omnizs38/BlockTheSpot/releases/latest) into that folder.
4. Launch Spotify.

### Uninstall

```powershell
iwr -useb https://raw.githubusercontent.com/omnizs38/BlockTheSpot/master/uninstall.ps1 | iex
```

Uninstall restores and verifies the genuine DLL **before** removing the payload. It leaves the backup and `config.ini.previous` for recovery, and refuses to remove anything if neither the current DLL nor backup is a compatible genuine copy.

**Interrupted installations:** caught replacement errors roll back. If rollback itself fails, the error identifies a `.blockthespot-stage-*` recovery folder containing `previous/` snapshots and `prior-state.json`. An interrupted script can leave the same recovery folder even without an error message. Keep it, close Spotify and restore the previous files (or repair the official client) before relaunching. A forced termination or power loss can interrupt a multi-file update; this is not a power-loss-atomic transaction.

## How it works

- **`chrome_elf.dll` (loader)** forwards Chromium exports to the original signed DLL, renamed `chrome_elf_required.dll`, and loads the payload. It preserves Windows x64 integer, floating-point and stack arguments and resolves exports without a shared mutable cache.
- **`blockthespot.dll` (payload)** intercepts CEF URL requests and SPA reads. It redirects Spotify's signature check to the original DLL rather than modifying Spotify's signed binaries on disk.
- **URL rules** block scoped ad request paths, including `/ads/`, `/ad-logic/`, `/ad/v1/`, Gabo receiver traffic and sponsored-playlist inventory. The default config also blocks `/desktop-update/`; remove that rule if you want Spotify's update requests to pass through. It deliberately does not block whole Spotify hosts or media-manifest paths, which can break normal playback.
- **SPA patches** force the client's own `adsEnabled` flag off and hide the leaderboard banner using length-preserving edits. Each signature must match exactly once or the file is left unchanged.

The request and UI layers are independent defenses, but **both depend on working native interception**. A matching JS signature alone is not proof that ad blocking works. The project does not spoof Premium subscription state.

## Troubleshooting

Information-level logging is enabled by default in `config.ini` (`[Log] Level=1`). Look in `%AppData%\Spotify\blockthespot.log` after launching:

- `CEF hook installation: OK` — the required interception paths were installed, not proof of playback behavior.
- `CEF URL handler active (URL decoded)` and `CEF ZIP read callback observed (filename decoded)` — actual callbacks executed successfully.
- `URL block rule N active` — that configured rule blocked its first request; full URLs remain hidden unless debug logging is enabled.
- `FAILED/PARTIAL` or `SPA patch skipped` — compatibility needs investigation; do not treat a successful launch as a successful ad block.

Paths are anchored to the DLL directory and support Unicode profile names; launching from a different working directory does not move the config or log. The payload is pinned for the process lifetime, so uninstall requires stopping Spotify rather than unloading live hooks.

The log is reset on startup. `Level=0` disables it. `Level=2` includes request URLs: **do not post debug logs publicly without redacting tokens and personal information**. When reporting a problem, include the Spotify version, the release/commit installed, symptoms, and the information-level log.

## Development

Cheap cross-platform checks:

```sh
python -m unittest discover -s tests -p 'test_*.py' -v
python tools/verify_patches.py config.ini <extracted-xpui-directory>
```

Windows builds/tests use `tools/build-and-test.ps1` in an x64 MSVC developer shell. GitHub Actions runs this automatically on branches and uploads DLLs plus verification reports. The installer and smoke-test scripts under `tools/` are restricted to disposable GitHub-hosted runners and must not be used against a personal installation.

## Credits

- [**thomas-quant/BlockTheSpot-Resilient**](https://github.com/thomas-quant/BlockTheSpot-Resilient): resilience, installer safety, compatibility automation, and real-client smoke tests used by this fork.
- [**mrpond/BlockTheSpot**](https://github.com/mrpond/BlockTheSpot): original project.
- [**Nuzair46/BlockTheSpot**](https://github.com/Nuzair46/BlockTheSpot): continuation and injection core.
- [**MichaelMiksa**](https://github.com/MichaelMiksa/BlockTheSpot---continued): identified the hook failure and contributed the by-name CEF delay-import approach adapted here, with co-author credit in the fixing commit. See [issue #1](https://github.com/omnizs38/BlockTheSpot/issues/1) and his [original implementation](https://github.com/MichaelMiksa/BlockTheSpot---continued/commit/7725515).

## Disclaimer

For educational use. Modifying the Spotify client may violate Spotify's Terms of Service. Consider [Spotify Premium](https://www.spotify.com/premium/). Use at your own risk.
