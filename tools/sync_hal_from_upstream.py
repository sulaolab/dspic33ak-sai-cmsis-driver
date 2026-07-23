#!/usr/bin/env python3
"""Synchronize the vendored dsPIC33AK HALs from their upstream HAL repos.

This CMSIS-Driver SAI wrapper vendors three sibling HALs:
  - dspic33ak-hal-spi-i2s-tdm -> src/hal_spi_i2s_tdm/
  - dspic33ak-hal-dma         -> src/hal_dma/
  - dspic33ak-hal-timer       -> src/hal_timer/   (high-resolution counter only)

Each HAL's UPSTREAM.md records the synchronized commit and is updated in place.
The repository-local default config src/hal_spi_i2s_tdm/dspic33ak_spi_i2s_tdm_conf.h
is NOT synced (the upstream ships only the *_conf.h_example template).
"""

from __future__ import annotations

import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


# (label, repo, branch, upstream source dir, destination dir, files)
HALS = (
    (
        "dspic33ak-hal-spi-i2s-tdm",
        "https://github.com/sulaolab/dspic33ak-hal-spi-i2s-tdm.git",
        "main",
        "src",
        "src/hal_spi_i2s_tdm",
        (
            "dspic33ak_spi_i2s_tdm.c",
            "dspic33ak_spi_i2s_tdm.h",
            "dspic33ak_spi_i2s_tdm_diag.c",
            "dspic33ak_spi_i2s_tdm_diag.h",
            "dspic33ak_spi_i2s_tdm_hw.c",
            "dspic33ak_spi_i2s_tdm_hw.h",
            "dspic33ak_spi_i2s_tdm_fs_clc.c",
            "dspic33ak_spi_i2s_tdm_fs_clc.h",
            "dspic33ak_spi_i2s_tdm_reg.h",
            "dspic33ak_spi_i2s_tdm_conf.h_example",
        ),
    ),
    (
        "dspic33ak-hal-dma",
        "https://github.com/sulaolab/dspic33ak-hal-dma.git",
        "main",
        "src",
        "src/hal_dma",
        (
            "dspic33ak_dma.c",
            "dspic33ak_dma.h",
            "dspic33ak_dma_reg.h",
        ),
    ),
    (
        "dspic33ak-hal-timer",
        "https://github.com/sulaolab/dspic33ak-hal-timer.git",
        "main",
        "src",
        "src/hal_timer",
        (
            "dspic33ak_high_res_timer.c",
            "dspic33ak_high_res_timer.h",
        ),
    ),
)


def run(command: list[str], cwd: Path | None = None) -> str:
    result = subprocess.run(
        command,
        cwd=cwd,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        if result.stdout:
            print(result.stdout, end="", file=sys.stdout)
        if result.stderr:
            print(result.stderr, end="", file=sys.stderr)
        raise SystemExit(result.returncode)
    return result.stdout.strip()


def require_repo_root(repo_root: Path) -> None:
    required = [repo_root / "README.md"]
    required += [repo_root / dest / "UPSTREAM.md" for (_, _, _, _, dest, _) in HALS]
    missing = [str(p.relative_to(repo_root)) for p in required if not p.exists()]
    if missing:
        raise SystemExit(f"Run this script from the repository root; missing: {', '.join(missing)}")


def clone_upstream(work_dir: Path, repo: str, branch: str, label: str) -> tuple[Path, str]:
    upstream_dir = work_dir / label
    run(["git", "clone", "--depth", "1", "--branch", branch, repo, str(upstream_dir)])
    commit = run(["git", "rev-parse", "HEAD"], cwd=upstream_dir)
    return upstream_dir, commit


def copy_hal_files(upstream_dir: Path, source_dir: str, repo_root: Path, dest_dir: str, files) -> None:
    src = upstream_dir / source_dir
    dst = repo_root / dest_dir
    dst.mkdir(parents=True, exist_ok=True)
    for name in files:
        source_path = src / name
        if not source_path.is_file():
            raise SystemExit(f"Upstream file not found: {source_path}")
        shutil.copy2(source_path, dst / name)


def update_upstream_md(repo_root: Path, dest_dir: str, commit: str) -> None:
    upstream_md = repo_root / dest_dir / "UPSTREAM.md"
    text = upstream_md.read_text(encoding="utf-8")
    updated, n = re.subn(
        r"- Upstream commit: [0-9a-fA-F]+",
        f"- Upstream commit: {commit}",
        text,
        count=1,
    )
    if n != 1:
        raise SystemExit(f"Could not update upstream commit line in {dest_dir}/UPSTREAM.md")
    upstream_md.write_text(updated, encoding="utf-8", newline="\n")


def main() -> int:
    repo_root = Path.cwd().resolve()
    require_repo_root(repo_root)

    with tempfile.TemporaryDirectory(prefix="dspic33ak_sai_hal_") as temp_dir:
        for (label, repo, branch, source_dir, dest_dir, files) in HALS:
            upstream_dir, commit = clone_upstream(Path(temp_dir), repo, branch, label)
            copy_hal_files(upstream_dir, source_dir, repo_root, dest_dir, files)
            update_upstream_md(repo_root, dest_dir, commit)
            print(f"Synchronized {dest_dir} from sulaolab/{label} @ {commit}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
