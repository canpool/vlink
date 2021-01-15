inc-g-$(CONFIG_CRC)     += inc
src-prefix              := src
src-$(CONFIG_CRC8)      += crc8.c
src-$(CONFIG_CRC16)     += crc16.c
src-$(CONFIG_CRC_CCITT) += crc_ccitt.c
src-$(CONFIG_CRC32)     += crc32.c
src-$(CONFIG_CRC64)     += crc64.c
