# Upstream HAL Source

The files in this directory are a vendor copy of the dsPIC33AK SPI/I2S/TDM HAL.

Upstream repository:

- Repository: https://github.com/sulaolab/dspic33ak-spi-i2s-tdm-hal
- Branch: main
- Source directory: src/

Synchronized into this repository under:

- Destination directory: src/hal_spi_i2s_tdm/

## Current Synchronized Revision

- Upstream commit: bdccdb7018b30503decb632e14603cfbbda84cfb

## Notes

- `dspic33ak_spi_i2s_tdm_conf.h` here is a concrete default config (single SPI1
  TDM8) so this repository compiles as-is; the upstream HAL ships only
  `dspic33ak_spi_i2s_tdm_conf.h_example`. Replace it for your board.

## Update Policy

The HAL-only repository is the upstream source of truth. Apply HAL fixes and HAL
feature changes to the upstream HAL repository first, then synchronize this
vendor copy. CMSIS-Driver SAI wrapper changes belong in this repository.
