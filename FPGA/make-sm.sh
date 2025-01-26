#!/usr/bin/bash
make -C opensbi O=$(pwd)/zza-tmp-build KEYSTONE_PLATFORM=vivado-risc-v PLATFORM_DIR=$(pwd)/plat/vivado-risc-v CROSS_COMPILE=/opt/riscv64/bin/riscv64-unknown-linux-gnu- FW_PAYLOAD_PATH=`realpath ./u-boot-nodtb.bin`


