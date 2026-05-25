# firmware

STM32WB55-based firmware for the Smart Barbell Clip. Targets the
P-NUCLEO-WB55.USBDongle (MB1293) during prototype work and a custom PCB later.
Built with CMake + Ninja and the GNU Arm Embedded toolchain. STM32CubeMX is
used as a pin-mux / clock-tree / peripheral-init generator (per Phase 5 of the
Project Plan); its output (`Core/`, `STM32_WPAN/`, `clip-fw.ioc`) lives in the
repo so the build is reproducible from `git clone` alone.

## Set-up

This walks through getting from a freshly cloned repo to a working build.

### What's tracked vs what isn't

`.gitignore` excludes a few large or regeneratable trees that the rest of the
build still depends on. After `git pull`, you have:

**Tracked (already present in the repo):**

- `firmware/CMakeLists.txt`, `cmake/` toolchain + per-board files
- `firmware/linker/STM32WB55CGUx_FLASH.ld` (CubeMX-generated linker script
  with the RAM1 / RAM2 / RAM_SHARED layout for FUS + BLE stack co-residence)
- `firmware/Core/Inc/`, `firmware/Core/Src/`, `firmware/Core/Startup/` —
  CubeMX-generated HAL wrappers, peripheral init, and the
  `startup_stm32wb55xx_cm4.s` vector table
- `firmware/STM32_WPAN/App/`, `firmware/STM32_WPAN/Target/` — BLE skeleton
  (GAP / GATT / custom service templates)
- `firmware/clip-fw.ioc` — the CubeMX project file. Single source of truth
  for pinout / clock / peripheral choices.
- `firmware/src/`, `firmware/include/` — hand-written application code
- `firmware/.vscode/tasks.json`, `launch.json`, `extensions.json`,
  `settings.json` — one-key Build / Flash / Debug for VS Code

**Excluded (you need to fetch or generate):**

- `firmware/third_party/cube-wb/` — the STM32CubeWB vendor tree
  (~400 MB after recursive submodules). Too big to commit; reproduced from
  one clone command. Provides CMSIS headers, the HAL drivers, the STM32_WPAN
  middleware, and the signed FUS / BLE coprocessor binaries.
- `firmware/build/` — CMake build artifacts. Created at configure time.
- `firmware/Drivers/` — never copied out of the CubeMX scratch folder. HAL /
  CMSIS sources are referenced from `third_party/cube-wb/` instead. The
  `.gitignore` rule is defense in depth in case CubeMX accidentally drops
  them in.
- `firmware/Makefile`, `firmware/Debug/`, `firmware/Release/` — CubeMX
  emits these in Makefile mode; we use CMake and ignore them.
- `firmware/*.ioc_bak` — CubeMX backup files. The canonical `.ioc` lives
  next to them.

### Prerequisites

These should already be installed from Phase 1.7 of the Project Plan. To
confirm:

- `arm-none-eabi-gcc` 13+ on `PATH` (use Arm's official tarball, not `apt`)
- `cmake` 3.22+
- `ninja-build`
- `openocd`
- STM32CubeProgrammer — required for FUS + BLE coprocessor flashing on the
  WB series (OpenOCD cannot do this; the binaries are signed by ST)
- STM32CubeMX — only needed if you intend to regenerate from `clip-fw.ioc`

### Step 1 — Clone the repo

From wherever your projects live:

```bash
git clone <repo-url> smart-barbell-clip
cd smart-barbell-clip
```

### Step 2 — Pull STM32CubeWB into `third_party/`

CubeWB is a meta-repository. Most of its real contents (CMSIS device headers,
HAL drivers, STM32_WPAN middleware, wireless binaries) are hosted as Git
submodules. A plain `git clone` leaves those folders empty — the clone must
use `--recurse-submodules`.

```bash
cd firmware/third_party
git clone --recurse-submodules --shallow-submodules --depth 1 \
    https://github.com/STMicroelectronics/STM32CubeWB.git cube-wb
cd ../..
```

If you already cloned without `--recurse-submodules` and
`cube-wb/Drivers/CMSIS/Device/ST/STM32WBxx/Source` looks empty, run the
submodule update from inside `cube-wb/`:

```bash
cd firmware/third_party/cube-wb
git submodule update --init --recursive
cd ../../..
```

### Step 3 — (Optional) Regenerate from `clip-fw.ioc`

Only needed if you've edited the `.ioc` (added a peripheral, moved a pin,
bumped a BLE config value), or if you want to sanity-check the committed
`Core/` / `STM32_WPAN/` against a fresh regen. Per Phase 5.6, CubeMX generates
into a scratch folder **outside** the repo, and you copy two trees back in.

1. Open `firmware/clip-fw.ioc` in STM32CubeMX.
2. In Project Manager: Project Location = `~/cubemx-scratch` (NOT the repo);
   Toolchain / IDE = Makefile; Application Structure = Advanced; Code
   Generator tab = tick "Generate peripheral initialization as a pair of
   `.c/.h` files per peripheral".
3. Click Project → Generate Code.
4. Copy `Core/` and `STM32_WPAN/` from the scratch folder back into
   `firmware/`. Do NOT copy `Drivers/` (we use the cube-wb copy) and do NOT
   copy the generated `Makefile` (we use CMake).
5. If CubeMX wrote a new `STM32WB55*.ld`, replace `firmware/linker/`'s copy
   with it.

On a fresh clone with the committed `Core/` / `STM32_WPAN/` intact, skip this
step entirely.

### Step 4 — Configure and build

From `firmware/`:

```bash
cmake -S . -B build -G Ninja -DBOARD=MB1293 -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Output lands in `build/`: `smart-barbell-clip-fw.elf`, `.bin`, `.hex`, and a
`.map` file. From here, flashing is either OpenOCD (`Flash (OpenOCD)` task in
VS Code, or the `openocd ... -c "program ..."` command line) or USB DFU via
STM32CubeProgrammer.

If you change `-DBOARD=` later, delete `build/` first — CMakeCache pins the
board choice on the first configure.

### Step 5 — (One-time per chip) FUS + BLE coprocessor

The WB55's CPU2 (Cortex-M0+) needs ST's signed BLE stack flashed onto it
before any application BLE code will work. Only STM32CubeProgrammer can do
this — OpenOCD cannot flash the radio core. Binaries live at
`firmware/third_party/cube-wb/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x/`.
Follow Phase 5.8 of the Project Plan for the GUI walkthrough and CLI
equivalent. This is a one-time operation per chip, not part of every build.

### Quick copy-paste setup

Run from the cloned repo root, with the prerequisites above already
installed:

```bash
# 1. Pull the STM32CubeWB vendor tree (excluded from the repo)
git clone --recurse-submodules --shallow-submodules --depth 1 \
    https://github.com/STMicroelectronics/STM32CubeWB.git \
    firmware/third_party/cube-wb

# 2. Configure and build
cd firmware
cmake -S . -B build -G Ninja -DBOARD=MB1293 -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

A successful run leaves `firmware/build/smart-barbell-clip-fw.elf` ready to
flash.
