/**
 * @file      main.c
 * @author    Mars
 * @version   V1.0.0
 * @date      2026-04-28
 * @brief     main
 * @details   details
 * @copyright Copyright © 2022 Voltronic Power Technology Corp.
 * @attention
 *
 *
 * @history
 * Version|Author|Date|Describe
 * -------|------|----|--------
 * V1.0.0|Mars|2026-04-28|Create file
 */

/* Compilation Options -------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "bootutil/bootutil.h"
#include "bootutil/image.h"
#include "bootutil/security_cnt.h"
#include "flash_map_backend/flash_map_backend.h"
#include "sysflash/sysflash.h"
#include <stdint.h>

/* Global macro --------------------------------------------------------------*/
#define DBG_TAG "MAIN "
#define DBG_LVL DBG_LOG
#include "rtdbg.h" // must after of DBG_LVL, DBG_TAG or other options

/* Global types --------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Global functions prototypes -----------------------------------------------*/

/* Global functions ----------------------------------------------------------*/

fih_ret boot_nv_security_counter_init(void)
{
    /* Do nothing. */
    return FIH_SUCCESS;
}

fih_ret boot_nv_security_counter_get(uint32_t image_id, fih_int *security_cnt)
{
    (void)image_id;
    *security_cnt = 30;

    return FIH_SUCCESS;
}

int32_t boot_nv_security_counter_update(uint32_t image_id, uint32_t img_security_cnt)
{
    (void)image_id;
    (void)img_security_cnt;

    /* Do nothing. */
    return 0;
}

int sim_log_enabled(int level)
{
    return 1;
}

static const char *swap_type_str(uint8_t type)
{
    switch (type)
    {
    case BOOT_SWAP_TYPE_NONE:
        return "NONE";
    case BOOT_SWAP_TYPE_TEST:
        return "TEST";
    case BOOT_SWAP_TYPE_PERM:
        return "PERM";
    case BOOT_SWAP_TYPE_REVERT:
        return "REVERT";
    case BOOT_SWAP_TYPE_FAIL:
        return "FAIL";
    case BOOT_SWAP_TYPE_PANIC:
        return "PANIC";
    default:
        return "UNKNOWN";
    }
}

static const char *magic_str(uint8_t magic)
{
    switch (magic)
    {
    case BOOT_MAGIC_GOOD:
        return "GOOD";
    case BOOT_MAGIC_BAD:
        return "BAD";
    case BOOT_MAGIC_UNSET:
        return "UNSET";
    case BOOT_MAGIC_ANY:
        return "ANY";
    case BOOT_MAGIC_NOTGOOD:
        return "NOTGOOD";
    default:
        return "UNKNOWN";
    }
}

static const char *flag_str(uint8_t flag)
{
    switch (flag)
    {
    case BOOT_FLAG_SET:
        return "SET";
    case BOOT_FLAG_BAD:
        return "BAD";
    case BOOT_FLAG_UNSET:
        return "UNSET";
    case BOOT_FLAG_ANY:
        return "ANY";
    default:
        return "UNKNOWN";
    }
}

static void do_boot(struct boot_rsp *rsp)
{
    const struct flash_area *flash_area;
    int area_id;
    int ret;

    area_id = flash_area_id_from_image_offset(rsp->br_image_off);

    ret = flash_area_open(area_id, &flash_area);
    if (ret != 0)
    {
        LOG_I("Unable to open flash area %d", area_id);
        FIH_PANIC;
    }

    LOG_I("Booting image at offset 0x%08x", rsp->br_image_off);

    flash_area_close(flash_area);

    {
        struct boot_swap_state st;
        ret = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(0), &st);
        printf("boot_read_swap_state rc=%d\n", ret);
        printf("swap_type=%s magic=%s copy_done=%s image_ok=%s\n", swap_type_str(st.swap_type),
               magic_str(st.magic), flag_str(st.copy_done), flag_str(st.image_ok));

        if (st.swap_type == BOOT_SWAP_TYPE_TEST && st.image_ok == BOOT_FLAG_UNSET)
        {
            boot_set_confirmed();
            LOG_I("[APP-sim] boot_set_confirmed");
        }
        else
        {
            LOG_I("[APP-sim] boot_set_confirmed not need");
        }
    }
}

int main(int argc, char *argv[])
{
    int rc;
    struct boot_rsp rsp;
    FIH_DECLARE(fih_rc, FIH_FAILURE);

    LOG_I("MCUBoot build");

    struct boot_swap_state st;

    if (argc > 1 && strcmp(argv[1], "write_image_ok") == 0)
    {
        // 请求TEST升级，permanent=0
        rc = boot_set_pending_multi(0, 0);
        printf("[APP-sim] boot_write_image_ok rc=%d\n", rc);
        return 0;
    }

    // 读取状态
    rc = boot_read_swap_state_by_id(FLASH_AREA_IMAGE_PRIMARY(0), &st);
    printf("boot_read_swap_state rc=%d\n", rc);
    printf("swap_type=%s magic=%s copy_done=%s image_ok=%s\n", swap_type_str(st.swap_type),
           magic_str(st.magic), flag_str(st.copy_done), flag_str(st.image_ok));

    FIH_CALL(boot_go, fih_rc, &rsp);

    if (FIH_NOT_EQ(fih_rc, FIH_SUCCESS))
    {
        LOG_I("Unable to find bootable image");
        FIH_PANIC;
    }

    do_boot(&rsp);

    while (1)
        ;
}

/* End main.c */
