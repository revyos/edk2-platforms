## @file SD3-10.dsc
#
#  RISC-V EFI on SOPHGO SD3-10 RISC-V platform
#
#  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = SD3-10
  PLATFORM_GUID                  = D98574D9-110C-4804-BA4E-0DE396A17A0E
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001c
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = RISCV64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Sophgo/SG2044Pkg/SD3-10/SD3-10.fdf

  #
  # Enable below options may cause build error or may not work on
  # the initial version of RISC-V package
  # Defines for default states.  These can be changed on the command line.
  # -D FLAG=VALUE
  #
  DEFINE SECURE_BOOT_ENABLE      = TRUE
  DEFINE DEBUG_ON_SERIAL_PORT    = TRUE
  DEFINE TPM2_ENABLE             = FALSE

  #
  # Network definition (IP6/TLS/HTTP_BOOT/ISCSI defaults come from
  # SG2044Common.dsc.inc; SNP is variant-specific here).
  #
  DEFINE NETWORK_SNP_ENABLE       = FALSE

  DEFINE FLASH_ENABLE             = TRUE
  DEFINE ETH_ENABLE               = FALSE
  DEFINE ACPI_ENABLE              = FALSE

  #
  # BMC: FALSE on SD3-10 (no BMC). BMC_SSIF_ENABLE is irrelevant when
  # BMC_ENABLE = FALSE but is set for completeness.
  #
  DEFINE BMC_ENABLE               = FALSE
  DEFINE BMC_SSIF_ENABLE          = FALSE

  #
  # RTC: Ds1307 hardware RTC.
  #
  DEFINE RTC_VIRTUAL              = FALSE

  #
  # Sietium GPU GOP driver (only on SD3-10 / SD3-10-LB).
  #
  DEFINE SIETIUM_GOP_ENABLE       = TRUE

  #
  # Prebuilt Intel UNDI drivers (only on the SRA3-40 family).
  #
  DEFINE INTEL_GIG_UNDI_ENABLE    = FALSE

!include Platform/Sophgo/SG2044Pkg/SG2044Common.dsc.inc

################################################################################
#
# Variant-specific PCDs (board identity + MCU/SSIF I2C bus numbers).
#
################################################################################
[PcdsFixedAtBuild]
  gSophgoTokenSpaceGuid.PcdMCUI2cBus|1
  gSophgoTokenSpaceGuid.PcdSsifI2cBusNum|3

[PcdsFixedAtBuild.common]
  #
  # MCU existence / platform class / board+product identity
  #
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdMcuExistence|TRUE
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdIsServerPlatform|FALSE
  # [board]
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdBoardName|L"SG2044_EVB"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdBoardVersion|L"1.1"
  # [product]
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductName|L"SD3-10"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductVersion|L"1.0"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductSN|L"TYUI7890"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductUUID|L"123e4567-e89b-12d3-a456-426614174000"
