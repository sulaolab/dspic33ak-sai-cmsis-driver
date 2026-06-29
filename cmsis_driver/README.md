# CMSIS-Driver SAI wrapper for the dsPIC33AK SPI/I2S/TDM HAL

`Driver_SAI_dsPIC33AK.{c,h}` maps the ARM CMSIS-Driver SAI API (`ARM_DRIVER_SAI`,
`Driver_SAI.h`) onto the `dspic33ak_spi_i2s_tdm` transport HAL. All `ARM_SAI_*` /
`ARM_DRIVER_*` types live in the wrapper, never in the HAL.

Prerequisite: the official ARM `Driver_SAI.h` (Apache-2.0, API v1.2), vendored at
`third_party/arm_cmsis_driver/Include/Driver_SAI.h`.

## Instance mapping

| CMSIS driver object | dsPIC33AK HAL |
|---|---|
| `Driver_SAI0` | the single `dspic33ak_spi_i2s_tdm` transport (SPI1 RX = block-timing reference) |

## Validated envelope

`GetCapabilities` and the `Control` parser advertise/accept **only** what the HAL
validates (mirrors `tdm_config_is_supported()`):

- dsPIC33AK SPI **slave**, external BCLK / FS / MCLK
- 32-bit word and slot
- `ARM_SAI_PROTOCOL_I2S` (2 slots) or `ARM_SAI_PROTOCOL_USER` = TDM8 (8 slots)
- MSB-first, default clock polarity, asynchronous (standalone)

`ARM_SAI_AUDIO_FREQ` (arg2) is checked against the integrator rate hook (see
below); it is **not** passed to the rate-agnostic HAL core.

## Implemented

- `GetVersion`, `GetCapabilities`
- `Initialize`, `Uninitialize`
- `PowerControl(ARM_POWER_FULL/OFF)` — logical power + explicit block-callback
  registration on the SPI1 instance; does not start the stream
- `Control`:
  - `ARM_SAI_CONFIGURE_TX/RX` — applies protocol/slot count via the HAL
    `configure()` **while stopped**, seeded from the integrator's default config
    (see hooks). Settings outside the validated envelope are rejected with the
    matching `ARM_SAI_ERROR_*` rather than silently ignored.
  - `ARM_SAI_CONTROL_TX/RX` (arg1 bit0 = enable) — whole-stream, full-duplex
    `open`/`start` and `stop`/`close`.
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

`RTE_Device_SAI_dsPIC33AK_example.h` enables `Driver_SAI0` (`RTE_SAI0 1`) and
declares the default high-level attributes (`RTE_SAI0_DEFAULT_PROTOCOL_I2S`,
`RTE_SAI0_DEFAULT_SAMPLE_RATE_HZ`). Board-electrical fields and block geometry
come from the integrator's default config (the `GetDefaultConfig` hook), not from
RTE. Copy the needed definitions into your application's `RTE_Device.h`.

## Unsupported (returns `ARM_DRIVER_ERROR_UNSUPPORTED` / specific error)

Master mode / MCLK output / prescaler, `DATA_SIZE` 16/24, justified/PCM/AC97
protocols, mono/companding, slot offset, `MASK_SLOTS_TX/RX`, `FRAME_ERROR`,
dynamic sample-rate switching, and true independent per-direction TX/RX. These
mirror the HAL's validated envelope.

## Status

Verified live (full-duplex I2S/TDM loopback) on dsPIC33AK512MPS512 in the Perseus
integration project. Build-checked standalone with the vendored HALs and the
default `dspic33ak_spi_i2s_tdm_conf.h`.
