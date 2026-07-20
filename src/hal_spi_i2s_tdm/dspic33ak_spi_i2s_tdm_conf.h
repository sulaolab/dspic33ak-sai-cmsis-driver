#ifndef DSPIC33AK_SPI_I2S_TDM_CONF_H
#define DSPIC33AK_SPI_I2S_TDM_CONF_H

//===========================================================
// dspic33ak_spi_i2s_tdm_conf.h  --  DEFAULT config for this repository (compiled)
//
// This is a concrete default config (single SPI1 TDM8 stream) so this repository
// compiles as-is for a compile-check. It is the file the HAL core resolves for
// `#include "dspic33ak_spi_i2s_tdm_conf.h"`. For your own board, edit the values
// below or replace this file. The annotated bring-your-own template is kept
// alongside as `dspic33ak_spi_i2s_tdm_conf.h_example`.
//
// This is the SOLE, self-contained HAL config entry: the core translation units include
// ONLY this header for config and it has NO app-layer dependency (it never includes
// app_specific_config_*). Everything the HAL needs lives here as plain literals -- so
// hal_spi_i2s_tdm/ stands alone (given a supplied conf.h) and is publishable as-is.
//
// Dependency direction: app code MAY read these macros (app -> HAL); the HAL MUST NOT
// read app config (HAL -> app is forbidden). If your project also derives these values
// in its own config, keep the two as independent owners and assert their CONSISTENCY on
// the APP side (compare your APP_* against the DSPIC33AK_TDM_* here and #error on a
// mismatch). The HAL does not police the app, and vice versa.
//
// Each setting is -D-overridable (#ifndef-guarded). This project config defaults to a
// single SPI1 TDM8 stream with no additional transport legs. Override with -D or edit
// below for I2S (2 slots), a different block size, or additional transport legs.
//
// Compile-time integration settings:
//   DSPIC33AK_TDM_SLOTS_PER_FS   slots per frame-sync: TDM8 = 8, I2S = 2.
//   DSPIC33AK_TDM_BLOCK_FRAMES   frames per ping/pong half (DMA block size).
//   DSPIC33AK_TDM_USE_SPI2      1 = second logical transport row is built.
//   DSPIC33AK_TDM_USE_SPI3/4    1 = additional physical SPI3/SPI4 rows are built (AK512 only).
//   DSPIC33AK_TDM_BASE_ON_SPI34 1 = remap logical rows 0/1 from SPI1/2 to SPI3/4 (AK512 only).
// AK128 has no SPI4 or DMA6/7, so paired SPI3/4 and four-leg modes are unavailable.
// (Sample rate is NOT a setting here -- the transport is rate-agnostic; the product's
// supported-rate policy lives in the app layer, not the HAL.)
// The core's static DMA ping-pong buffers are sized 2 * SLOTS_PER_FS *
// BLOCK_FRAMES, and inst_configure() / configure_system() reject a config_t whose slots_per_fs /
// block_frames do not match these compile-time values.
//===========================================================

// --- HAL geometry / topology (literals; -D wins) ---
#ifndef DSPIC33AK_TDM_SLOTS_PER_FS
#define DSPIC33AK_TDM_SLOTS_PER_FS    8     // TDM8 (I2S = 2)
#endif
#ifndef DSPIC33AK_TDM_BLOCK_FRAMES
#define DSPIC33AK_TDM_BLOCK_FRAMES    32    // frames per ping/pong half
#endif
#ifndef DSPIC33AK_TDM_USE_SPI2
#define DSPIC33AK_TDM_USE_SPI2        0     // single SPI Audio transport by default
#endif
#ifndef DSPIC33AK_TDM_USE_SPI3
#define DSPIC33AK_TDM_USE_SPI3        0     // no additional SPI3 row by default
#endif
#ifndef DSPIC33AK_TDM_USE_SPI4
#define DSPIC33AK_TDM_USE_SPI4        0     // no additional SPI4 row by default
#endif
#ifndef DSPIC33AK_TDM_BASE_ON_SPI34
#define DSPIC33AK_TDM_BASE_ON_SPI34   0     // default logical bank is physical SPI1/SPI2
#endif

//===========================================================
// DMA channel allocation (single source of truth for the SPI<->DMA binding)
//
// Each SPI Audio instance owns one RX + one TX DMA channel, read by the HAL core for its
// s_spi_legs[] table. NOTE: the RX interrupt vectors are EXPLICIT C (_DMA0/2/4/6Interrupt),
// each bound to its RX-DMA channel by a compile-time assert -- they are NOT token-paste-generated
// from these macros. So with DSPIC33AK_TDM_DEFINE_DMA_VECTORS=1 (default), the RX-DMA channels
// are effectively fixed at 0/2/4/6 for SPI1/2/3/4. Changing e.g. SPI1_RX_DMA to 4 fails the
// build on the assert. To use other RX channels, set
// DSPIC33AK_TDM_DEFINE_DMA_VECTORS=0 and provide your own _DMA<rx>Interrupt that calls
// dspic33ak_spi_i2s_tdm_inst_rx_isr(), or edit the core's explicit vector section to match.
// TX channels are free (interrupt-less). Each is -D overridable. Maintain a chip-wide map by
// hand: the HAL cannot see other subsystems' DMA usage. Assignment errors are caught in two
// places: an RX-DMA channel that no longer matches its explicit vector fails the build on the
// _Static_assert; a duplicate channel (RX==TX on a leg, or shared across legs) is rejected at
// runtime by the topology validation (ERR_TOPOLOGY) before open()/start().
//===========================================================
#ifndef DSPIC33AK_TDM_SPI1_RX_DMA
#define DSPIC33AK_TDM_SPI1_RX_DMA   0
#endif
#ifndef DSPIC33AK_TDM_SPI1_TX_DMA
#define DSPIC33AK_TDM_SPI1_TX_DMA   1
#endif
#ifndef DSPIC33AK_TDM_SPI2_RX_DMA
#define DSPIC33AK_TDM_SPI2_RX_DMA   2
#endif
#ifndef DSPIC33AK_TDM_SPI2_TX_DMA
#define DSPIC33AK_TDM_SPI2_TX_DMA   3
#endif
#ifndef DSPIC33AK_TDM_SPI3_RX_DMA
#define DSPIC33AK_TDM_SPI3_RX_DMA   4
#endif
#ifndef DSPIC33AK_TDM_SPI3_TX_DMA
#define DSPIC33AK_TDM_SPI3_TX_DMA   5
#endif
#ifndef DSPIC33AK_TDM_SPI4_RX_DMA
#define DSPIC33AK_TDM_SPI4_RX_DMA   6
#endif
#ifndef DSPIC33AK_TDM_SPI4_TX_DMA
#define DSPIC33AK_TDM_SPI4_TX_DMA   7
#endif


//===========================================================
// DMA interrupt-vector ownership.
//   1 (default) : TURNKEY -- the HAL DEFINES the _DMA<rx>Interrupt vectors itself.
//   0           : the HAL defines NO vectors. The integrator owns the IVT and calls
//                 dspic33ak_spi_i2s_tdm_inst_rx_isr(spiN()) from their own
//                 _DMA<rx>Interrupt for each instance's RX channel.
//===========================================================
#ifndef DSPIC33AK_TDM_DEFINE_DMA_VECTORS
#define DSPIC33AK_TDM_DEFINE_DMA_VECTORS   1
#endif


//===========================================================
// Instance count + physical assignment.
//
// The transport core defines its leg enum, per-instance ping-pong buffers, the
// s_spi_legs[] table, and the explicit _DMA<rx>Interrupt vectors directly in C, keyed off
// the per-instance channel #defines above (DSPIC33AK_TDM_SPIn_RX/TX_DMA) and the stream
// geometry (DSPIC33AK_TDM_SLOTS_PER_FS / _BLOCK_FRAMES). By default, logical rows 0/1 map
// to physical SPI1/SPI2. DSPIC33AK_TDM_BASE_ON_SPI34 explicitly remaps those same two rows
// to SPI3/SPI4; DSPIC33AK_TDM_USE_SPI3/4 instead add physical SPI3/SPI4 rows after SPI1/SPI2.
// The per-leg clock role / stream shape are set at runtime by the integrator's config
// (dspic33ak_spi_i2s_tdm_inst_configure() / _configure_system()), not here.
//===========================================================

// Per-leg SYNC DOMAIN id: the s_spi_legs[] default (a caller using configure_system() may
// override it per leg at runtime). Legs sharing a domain are co-clocked and started phase-locked
// as a group; legs in different domains are started/rolled-back separately and need not share
// BCLK/FS. NOTE: this is NOT full independence -- source-readiness is engine-wide/primary-gated and
// shared resources (CLC10, the board clock port) are not per-domain. NOT the clock role
// (config_t.clock_role). Default: one co-clocked group (domain 0); give a separate leg its own id
// (1, 2, ...) and add the matching DSPIC33AK_TDM_<name>_SYNC_DOMAIN.
#ifndef DSPIC33AK_TDM_SPI1_SYNC_DOMAIN
#define DSPIC33AK_TDM_SPI1_SYNC_DOMAIN   (0)
#endif
#ifndef DSPIC33AK_TDM_SPI2_SYNC_DOMAIN
#define DSPIC33AK_TDM_SPI2_SYNC_DOMAIN   (0)
#endif
#ifndef DSPIC33AK_TDM_SPI3_SYNC_DOMAIN
#define DSPIC33AK_TDM_SPI3_SYNC_DOMAIN   (2)
#endif
#ifndef DSPIC33AK_TDM_SPI4_SYNC_DOMAIN
#define DSPIC33AK_TDM_SPI4_SYNC_DOMAIN   (3)
#endif

// sync_domain must be 0..31 (start_all_domains()'s dedup/rollback mask range). The core also
// rejects an out-of-range domain at inst_configure()/configure_system(), but catch it here too.
// Reject negatives as well as >=32: a negative literal would cast to a large uint8_t at runtime.
#if ((DSPIC33AK_TDM_SPI1_SYNC_DOMAIN) < 0) || ((DSPIC33AK_TDM_SPI1_SYNC_DOMAIN) >= 32)
#error "DSPIC33AK_TDM_SPI1_SYNC_DOMAIN must be in 0..31."
#endif
#if DSPIC33AK_TDM_USE_SPI2 && (((DSPIC33AK_TDM_SPI2_SYNC_DOMAIN) < 0) || ((DSPIC33AK_TDM_SPI2_SYNC_DOMAIN) >= 32))
#error "DSPIC33AK_TDM_SPI2_SYNC_DOMAIN must be in 0..31."
#endif
#if DSPIC33AK_TDM_USE_SPI3 && (((DSPIC33AK_TDM_SPI3_SYNC_DOMAIN) < 0) || ((DSPIC33AK_TDM_SPI3_SYNC_DOMAIN) >= 32))
#error "DSPIC33AK_TDM_SPI3_SYNC_DOMAIN must be in 0..31."
#endif
#if DSPIC33AK_TDM_USE_SPI4 && (((DSPIC33AK_TDM_SPI4_SYNC_DOMAIN) < 0) || ((DSPIC33AK_TDM_SPI4_SYNC_DOMAIN) >= 32))
#error "DSPIC33AK_TDM_SPI4_SYNC_DOMAIN must be in 0..31."
#endif


#if (DSPIC33AK_TDM_SLOTS_PER_FS <= 0)
#error "DSPIC33AK_TDM_SLOTS_PER_FS must be positive."
#endif

#if (DSPIC33AK_TDM_SLOTS_PER_FS > 255)
#error "DSPIC33AK_TDM_SLOTS_PER_FS must fit in uint8_t."
#endif

#if (DSPIC33AK_TDM_BLOCK_FRAMES <= 0)
#error "DSPIC33AK_TDM_BLOCK_FRAMES must be positive."
#endif

#if (DSPIC33AK_TDM_BLOCK_FRAMES > 65535)
#error "DSPIC33AK_TDM_BLOCK_FRAMES must fit in uint16_t."
#endif

#if ((DSPIC33AK_TDM_USE_SPI2 != 0) && (DSPIC33AK_TDM_USE_SPI2 != 1))
#error "DSPIC33AK_TDM_USE_SPI2 must be 0 or 1."
#endif
#if ((DSPIC33AK_TDM_USE_SPI3 != 0) && (DSPIC33AK_TDM_USE_SPI3 != 1))
#error "DSPIC33AK_TDM_USE_SPI3 must be 0 or 1."
#endif
#if ((DSPIC33AK_TDM_USE_SPI4 != 0) && (DSPIC33AK_TDM_USE_SPI4 != 1))
#error "DSPIC33AK_TDM_USE_SPI4 must be 0 or 1."
#endif
#if ((DSPIC33AK_TDM_BASE_ON_SPI34 != 0) && (DSPIC33AK_TDM_BASE_ON_SPI34 != 1))
#error "DSPIC33AK_TDM_BASE_ON_SPI34 must be 0 or 1."
#endif
#if (DSPIC33AK_TDM_USE_SPI3 != DSPIC33AK_TDM_USE_SPI4)
#error "The SPI3/SPI4 expansion requires SPI3 and SPI4 together."
#endif
#if DSPIC33AK_TDM_BASE_ON_SPI34 && !DSPIC33AK_TDM_USE_SPI2
#error "The SPI34 test bank requires two logical rows (DSPIC33AK_TDM_USE_SPI2=1)."
#endif
#if DSPIC33AK_TDM_BASE_ON_SPI34 && (DSPIC33AK_TDM_USE_SPI3 || DSPIC33AK_TDM_USE_SPI4)
#error "SPI34 test-bank mode and simultaneous four-leg mode are mutually exclusive."
#endif
#if DSPIC33AK_TDM_USE_SPI3 && !DSPIC33AK_TDM_USE_SPI2
#error "SPI3/SPI4 expansion currently requires the existing SPI1/SPI2 pair."
#endif

#if (DSPIC33AK_TDM_SLOTS_PER_FS > (2147483647 / (2 * DSPIC33AK_TDM_BLOCK_FRAMES)))
#error "SPI/I2S/TDM DMA buffer geometry overflows the static buffer element count."
#endif

#endif // DSPIC33AK_SPI_I2S_TDM_CONF_H
