#!/usr/bin/env bash
set -euo pipefail

zip -r b09902004.zip b09902004/src b09902004/Makefile b09902004/report.pdf
sha1sum b09902004.zip | tee b09902004.zip.sha1sum
