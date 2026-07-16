/* SPDX-License-Identifier: MIT-0 */
#ifndef RTE_DEVICE_SAI_DSPIC33AK_EXAMPLE_H
#define RTE_DEVICE_SAI_DSPIC33AK_EXAMPLE_H

/*
 * Example RTE configuration for the dsPIC33AK SAI CMSIS-Driver wrapper.
 *
 * This is an SAI-only example configuration file. It is not a shared
 * application-level RTE_Device.h. In an integrated application, copy the required
 * SAI definitions into that application's RTE_Device.h or equivalent.
 *
 * Driver_SAI0 maps to the single dspic33ak_spi_i2s_tdm transport (SPI1 = the primary leg).
 *
 * Validated envelope (mirrors the HAL tdm_config_is_supported()): dsPIC33AK SPI
 * SLAVE, external BCLK/FS/MCLK, 32-bit word/slot, and either I2S (2 slots) or TDM8
 * (8 slots). The wrapper seeds its config from the integrator's default config
 * (the Driver_SAI_dsPIC33AK_GetDefaultConfig() hook) and overrides only the
 * protocol/slot count from Control(CONFIGURE_*); the HAL core itself has no
 * default-config API. AUDIO_FREQ is not passed to the rate-agnostic HAL core -- it
 * is validated by the wrapper's rate hook (Driver_SAI_dsPIC33AK_IsSampleRateSupported).
 * The wrapper does not advertise or accept anything outside this envelope.
 */

#define RTE_SAI0 1

/* Default sample rate used by the wrapper's weak IsSampleRateSupported() hook. The
 * board-electrical fields (BRG, FRMSYPW/SPIFE/CKP/CKE, MCLKEN, ignore over/underrun) and the
 * block geometry come from the board/integration default config, NOT from here.
 *
 * NOTE: there is deliberately NO protocol selector here. The advertised/accepted protocol
 * (I2S vs TDM8) is fixed by the compiled HAL geometry (DSPIC33AK_TDM_SLOTS_PER_FS in the HAL
 * conf.h): 2 => I2S, 8 => TDM8. An RTE protocol macro would have no effect, so it is omitted. */
#define RTE_SAI0_DEFAULT_SAMPLE_RATE_HZ 48000u

#endif /* RTE_DEVICE_SAI_DSPIC33AK_EXAMPLE_H */
