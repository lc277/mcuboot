dd if=/dev/zero of=./app_raw_v1.bin bs=1024 count=1
imgtool sign \
--header-size 512 \
--align 1 \
--slot-size 1048576 \
--version "1.0.0" \
./app_raw_v1.bin ./app_signed_v1.bin
# truncate -s 1048576 ./app_signed_v1.bin

dd if=/dev/zero of=./app_raw_v2.bin bs=2048 count=1
imgtool sign \
--header-size 512 \
--align 1 \
--slot-size 1048576 \
--pad \
--version "2.0.0" \
./app_raw_v2.bin ./app_signed_v2.bin
# truncate -s 1048576 ./app_signed_v2.bin

truncate -s 5M sim_flash.bin
dd if=./app_signed_v1.bin of=sim_flash.bin bs=4096 oseek=262144 conv=notrunc
dd if=./app_signed_v2.bin of=sim_flash.bin bs=4096 oseek=1310720 conv=notrunc

# 2. 篡改slot0镜像头部magic（offset=0x40000，前4字节）
# printf '\x00\x00\x00\x00' | dd of=sim_flash.bin bs=1 seek=262144 count=4 conv=notrunc
