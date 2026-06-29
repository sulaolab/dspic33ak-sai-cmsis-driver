# Upstream HAL Source

The files in this directory are a partial vendor copy of the dsPIC33AK Timer HAL:
only the Timer2 high-resolution counter, used by the SPI/I2S/TDM HAL load
monitor (a compile/link sibling dependency, runtime-gated via
`dspic33ak_high_res_timer_is_initialized()`). The tick-timer half of the upstream
Timer HAL is not vendored here.

Upstream repository:

- Repository: https://github.com/sulaolab/dspic33ak-timer-hal
- Branch: main
- Source directory: src/

Synchronized into this repository under:

- Destination directory: src/hal_timer/ (high-resolution counter only)

## Current Synchronized Revision

- Upstream commit: 98fd377820637c9686b5f66d6a425564d2941519

## Update Policy

The HAL-only repository is the upstream source of truth. Apply HAL changes
upstream first, then synchronize this vendor copy.
