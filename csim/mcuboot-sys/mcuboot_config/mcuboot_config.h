/*
 * Copyright (c) 2018 Open Source Foundries Limited
 * Copyright (c) 2019 Arm Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Original code taken from mcuboot project at:
 * https://github.com/JuulLabs-OSS/mcuboot
 * Git SHA of the original version: ac55554059147fff718015be9f4bd3108123f50a
 */

#ifndef __MCUBOOT_CONFIG_H__
#define __MCUBOOT_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * This file is also included by the simulator, but we don't want to
 * define anything here in simulator builds.
 *
 * Instead of using mcuboot_config.h, the simulator adds MCUBOOT_xxx
 * configuration flags to the compiler command lines based on the
 * values of environment variables. However, the file still must
 * exist, or bootutil won't build.
 */

/* FIH故障注入防护，仿真环境关闭 */
// #define MCUBOOT_FIH_PROFILE_HIGH

/* ========== 镜像签名与加密：仿真调试全部关闭 ========== */
/* 不启用EC256签名 */
// #define MCUBOOT_SIGN_EC256
/* 不启用RSA签名 */
// #define MCUBOOT_SIGN_RSA
/* 不启用镜像加密 */
// #define MCUBOOT_ENC_IMAGES
// #define MCUBOOT_ENCRYPT_EC256

/* ========== 镜像头部与Flash对齐配置，必须和imgtool参数严格匹配 ========== */
/* MCUboot镜像头部大小，imgtool --header-size 512 */
#define MCUBOOT_HEADER_SIZE 512
/* Flash读写对齐，模拟后端返回align=1，imgtool --align 1 */
#define MCUBOOT_ALIGN_VAL 1

/* ========== Boot核心行为配置 ========== */
#define MCUBOOT_VALIDATE_PRIMARY_SLOT
#define MCUBOOT_VERIFY_IMG_HASH

/* 标准scratch‑swap模式，需要scratch slot ID3 */
// #define MCUBOOT_SWAP_USING_SCRATCH

#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS

// #define MCUBOOT_HW_ROLLBACK_PROT
#define MCUBOOT_BOOTSTRAP

/* ========== 密码学后端：使用TinyCrypt，仅做SHA256哈希校验，无非对称签名 ========== */
// #define MCUBOOT_USE_TINYCRYPT
#define MCUBOOT_USE_MBED_TLS

/* ========== 日志配置 ========== */
/* 开启bootutil日志输出，用于调试 */
#define MCUBOOT_HAVE_LOGGING

/* ========== 镜像数量配置 ========== */
/* 镜像个数：当前单镜像模式 image0 */
#define MCUBOOT_IMAGE_NUMBER 1

/* ========== Slot扇区最大数量 ========== */
/* 每个slot最大支持sector数量，1MB slot /4K sector =256，配置512留余量 */
#define MCUBOOT_MAX_IMG_SECTORS 512

/* ========== 看门狗：仿真环境空实现 ========== */
#define MCUBOOT_WATCHDOG_FEED()                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        /* Do nothing. Simulation no watchdog */                                                                       \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __MCUBOOT_CONFIG_H__ */
