#!/bin/bash




sudo perf stat -o config_setup.txt -e r004,r011,r014,r01D,r065,r079 -x "\t" ./parsec-3.0/bin/parsecmgmt -a run -p canneal



exit 0


