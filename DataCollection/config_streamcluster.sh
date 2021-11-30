#!/bin/bash




sudo perf stat -o config_setup.txt -e r013,r010,r012,r004,r003,r002 -x "\t" ./parsec-3.0/bin/parsecmgmt -a run -p streamcluster



exit 0


