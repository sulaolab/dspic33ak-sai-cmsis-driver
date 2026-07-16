# CMSIS-Driver SAI wrapper for the dsPIC33AK SPI/I2S/TDM HAL

`Driver_SAI_dsPIC33AK.{c,h}` maps the ARM CMSIS-Driver SAI API (`ARM_DRIVER_SAI`,
`Driver_SAI.h`) onto the `dspic33ak_spi_i2s_tdm` transport HAL. All `ARM_SAI_*` /
`ARM_DRIVER_*` types live in the wrapper, never in the HAL.

Prerequisite: the official ARM `Driver_SAI.h` (Apache-2.0, API v1.2), vendored at
`third_party/arm_cmsis_driver/Include/Driver_SAI.h`.

## Instance mapping

| CMSIS driver object | dsPIC33AK HAL |
|---|---|
| `Driver_SAI0` | the single `dspic33ak_spi_i2s_tdm` transport (SPI1 = the primary leg) |

## Validated envelope

`GetCapabilities` and the `Control` parser advertise/accept **only** what the HAL
validates (mirrors `tdm_config_is_supported()`):

- dsPIC33AK SPI **slave**, external BCLK / FS / MCLK
- 32-bit word and slot
- `ARM_SAI_PROTOCOL_I2S` (2 slots) **or** `ARM_SAI_PROTOCOL_USER` = TDM8 (8 slots),
  **whichever matches the compiled HAL geometry** (`DSPIC33AK_TDM_SLOTS_PER_FS`:
  2 → I2S, 8 → TDM8). Only that one protocol is advertised by `GetCapabilities`
  and accepted by `Control`; the other returns `ARM_SAI_ERROR_PROTOCOL`. The HAL's
  DMA ping-pong geometry is static per build, so a single binary realises one
  protocol — rebuild with the other `slots_per_fs` to switch.
- MSB-first, default clock polarity, asynchronous (standalone)

`ARM_SAI_AUDIO_FREQ` (arg2) is checked against the integrator rate hook (see
below); it is **not** passed to the rate-agnostic HAL core.

## Implemented

- `GetVersion`, `GetCapabilities`
- `Initialize`, `Uninitialize`
- `PowerControl(ARM_POWER_FULL/OFF)` — logical power + explicit block-callback
  registration on the SPI1 instance; does not start the stream. FULL also performs the
  DMA HAL's one-time `dspic33ak_dma_global_init()` (idempotent), so the integrator does
  NOT need a separate DMA-init startup step.
- `Control`:
  - `ARM_SAI_CONFIGURE_TX/RX` — applies protocol/slot count via the HAL
    `inst_configure()` **while stopped**, seeded from the integrator's default config
    (see hooks). Settings outside the validated envelope are rejected with the
    matching `ARM_SAI_ERROR_*` rather than silently ignored. **`CONFIGURE_TX` and
    `CONFIGURE_RX` configure the same full-duplex transport** (no independent
    per-direction config): call either, or call both with identical parameters; a
    differing second configure just re-applies (last wins).
  - `ARM_SAI_CONTROL_TX/RX` (arg1 bit0 = enable) — whole-stream, full-duplex
    `open`/`start` and `stop`/`close`. Higher `arg1` bits (bit1 = mute, and any
    undefined bits) are rejected with `ARM_DRIVER_ERROR_UNSUPPORTED` — the transport
    HAL has no codec mute. While a direction is enabled, a block with no armed
    `Send`/`Receive` buffer raises the sticky `tx_underflow`/`rx_overflow` and fires
    `ARM_SAI_EVENT_TX_UNDERFLOW`/`_RX_OVERFLOW` once (cleared by the next
    `Send`/`Receive`).
  - `ARM_SAI_ABORT_SEND/RECEIVE` — cancels the wrapper's copy-layer transfer.
- `Send` / `Receive` copy layer; `GetTxCount` / `GetRxCount`; `GetStatus`
  (`tx_busy`/`rx_busy` from the active transfer; `tx_underflow`/`rx_overflow`
  sticky).

## Send / Receive copy layer

The HAL is zero-copy (its block callback hands pointers into the RX/TX ping-pong
halves). CMSIS `Send`/`Receive` caller-buffer semantics are realised by a
per-block `memcpy` in the wrapper's block bridge, which runs in the RX-block (DMA)
ISR context — keep callbacks short and watch
`dspic33ak_spi_i2s_tdm_get_status().load`.

- **Block-aligned only.** `num` must be a whole multiple of `block_samples`
  (`block_frames * slots_per_fs`); otherwise `Send`/`Receive` returns
  `ARM_DRIVER_ERROR_PARAMETER`.
- **No deep copy at `Send()` time.** The bridge reads `data` progressively, one
  ping-pong block per DMA tick, so the caller buffer must stay valid and
  unmodified until `ARM_SAI_EVENT_SEND_COMPLETE`. A continuous stream must
  double-buffer: refill a buffer only after its `SEND_COMPLETE`, never the one
  still in flight.

## Integration hooks (override in your project)

The wrapper is board-/app-independent. Two `__attribute__((weak))` hooks (declared
in `Driver_SAI_dsPIC33AK.h`) carry the board/app concerns; provide strong
definitions (same pattern as the I2C wrapper's `Driver_I2C_dsPIC33AK_GetMs()`):

```c
/* Seed the board-electrical config the wrapper applies on Control(CONFIGURE_*).
 * Weak default returns false -> a stream cannot start until you provide this. */
bool Driver_SAI_dsPIC33AK_GetDefaultConfig(dspic33ak_spi_i2s_tdm_config_t *cfg);

/* Your product's supported-rate allow-list for ARM_SAI_AUDIO_FREQ.
 * Weak default accepts only RTE_SAI0_DEFAULT_SAMPLE_RATE_HZ. */
bool Driver_SAI_dsPIC33AK_IsSampleRateSupported(uint32_t hz);
```

## Configuration

`RTE_Device_SAI_dsPIC33AK_example.h` enables `Driver_SAI0` (`RTE_SAI0 1`) and declares the
default sample rate (`RTE_SAI0_DEFAULT_SAMPLE_RATE_HZ`, used by the weak
`IsSampleRateSupported()` hook). The protocol (I2S vs TDM8) is NOT selected in RTE — it is
fixed by the compiled HAL geometry (`DSPIC33AK_TDM_SLOTS_PER_FS`: 2 => I2S, 8 => TDM8), and
the wrapper advertises/accepts exactly that one protocol. Board-electrical fields and block
geometry come from the integrator's default config (the `GetDefaultConfig` hook), not from
RTE. Copy the needed definitions into your application's `RTE_Device.h`.

## Board port

The transport HAL reaches board pin/PPS routing, CLC pass-through, and external
clock readiness only through a registered port
(`dspic33ak_spi_i2s_tdm_set_port()`). If your board needs any of these, register
the port **before** `Control(CONFIGURE_*)` / `Control(CONTROL_TX/RX)` — the
wrapper does not register it for you (it is board-specific). See the HAL README.

## A note on `ARM_SAI_MCLK_PIN`

This wrapper is slave-only: it accepts `ARM_SAI_MCLK_PIN_INPUT` or
`ARM_SAI_MCLK_PIN_INACTIVE` and rejects MCLK output (master-mode). CMSIS marks
`MCLK_PIN_INPUT` as "master only"; here it is interpreted as **"an external MCLK
is present"** — the natural dsPIC33AK slave case (external BCLK/FS/MCLK). It does
not make the wrapper a master.

## Unsupported (returns `ARM_DRIVER_ERROR_UNSUPPORTED` / specific error)

Master mode / MCLK output / prescaler, `DATA_SIZE` 16/24, justified/PCM/AC97
protocols, mono/companding, slot offset, `MASK_SLOTS_TX/RX`, `FRAME_ERROR`,
dynamic sample-rate switching, and true independent per-direction TX/RX. These
mirror the HAL's validated envelope.

## Status

Verified live (full-duplex I2S/TDM loopback) on dsPIC33AK512MPS512 in the Perseus
integration project. Build-checked standalone with the vendored HALs and the
default `dspic33ak_spi_i2s_tdm_conf.h`.
