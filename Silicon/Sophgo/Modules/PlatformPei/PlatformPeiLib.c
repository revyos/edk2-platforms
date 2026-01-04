/** @file
 The library call to pass the device tree to DXE via HOB.

 Copyright (c) 2021, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>
 Copyright (c) 2024, SOPHGO Inc, All rights reserved.<BR>

 SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Include/Library/PrePiLib.h>
#include <libfdt.h>
#include <Guid/FdtHob.h>
#include <Library/BaseRiscVSbiLib.h>

/**
  @param DeviceTreeAddress       Pointer to FDT.
  @retval EFI_SUCCESS            The address of FDT is passed in HOB.
          EFI_UNSUPPORTED        Can't locate FDT.
**/
EFI_STATUS
EFIAPI
PlatformPeimInitialization (
  VOID
  )
{
  VOID                        *Base;
  VOID                        *NewBase;
  UINTN                       FdtSize;
  UINTN                       FdtPages;
  UINT64                      *FdtHobData;
  VOID                        *DeviceTreeAddress;
  EFI_RISCV_FIRMWARE_CONTEXT  *FirmwareContext;

  FirmwareContext = NULL;
  GetFirmwareContextPointer (&FirmwareContext);

  if (FirmwareContext == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Firmware Context is NULL\n",
      __func__
      ));
    return EFI_UNSUPPORTED;
  }

  DeviceTreeAddress = (VOID *)FirmwareContext->FlattenedDeviceTree;
  if (DeviceTreeAddress == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid FDT pointer\n", __func__));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "%a: Build FDT HOB - FDT at address: 0x%lx \n", __func__, DeviceTreeAddress));
  Base = DeviceTreeAddress;
  if (fdt_check_header (Base) != 0) {
    DEBUG ((DEBUG_ERROR, "%a: Corrupted DTB\n", __func__));
    return EFI_UNSUPPORTED;
  }

  FdtSize  = fdt_totalsize (Base);
  FdtPages = EFI_SIZE_TO_PAGES (FdtSize);
  NewBase  = AllocatePages (FdtPages);
  if (NewBase == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Could not allocate memory for DTB\n", __func__));
    return EFI_UNSUPPORTED;
  }

  fdt_open_into (Base, NewBase, EFI_PAGES_TO_SIZE (FdtPages));

  FdtHobData = BuildGuidHob (&gFdtHobGuid, sizeof *FdtHobData);
  if (FdtHobData == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Could not build FDT Hob\n", __func__));
    return EFI_UNSUPPORTED;
  }

  *FdtHobData = (UINTN)NewBase;

  return EFI_SUCCESS;
}
