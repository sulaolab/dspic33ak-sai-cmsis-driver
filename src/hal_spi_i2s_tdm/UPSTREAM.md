# Upstream HAL Source

The files in this directory are a vendor copy of the dsPIC33AK SPI/I2S/TDM HAL.

Upstream repository:

- Repository: https://github.com/sulaolab/dspic33ak-spi-i2s-tdm-hal
- Branch: feat/system-topology-v2 (pre-release)
- Source directory: src/

Synchronized into this repository under:

- Destination directory: src/hal_spi_i2s_tdm/

## Current Synchronized Revision

- Upstream commit: a13499da0e36db8aa9675105a2ee1df668c2cf00

> Pre-release: this revision is vendored from the upstream's `feat/system-topology-v2`
> branch (the system-topology HAL: `configure_system()`, `open()` with no role argument,
> per-domain framing validation, config-ownership mode). The `main`-based
> `tools/sync_hal_from_upstream.py` sync will be re-run once that upstream work merges to
> the upstream `main`; this vendor copy is byte-identical (EOL-normalized) to that upstream
> commit (the branch tip at vendor time).

## Notes

- `dspic33ak_spi_i2s_tdm_conf.h` here is a concrete default config (single SPI1
  TDM8) so this repository compiles as-is; the upstream HAL ships only
  `dspic33ak_spi_i2s_tdm_conf.h_example`. Replace it for your board.

## Update Policy

The HAL-only repository is the upstream source of truth. Apply HAL fixes and HAL
feature changes to the upstream HAL repository first, then synchronize this
vendor copy. CMSIS-Driver SAI wrapper changes belong in this repository.
