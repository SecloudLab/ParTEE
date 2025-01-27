#!/usr/bin/bash
make -C opensbi O=$(pwd)/build PLATFORM=vivado PLATFORM_DIR=$(pwd)/plat/vivado CROSS_COMPILE=/opt/riscv64/bin/riscv64-unknown-linux-gnu- FW_PAYLOAD_PATH=`realpath ./u-boot-nodtb.bin`


