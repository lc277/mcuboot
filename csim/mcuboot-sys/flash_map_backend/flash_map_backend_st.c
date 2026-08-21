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
#if 0

#include <stddef.h>
#include <stdint.h>

#include <bootutil/bootutil_log.h>

#include "flash_map_backend/flash_map_backend.h"
#include "os/os_malloc.h"
#include "sysflash/sysflash.h"

#define OK    0
#define ERROR -1

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: flash_area_open
 *
 * Description:
 *   Retrieve flash area from the flash map for a given ID.
 *
 * Input Parameters:
 *   id - ID of the flash area.
 *
 * Output Parameters:
 *   fa - Pointer which will contain the reference to flash_area.
 *        If ID is unknown, it will be NULL on output.
 *
 * Returned Value:
 *   Zero on success, or negative value in case of error.
 *
 ****************************************************************************/

int flash_area_open(uint8_t id, const struct flash_area** fa)
{
    int fd;
    int ret;

    BOOT_LOG_INF("ID:%" PRIu8, id);

    return OK;
}

/****************************************************************************
 * Name: flash_area_close
 *
 * Description:
 *   Close a given flash area.
 *
 * Input Parameters:
 *   fa - Flash area to be closed.
 *
 * Returned Value:
 *   None.
 *
 ****************************************************************************/

void flash_area_close(const struct flash_area* fa)
{
    BOOT_LOG_INF("ID:%" PRIu8, fa->fa_id);
}

/****************************************************************************
 * Name: flash_area_read
 *
 * Description:
 *   Read data from flash area.
 *   Area readout boundaries are asserted before read request. API has the
 *   same limitation regarding read-block alignment and size as the
 *   underlying flash driver.
 *
 * Input Parameters:
 *   fa  - Flash area to be read.
 *   off - Offset relative from beginning of flash area to be read.
 *   len - Number of bytes to read.
 *
 * Output Parameters:
 *   dst - Buffer to store read data.
 *
 * Returned Value:
 *   Zero on success, or negative value in case of error.
 *
 ****************************************************************************/

int flash_area_read(const struct flash_area* fa, uint32_t off, void* dst, uint32_t len)
{
    BOOT_LOG_INF("ID:%" PRIu8 " offset:%" PRIu32 " length:%" PRIu32, fa->fa_id, off, len);

    return OK;
}

/****************************************************************************
 * Name: flash_area_write
 *
 * Description:
 *   Write data to flash area.
 *   Area write boundaries are asserted before write request. API has the
 *   same limitation regarding write-block alignment and size as the
 *   underlying flash driver.
 *
 * Input Parameters:
 *   fa  - Flash area to be written.
 *   off - Offset relative from beginning of flash area to be written.
 *   src - Buffer with data to be written.
 *   len - Number of bytes to write.
 *
 * Returned Value:
 *   Zero on success, or negative value in case of error.
 *
 ****************************************************************************/

int flash_area_write(const struct flash_area* fa, uint32_t off, const void* src, uint32_t len)
{
    BOOT_LOG_INF("ID:%" PRIu8 " offset:%" PRIu32 " length:%" PRIu32, fa->fa_id, off, len);

    return OK;
}

/****************************************************************************
 * Name: flash_area_erase
 *
 * Description:
 *   Erase a given flash area range.
 *   Area boundaries are asserted before erase request. API has the same
 *   limitation regarding erase-block alignment and size as the underlying
 *   flash driver.
 *
 * Input Parameters:
 *   fa  - Flash area to be erased.
 *   off - Offset relative from beginning of flash area to be erased.
 *   len - Number of bytes to be erase.
 *
 * Returned Value:
 *   Zero on success, or negative value in case of error.
 *
 ****************************************************************************/

int flash_area_erase(const struct flash_area* fa, uint32_t off, uint32_t len)
{
    int ret = 0;
    BOOT_LOG_INF("ID:%" PRIu8 " offset:%" PRIu32 " length:%" PRIu32, fa->fa_id, off, len);

    return ret;
}

/****************************************************************************
 * Name: flash_area_align
 *
 * Description:
 *   Get write block size of the flash area.
 *   Write block size might be treated as read block size, although most
 *   drivers support unaligned readout.
 *
 * Input Parameters:
 *   fa - Flash area.
 *
 * Returned Value:
 *   Alignment restriction for flash writes in the given flash area.
 *
 ****************************************************************************/

uint32_t flash_area_align(const struct flash_area* fa)
{
    /* MTD access alignment is handled by the character and block device
     * drivers.
     */

    const uint32_t minimum_write_length = 1;

    BOOT_LOG_INF("ID:%" PRIu8 " align:%" PRIu32, fa->fa_id, minimum_write_length);

    return minimum_write_length;
}

/****************************************************************************
 * Name: flash_area_erased_val
 *
 * Description:
 *   Get the value expected to be read when accessing any erased flash byte.
 *   This API is compatible with the MCUboot's porting layer.
 *
 * Input Parameters:
 *   fa - Flash area.
 *
 * Returned Value:
 *   Byte value of erased memory.
 *
 ****************************************************************************/

uint8_t flash_area_erased_val(const struct flash_area* fa)
{
    uint8_t erased_val = 0;

    BOOT_LOG_INF("ID:%" PRIu8 " erased_val:0x%" PRIx8, fa->fa_id, erased_val);

    return erased_val;
}

/****************************************************************************
 * Name: flash_area_get_sectors
 *
 * Description:
 *   Retrieve info about sectors within the area.
 *
 * Input Parameters:
 *   fa_id   - ID of the flash area whose info will be retrieved.
 *   count   - On input, represents the capacity of the sectors buffer.
 *
 * Output Parameters:
 *   count   - On output, it shall contain the number of retrieved sectors.
 *   sectors - Buffer for sectors data.
 *
 * Returned Value:
 *   Zero on success, or negative value in case of error.
 *
 ****************************************************************************/

int flash_area_get_sectors(int fa_id, uint32_t* count, struct flash_sector* sectors)
{
    BOOT_LOG_INF("ID:%d count:%" PRIu32, fa_id, *count);

    return OK;
}

/****************************************************************************
 * Name: flash_area_id_from_multi_image_slot
 *
 * Description:
 *   Return the flash area ID for a given slot and a given image index
 *   (in case of a multi-image setup).
 *
 * Input Parameters:
 *   image_index - Index of the image.
 *   slot        - Image slot, which may be 0 (primary) or 1 (secondary).
 *
 * Returned Value:
 *   Flash area ID (0 or 1), or negative value in case the requested slot
 *   is invalid.
 *
 ****************************************************************************/

int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    BOOT_LOG_INF("image_index:%d slot:%d", image_index, slot);

    switch (slot)
    {
    case 0:
        return FLASH_AREA_IMAGE_PRIMARY(image_index);
    case 1:
        return FLASH_AREA_IMAGE_SECONDARY(image_index);
    }

    BOOT_LOG_ERR("Unexpected Request: image_index:%d, slot:%d", image_index, slot);

    return ERROR; /* flash_area_open will fail on that */
}

/****************************************************************************
 * Name: flash_area_id_from_image_slot
 *
 * Description:
 *   Return the flash area ID for a given slot.
 *
 * Input Parameters:
 *   slot - Image slot, which may be 0 (primary) or 1 (secondary).
 *
 * Returned Value:
 *   Flash area ID (0 or 1), or negative value in case the requested slot
 *   is invalid.
 *
 ****************************************************************************/

int flash_area_id_from_image_slot(int slot)
{
    BOOT_LOG_INF("slot:%d", slot);

    return flash_area_id_from_multi_image_slot(0, slot);
}

/****************************************************************************
 * Name: flash_area_id_to_multi_image_slot
 *
 * Description:
 *   Convert the specified flash area ID and image index (in case of a
 *   multi-image setup) to an image slot index.
 *
 * Input Parameters:
 *   image_index - Index of the image.
 *   area_id     - Unique identifier that is represented by fa_id in the
 *                 flash_area struct.
 * Returned Value:
 *   Image slot index (0 or 1), or negative value in case ID doesn't
 *   correspond to an image slot.
 *
 ****************************************************************************/

int flash_area_id_to_multi_image_slot(int image_index, int area_id)
{
    BOOT_LOG_INF("image_index:%d area_id:%d", image_index, area_id);

    if (area_id == FLASH_AREA_IMAGE_PRIMARY(image_index))
    {
        return 0;
    }

    if (area_id == FLASH_AREA_IMAGE_SECONDARY(image_index))
    {
        return 1;
    }

    BOOT_LOG_ERR("Unexpected Request: image_index:%d, area_id:%d", image_index, area_id);

    return ERROR; /* flash_area_open will fail on that */
}

/****************************************************************************
 * Name: flash_area_id_from_image_offset
 *
 * Description:
 *   Return the flash area ID for a given image offset.
 *
 * Input Parameters:
 *   offset - Image offset.
 *
 * Returned Value:
 *   Flash area ID (0 or 1), or negative value in case the requested offset
 *   is invalid.
 *
 ****************************************************************************/

int flash_area_id_from_image_offset(uint32_t offset)
{

    BOOT_LOG_INF("offset:%" PRIu32, offset);

    BOOT_LOG_ERR("Unexpected Request: offset:%" PRIu32, offset);

    return ERROR; /* flash_area_open will fail on that */
}

/****************************************************************************
 * Name: flash_area_get_sector
 *
 * Description:
 *   Retrieve the flash sector a given offset belongs to.
 *
 * Input Parameters:
 *   fap - flash area structure
 *   off - address offset.
 *   sector - flash sector
 *
 * Returned Value:
 *   Returns 0 on success, or an error code on failure.
 *
 ****************************************************************************/

int flash_area_get_sector(const struct flash_area* fap, uint32_t off, struct flash_sector* fs)
{

    return 0;
}

int flash_area_to_sectors(int idx, int* cnt, struct flash_area* fa)
{
    return -1;
}

#endif

