# Hot Pursuit NX

Hot Pursuit NX is a port of nfs-recompiled: A static recompilation of **Need for Speed II: Special Edition** and **Need for Speed III: Hot Pursuit**.

<a href="screenshots/nfsiise-1.png"><img src="screenshots/nfsiise-1.png" alt="nfs2 intro video" width="50%"></a>

<a href="screenshots/nfsiise-2.png"><img src="screenshots/nfsiise-2.png" alt="nfs2 menu" width="50%"></a>

<a href="screenshots/nfsiise-3.png"><img src="screenshots/nfsiise-3.png" alt="nfs2 gameplay" width="50%"></a>

<a href="screenshots/nfsiiihp-1.png"><img src="screenshots/nfsiiihp-1.png" alt="nfs3 menu" width="50%"></a>

<a href="screenshots/nfsiiihp-2.png"><img src="screenshots/nfsiiihp-2.png" alt="nfs3 gameplay" width="50%"></a>

## Setup

### 1. Game Data
You need the original game files.
1.  **Install the game** from your original CD (or mount the iso).
    On Linux, install the game from the CD-ROM through Wine, e.g. `WINEARCH=win32 WINEPREFIX=/opt/win98 wine /mnt/AUTORUN.EXE`
    Note that Wine's WOW64 mode will not work for this as both games are 16bit executables.
    I found that a Windows 7 vm was the easiest way to extract the files.
2.  **Copy CD data**: The game expects certain files to be on the CD. To run without the CD, copy the `Fedata` and `GameData` folders from the CD-ROM into your game installation directory (merging with existing folders).

### 2. Running

Both nfs2se.nro and nfs3hp.nro are provided on the releases page.
Place nfs2se.nro into `sdmc:/switch/nfs2se/` and nfs3hp.nro into  `sdmc:/switch/nfs3hp/`.

Also, be sure to place your extracted game data (see step 1) in `sdmc:/switch/nfs2se/`
and `sdmc:/switch/nfs3hp/` for their respective games. (The FeData and Gamedata
folders should be beside the nro in that folder).

## Controls

The Switch controls are mapped to the original keyboard commands used by both games.

| Switch control | Action |
| --- | --- |
| Left stick / D-pad | Steer |
| ZR | Accelerate |
| ZL | Brake/reverse |
| A | Handbrake |
| B | Look behind |
| X | Cycle camera |
| Y | Horn/siren |
| L | Back/pause pause |
| R | Confirm |
| Plus | Shift up |
| Minus | Shift down |
| Left stick button | Reset car |
| Right stick button | Lay spike strip (only nfs3) |

## Building from source

### Prerequisites
- CMake ≥ 3.16
- DevkitPro
- SDL3 development libraries (native builds only)
- 
**Debian / Ubuntu:**
```bash
sudo apt install build-essential cmake nasm python3 python3-pip \
               libsdl3-dev libgl-dev
```

### Build Instructions

Install CMake, Git, nxvk (with gl support), and the devkitPro Nintendo Switch toolchain. The devkitPro
installation must include devkitA64, libnx, switch-tools.

Configure and build both NROs with:

```bash
cmake -B build-switch \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-switch.cmake \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build-switch --parallel "$(nproc)"
```
