# dspic33ak-sai-cmsis-driver

CMSIS-Driver **SAI** wrapper for the dsPIC33AK **SPI/I2S/TDM** HAL.

This repository maps the ARM CMSIS-Driver SAI API (`ARM_DRIVER_SAI`,
`Driver_SAI.h`) onto the dsPIC33AK `dspic33ak_spi_i2s_tdm` transport HAL, together
with vendor copies of that HAL and its sibling HALs so the wrapper builds without
external source dependencies.

## Why "SAI" wraps an "SPI/I2S/TDM" HAL

CMSIS-Driver names the **audio serial** driver class *SAI* (Serial Audio
Interface). On dsPIC33AK there is no peripheral literally called "SAI": I2S/TDM
audio is carried over the **SPI peripheral in framed mode** (`AUDEN=0`,
`FRMEN=1`). So:

- The **CMSIS driver class** this repository implements is **SAI** — hence the
  repository name and `Driver_SAI0`, following the convention that a CMSIS-Driver
  repository is named after its CMSIS class (`...-i2c-cmsis-driver`,
  `...-usart-cmsis-driver`, `...-can-cmsis-driver`, this `...-sai-cmsis-driver`).
- The **underlying dsPIC33AK HAL** is the framed-SPI I2S/TDM transport
  [dspic33ak-spi-i2s-tdm-hal](https://github.com/sulaolab/dspic33ak-spi-i2s-tdm-hal).

In short: **SAI is the portable API; SPI/I2S/TDM is the dsPIC33AK transport that
realises it.** All `ARM_SAI_*` / `ARM_DRIVER_*` types live in the wrapper, never
in the HAL.

## Layout

```text
cmsis_driver/
  Driver_SAI_dsPIC33AK.c / .h           the SAI wrapper (Driver_SAI0)
  RTE_Device_SAI_dsPIC33AK_example.h    example RTE configuration
  README.md                             wrapper API / envelope / usage
src/
  hal_spi_i2s_tdm/                       vendored SPI/I2S/TDM transport HAL
    dspic33ak_spi_i2s_tdm_conf.h         default config (single SPI1 TDM8; compiled)
    dspic33ak_spi_i2s_tdm_conf.h_example annotated bring-your-own template
    UPSTREAM.md
  hal_dma/                               vendored DMA HAL (required sibling)
    UPSTREAM.md
  hal_timer/                             vendored Timer2 high-res counter (load monitor)
    UPSTREAM.md
third_party/arm_cmsis_driver/            minimal ARM CMSIS-Driver headers (Apache-2.0)
  Include/Driver_Common.h, Include/Driver_SAI.h, LICENSE.txt, README.md
tools/
  sync_hal_from_upstream.py              re-vendor the three HALs from their upstreams
```

## Vendored HALs

The HAL-only repositories are the upstream sources of truth; this repository
vendors validated snapshots (each `src/hal_*/UPSTREAM.md` records the exact
commit). Apply HAL changes upstream first, then re-sync with
`tools/sync_hal_from_upstream.py`. CMSIS-Driver SAI wrapper changes belong here.

| Vendored under | Upstream | Role |
|---|---|---|
| `src/hal_spi_i2s_tdm/` | [dspic33ak-spi-i2s-tdm-hal](https://github.com/sulaolab/dspic33ak-spi-i2s-tdm-hal) | the transport the wrapper maps onto |
| `src/hal_dma/` | [dspic33ak-dma-hal](https://github.com/sulaolab/dspic33ak-dma-hal) | DMA channel setup (required) |
| `src/hal_timer/` | [dspic33ak-timer-hal](https://github.com/sulaolab/dspic33ak-timer-hal) | Timer2 high-res counter for the load monitor (compile/link sibling, runtime-gated) |

## Integration

1. Add to the build: `cmsis_driver/Driver_SAI_dsPIC33AK.c`, the vendored HAL
   `src/hal_*/**.c`, and put `cmsis_driver/`, each `src/hal_*/`,
   `third_party/arm_cmsis_driver/Include/` on the include path.
2. Provide the HAL config: edit `src/hal_spi_i2s_tdm/dspic33ak_spi_i2s_tdm_conf.h`
   (or replace it from `*_conf.h_example`) for your board / topology.
3. **Override the two weak integration hooks** (declared in
   `Driver_SAI_dsPIC33AK.h`) — the wrapper is board-/app-independent:
   - `Driver_SAI_dsPIC33AK_GetDefaultConfig(cfg)` — seed the board-electrical
     config the wrapper applies on `Control(CONFIGURE_*)`. The weak default
     returns `false`, so a stream cannot start until you provide this.
   - `Driver_SAI_dsPIC33AK_IsSampleRateSupported(hz)` — your product's
     supported-rate allow-list for `ARM_SAI_AUDIO_FREQ`. The weak default accepts
     only `RTE_SAI0_DEFAULT_SAMPLE_RATE_HZ`.
   (Same pattern as the I2C wrapper's `Driver_I2C_dsPIC33AK_GetMs()`.)
4. Register the board/clock port with `dspic33ak_spi_i2s_tdm_set_port()` **before**
   `Control(CONFIGURE_*)` / `Control(CONTROL_TX/RX)` if your board needs PPS pin
   routing, CLC pass-through, or external-clock readiness gating (board-specific;
   the wrapper does not register it for you).
5. Own the DMA RX interrupt vector as the HAL documents (turnkey by default; see
   the HAL's `DSPIC33AK_TDM_DEFINE_DMA_VECTORS`).

See `cmsis_driver/README.md` for the validated envelope, the Send/Receive copy
layer, and usage.

## Status

Verified live (full-duplex I2S/TDM loopback) on **dsPIC33AK512MPS512** in the
Perseus integration project. The wrapper advertises and accepts **only** the
HAL's validated envelope: dsPIC33AK SPI **slave**, external BCLK/FS, external-input
or inactive MCLK, 32-bit
word/slot, and **either** I2S (2 slots) **or** TDM8 (8 slots) — whichever matches
the compiled HAL geometry (`DSPIC33AK_TDM_SLOTS_PER_FS`); a single build realises
one protocol, and `GetCapabilities`/`Control` advertise/accept exactly that one.
Master clock, `DATA_SIZE` 16/24, justified/PCM/AC97, mono/companding, slot
offset/mask, `FRAME_ERROR`, dynamic sample-rate switching, and true independent
per-direction TX/RX are not implemented (they mirror the HAL's validated
envelope).

## License

MIT No Attribution License (MIT-0). See [LICENSE](LICENSE). The vendored ARM
CMSIS-Driver headers under `third_party/` are Apache-2.0 (see that directory's
LICENSE.txt); their original SPDX headers are kept intact.

Attribution is appreciated but not required.
