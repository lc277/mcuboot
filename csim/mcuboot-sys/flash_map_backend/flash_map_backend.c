/****************************************************************************
 * boot/nuttx/src/flash_map_backend/flash_map_backend.c
 *
 * Copyright (c) 2021 Espressif Systems (Shanghai) Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <bootutil/bootutil_log.h>

#include "flash_map_backend/flash_map_backend.h"
#include "os/os_malloc.h"
#include "sysflash/sysflash.h"

#define OK    0
#define ERROR -1

/* ========== Linux File Flash Simulation Config ========== */
#define SIM_FLASH_FILE_PATH "./sim_flash.bin"
/* 模拟flash总大小 4MB，和mcuuboot常见配置对齐 */
#define SIM_FLASH_TOTAL_SIZE (4 * 1024 * 1024U)
/* flash erase sector size，模拟nor flash 4KB sector */
#define SIM_SECTOR_SIZE 4096U
/* flash write alignment，一般1字节，部分flash需要4 */
#define SIM_WRITE_ALIGN 1U
/* 擦除之后字节值，nor flash默认0xFF，原桩代码写0是bug */
#define SIM_ERASED_BYTE 0xFFU

static int g_sim_fd = -1;

/* 模拟flash_area 分区表，对应sysflash.h定义的FLASH_AREA_IMAGE_PRIMARY / SECONDARY */
/* 模拟flash_area 分区表，严格匹配你sysflash.h宏定义 */
static const struct flash_area s_flash_areas[] = {
    /* bootloader 256K */
    {.fa_id = FLASH_AREA_BOOTLOADER,        .fa_off = 0x000000, .fa_size = 0x040000},
    /* Image0 primary 1MB */
    {.fa_id = FLASH_AREA_IMAGE_0_PRIMARY,   .fa_off = 0x040000, .fa_size = 0x100000},
    /* Image0 secondary 1MB */
    {.fa_id = FLASH_AREA_IMAGE_0_SECONDARY, .fa_off = 0x140000, .fa_size = 0x100000},
    /* scratch：必须至少1MB，和image0 slot一样大 */
    {.fa_id = FLASH_AREA_IMAGE_SCRATCH,     .fa_off = 0x240000, .fa_size = 0x100000},
    /* Image1 */
    {.fa_id = FLASH_AREA_IMAGE_1_PRIMARY,   .fa_off = 0x340000, .fa_size = 0x080000},
    {.fa_id = FLASH_AREA_IMAGE_1_SECONDARY, .fa_off = 0x3c0000, .fa_size = 0x080000},
};

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

/* 内部：根据id查找flash_area */
static const struct flash_area* flash_area_lookup(uint8_t id)
{
    for (size_t i = 0; i < ARRAYSIZE(s_flash_areas); i++)
    {
        if (s_flash_areas[i].fa_id == id)
        {
            return &s_flash_areas[i];
        }
    }
    return NULL;
}

/* 确保模拟flash文件存在，大小等于SIM_FLASH_TOTAL_SIZE，初始全部0xFF */
static int sim_flash_init(void)
{
    if (g_sim_fd >= 0)
    {
        return OK;
    }
    g_sim_fd = open(SIM_FLASH_FILE_PATH, O_RDWR | O_CREAT, 0644);
    if (g_sim_fd < 0)
    {
        BOOT_LOG_ERR("open sim flash file failed %s errno=%d", SIM_FLASH_FILE_PATH, errno);
        return ERROR;
    }
    /* 扩展文件到总flash大小 */
    off_t cur = lseek(g_sim_fd, 0, SEEK_END);
    if ((size_t)cur < SIM_FLASH_TOTAL_SIZE)
    {
        if (lseek(g_sim_fd, SIM_FLASH_TOTAL_SIZE - 1, SEEK_SET) < 0)
        {
            return ERROR;
        }
        uint8_t tmp = SIM_ERASED_BYTE;
        ssize_t ret = write(g_sim_fd, &tmp, 1);
        if (ret < 0)
        {
            BOOT_LOG_ERR("write sim flash file failed errno=%d", errno);
            return ERROR;
        }
    }
    lseek(g_sim_fd, 0, SEEK_SET);
    return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int flash_area_open(uint8_t id, const struct flash_area** fa)
{
    if (fa == NULL)
    {
        return ERROR;
    }
    *fa = NULL;

    /* 关键：拦截无效slot 255 */
    if (id == FLASH_SLOT_DOES_NOT_EXIST)
    {
        BOOT_LOG_ERR("flash_area_open invalid slot FLASH_SLOT_DOES_NOT_EXIST(255)");
        return ERROR;
    }

    if (sim_flash_init() != OK)
    {
        return ERROR;
    }

    BOOT_LOG_SIM("open ID:%" PRIu8, id);

    const struct flash_area* area = flash_area_lookup(id);
    if (!area)
    {
        BOOT_LOG_ERR("flash area id %" PRIu8 " not found", id);
        return ERROR;
    }
    *fa = area;
    return OK;
}

void flash_area_close(const struct flash_area* fa)
{
    BOOT_LOG_SIM("close ID:%" PRIu8, fa ? fa->fa_id : 0xFF);
}

int flash_area_read(const struct flash_area* fa, uint32_t off, void* dst, uint32_t len)
{
    if (!fa || !dst)
        return ERROR;
    /* 边界校验：不能超出分区 */
    if ((off + len) > fa->fa_size)
    {
        BOOT_LOG_ERR("read out of area boundary off=%u len=%u areasize=%u", off, len, fa->fa_size);
        return ERROR;
    }
    off_t file_offset = fa->fa_off + off;

    BOOT_LOG_SIM("read ID:%" PRIu8 " offset:%" PRIu32 " length:%" PRIu32 " file_off=%ld",
                 fa->fa_id,
                 off,
                 len,
                 (long)file_offset);

    lseek(g_sim_fd, file_offset, SEEK_SET);
    ssize_t r = read(g_sim_fd, dst, len);
    if ((uint32_t)r != len)
    {
        BOOT_LOG_ERR("read fail ret=%zd errno=%d", r, errno);
        return ERROR;
    }
    return OK;
}

/* Flash写约束：只能写1→0，不能0→1；模拟真实nor flash行为 */
static int sim_flash_write_nor(int fd, off_t file_off, const uint8_t* src, uint32_t len)
{
    uint8_t* buf = malloc(len);
    if (!buf)
        return ERROR;
    lseek(fd, file_off, SEEK_SET);
    ssize_t rd = read(fd, buf, len);
    if ((uint32_t)rd != len)
    {
        free(buf);
        return ERROR;
    }
    /* nor flash: new & old ，不允许 bit 0 → 1 */
    for (uint32_t i = 0; i < len; i++)
    {
        if (((~buf[i]) & src[i]) != 0)
        {
            BOOT_LOG_ERR("nor flash write violation at %ld old=0x%02x new=0x%02x", (long)file_off + i, buf[i], src[i]);
            free(buf);
            return ERROR;
        }
        buf[i] = buf[i] & src[i];
    }
    lseek(fd, file_off, SEEK_SET);
    ssize_t wr = write(fd, buf, len);
    free(buf);
    if ((uint32_t)wr != len)
    {
        return ERROR;
    }
    fsync(fd);
    return OK;
}

int flash_area_write(const struct flash_area* fa, uint32_t off, const void* src, uint32_t len)
{
    if (!fa || !src)
        return ERROR;
    if ((off + len) > fa->fa_size)
    {
        BOOT_LOG_ERR("write out of area boundary");
        return ERROR;
    }
    off_t file_offset = fa->fa_off + off;
    BOOT_LOG_SIM("write ID:%" PRIu8 " offset:%" PRIu32 " length:%" PRIu32 " file_off=%ld",
                 fa->fa_id,
                 off,
                 len,
                 (long)file_offset);

    return sim_flash_write_nor(g_sim_fd, file_offset, src, len);
}

int flash_area_erase(const struct flash_area* fa, uint32_t off, uint32_t len)
{
    if (!fa)
        return ERROR;
    if ((off + len) > fa->fa_size)
    {
        BOOT_LOG_ERR("erase out of area boundary");
        return ERROR;
    }
    /* 对齐到sector边界，mcuuboot要求erase按sector对齐 */
    if ((off % SIM_SECTOR_SIZE) != 0 || (len % SIM_SECTOR_SIZE) != 0)
    {
        BOOT_LOG_ERR("erase not sector aligned off=%u len=%u sector=%u", off, len, SIM_SECTOR_SIZE);
        return ERROR;
    }

    off_t file_offset = fa->fa_off + off;
    BOOT_LOG_SIM("erase ID:%" PRIu8 " offset:%" PRIu32 " length:%" PRIu32 " file_off=%ld",
                 fa->fa_id,
                 off,
                 len,
                 (long)file_offset);

    uint8_t* erase_buf = malloc(len);
    if (!erase_buf)
        return ERROR;
    memset(erase_buf, SIM_ERASED_BYTE, len);

    lseek(g_sim_fd, file_offset, SEEK_SET);
    ssize_t w = write(g_sim_fd, erase_buf, len);
    fsync(g_sim_fd);
    free(erase_buf);

    if ((uint32_t)w != len)
    {
        BOOT_LOG_ERR("erase write failed ret=%zd", w);
        return ERROR;
    }
    return OK;
}

uint32_t flash_area_align(const struct flash_area* fa)
{
    (void)fa;
    BOOT_LOG_SIM("align ID:%" PRIu8 " align:%" PRIu32, fa->fa_id, SIM_WRITE_ALIGN);
    return SIM_WRITE_ALIGN;
}

uint8_t flash_area_erased_val(const struct flash_area* fa)
{
    (void)fa;
    BOOT_LOG_SIM("erased ID:%" PRIu8 " erased_val:0x%" PRIx8, fa->fa_id, SIM_ERASED_BYTE);
    return SIM_ERASED_BYTE;
}

int flash_area_get_sectors(int fa_id, uint32_t* count, struct flash_sector* sectors)
{
    BOOT_LOG_SIM("get_sectors ID:%d count:%" PRIu32, fa_id, *count);

    const struct flash_area* fa = flash_area_lookup(fa_id);
    if (!fa)
    {
        return ERROR;
    }
    uint32_t total_sectors = fa->fa_size / SIM_SECTOR_SIZE;
    uint32_t copy_cnt = (*count < total_sectors) ? *count : total_sectors;

    for (uint32_t i = 0; i < copy_cnt; i++)
    {
        sectors[i].fs_off = i * SIM_SECTOR_SIZE;
        sectors[i].fs_size = SIM_SECTOR_SIZE;
    }
    *count = total_sectors;
    return OK;
}

int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    BOOT_LOG_SIM("image_index:%d slot:%d", image_index, slot);

    switch (slot)
    {
    case 0:
        return FLASH_AREA_IMAGE_PRIMARY(image_index);
    case 1:
        return FLASH_AREA_IMAGE_SECONDARY(image_index);
    }

    BOOT_LOG_ERR("Unexpected Request: image_index:%d, slot:%d", image_index, slot);
    return ERROR;
}

int flash_area_id_from_image_slot(int slot)
{
    BOOT_LOG_SIM("slot:%d", slot);
    return flash_area_id_from_multi_image_slot(0, slot);
}

int flash_area_id_to_multi_image_slot(int image_index, int area_id)
{
    BOOT_LOG_SIM("image_index:%d area_id:%d", image_index, area_id);

    if (area_id == FLASH_AREA_IMAGE_PRIMARY(image_index))
    {
        return 0;
    }
    if (area_id == FLASH_AREA_IMAGE_SECONDARY(image_index))
    {
        return 1;
    }

    BOOT_LOG_ERR("Unexpected Request: image_index:%d, area_id:%d", image_index, area_id);
    return ERROR;
}

int flash_area_id_from_image_offset(uint32_t offset)
{
    BOOT_LOG_SIM("offset:%" PRIu32, offset);
    for (size_t i = 0; i < ARRAYSIZE(s_flash_areas); i++)
    {
        const struct flash_area* fa = &s_flash_areas[i];
        if (offset >= fa->fa_off && offset < (fa->fa_off + fa->fa_size))
        {
            return (int)fa->fa_id;
        }
    }
    BOOT_LOG_ERR("offset %" PRIu32 " does not belong to any flash area", offset);
    return ERROR;
}

int flash_area_get_sector(const struct flash_area* fap, uint32_t off, struct flash_sector* fs)
{
    if (!fap || !fs)
        return ERROR;
    if (off >= fap->fa_size)
        return ERROR;

    fs->fs_off = (off / SIM_SECTOR_SIZE) * SIM_SECTOR_SIZE;
    fs->fs_size = SIM_SECTOR_SIZE;
    return 0;
}

int flash_area_to_sectors(int idx, int* cnt, struct flash_area* fa)
{
    (void)idx;
    (void)cnt;
    (void)fa;
    return -1;
}
