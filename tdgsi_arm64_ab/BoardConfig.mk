include build/make/target/board/generic_arm64/BoardConfig.mk
include device/phh/treble/board-base.mk

ifeq ($(BOARD_SYSTEMIMAGE_PARTITION_RESERVED_SIZE),)
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648
else
BOARD_SYSTEMIMAGE_PARTITION_RESERVED_SIZE := 33554432
endif

# compress the boot initramfs with LZ4 (faster decompression at boot than the
# default gzip; LZ4's ratio is slightly worse so the ramdisk is marginally
# larger, but boot-time decompression is the win).
BOARD_RAMDISK_USE_LZ4 := true
