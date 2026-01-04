/** @file
  RISC-V SEC phase module for SOPHGO Platform.

  Copyright (c) 2008 - 2023, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2022, Ventana Micro Systems Inc. All rights reserved.<BR>
  Copyright (c) 2023, Academy of Intelligent Innovation, Shandong Universiy, China.P.R. All rights reserved.<BR>
  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PeilessSec.h"
#include <Ppi/TemporaryRamSupport.h>
#include <Ppi/SecHobData.h>

EFI_STATUS
EFIAPI
TemporaryRamMigration (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN EFI_PHYSICAL_ADDRESS     TemporaryMemoryBase,
  IN EFI_PHYSICAL_ADDRESS     PermanentMemoryBase,
  IN UINTN                    CopySize
  );

STATIC EFI_PEI_TEMPORARY_RAM_SUPPORT_PPI mTemporaryRamSupportPpi = {
  TemporaryRamMigration
};

EFI_STATUS
EFIAPI
GetSecHobData (
  IN CONST EFI_SEC_HOB_DATA_PPI *This,
  OUT EFI_HOB_GENERIC_HEADER    **HobList
  );

STATIC EFI_SEC_HOB_DATA_PPI mSecHobDataPpi = {
  GetSecHobData
};

EFI_PEI_PPI_DESCRIPTOR mPrivateDispatchTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiSecHobDataPpiGuid,
    &mSecHobDataPpi
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiTemporaryRamSupportPpiGuid,
    &mTemporaryRamSupportPpi
  },
};

/** Temporary RAM migration function.

  This function migrates the data from temporary RAM to permanent
  memory.

  @param[in]  PeiServices           PEI service
  @param[in]  TemporaryMemoryBase   Temporary memory base address
  @param[in]  PermanentMemoryBase   Permanent memory base address
  @param[in]  CopySize              Size to copy

**/
EFI_STATUS
EFIAPI
TemporaryRamMigration (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN EFI_PHYSICAL_ADDRESS     TemporaryMemoryBase,
  IN EFI_PHYSICAL_ADDRESS     PermanentMemoryBase,
  IN UINTN                    CopySize
  )
{
  VOID      *OldHeapBase;
  VOID      *NewHeapBase;
  VOID      *OldStackBase;
  VOID      *NewStackBase;
  UINT32     Offset;

  DEBUG ((DEBUG_INFO,
    "%a: Temp Mem Base:0x%Lx, Permanent Mem Base:0x%Lx, CopySize:0x%Lx\n",
    __func__,
    TemporaryMemoryBase,
    PermanentMemoryBase,
    (UINT64)CopySize
    ));

  OldHeapBase = (VOID*)(UINTN)TemporaryMemoryBase;
  NewHeapBase = (VOID*)((UINTN)PermanentMemoryBase + FixedPcdGet32 (PcdTemporaryRamSize));

  OldStackBase = (VOID*)((UINTN)TemporaryMemoryBase + (CopySize - FixedPcdGet32 (PcdTemporaryRamSize)));
  NewStackBase = (VOID*)((UINTN)PermanentMemoryBase);

  CopyMem (NewHeapBase, OldHeapBase, CopySize - FixedPcdGet32 (PcdTemporaryRamSize));   // Migrate Heap
  CopyMem (NewStackBase, OldStackBase,  FixedPcdGet32 (PcdTemporaryRamSize)); // Migrate Stack

  //
  // Relocate PEI Service **
  //
  Offset = (unsigned long)((UINTN)NewStackBase - (UINTN)OldStackBase);
  DEBUG ((DEBUG_INFO, "Relocate: PEI Service at 0x%x offset is 0x%x\n\n", NewStackBase, Offset));

  register uintptr_t a0 asm ("a0") = (uintptr_t)((UINTN)NewStackBase - (UINTN)OldStackBase);
  asm volatile ("add sp, sp, a0"::"r"(a0):);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetSecHobData (
  IN CONST EFI_SEC_HOB_DATA_PPI *This,
  OUT EFI_HOB_GENERIC_HEADER    **HobList
  )
{
  VOID                  *HobStart;
  EFI_PEI_HOB_POINTERS  Hob;

  HobStart = GetHobList ();
  Hob.Raw = (UINT8 *)HobStart;

  if (Hob.Header->HobType == EFI_HOB_TYPE_HANDOFF) {
    DEBUG ((DEBUG_INFO, "Success to find the base PHIT Hob\n"));
    Hob.Raw = GET_NEXT_HOB (Hob);
  }
  *HobList = Hob.Header;

  return EFI_SUCCESS;
}

/**
  Initialize the memory and CPU, setting the boot mode, and platform
  initialization. It also builds the core information HOB.

  @return EFI_SUCCESS     Status.
**/
STATIC
EFI_STATUS
EFIAPI
SecInitializePlatform (
  VOID
  )
{
  EFI_STATUS  Status;
  FIRMWARE_SEC_PERFORMANCE      Performance;
  UINT64                        StartTimeStamp;

  // Store timer value logged at the beginning of firmware image execution
  StartTimeStamp = GetPerformanceCounter();
  Performance.ResetEnd = GetTimeInNanoSecond (StartTimeStamp);

  // Build SEC Performance Data Hob
  BuildGuidDataHob (&gEfiFirmwarePerformanceGuid, &Performance, sizeof (Performance));
  BuildFvHob (PcdGet32 (PcdRiscVDxeFvBase), PcdGet32 (PcdRiscVDxeFvSize));

  Status = PlatformPeimInitialization ();
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/** Transion from SEC phase to PEI phase.

  This function transits to S-mode PEI phase from M-mode SEC phase.

  @param[in]  SecCoreData     SecCore Infomation transfered to PEI core.

**/
VOID
EFIAPI
PeiCore (
  EFI_SEC_PEI_HAND_OFF            *SecCoreData
  )
{
  EFI_PEI_CORE_ENTRY_POINT        PeiCoreEntryPoint;
  EFI_FIRMWARE_VOLUME_HEADER      *BootFv;
  EFI_PEI_PPI_DESCRIPTOR          *EfiPeiPpiDescriptor;

  BootFv = (EFI_FIRMWARE_VOLUME_HEADER *)FixedPcdGet32 (PcdRiscVPeiFvBase);
  LoadPeiEntryPointFromFv (&PeiCoreEntryPoint);

  SecCoreData->DataSize               = sizeof (EFI_SEC_PEI_HAND_OFF);
  SecCoreData->BootFirmwareVolumeBase = BootFv;
  SecCoreData->BootFirmwareVolumeSize = (UINTN)BootFv->FvLength;

  EfiPeiPpiDescriptor = (EFI_PEI_PPI_DESCRIPTOR *)&mPrivateDispatchTable;
  //
  // Transfer the control to the PEI core
  //
  (*PeiCoreEntryPoint)(SecCoreData, EfiPeiPpiDescriptor);
}

/**

  Entry point to the C language phase of SEC. After the SEC assembly
  code has initialized some temporary memory and set up the stack,
  the control is transferred to this function.


  @param[in]  BootHartId         Hardware thread ID of boot hart.
  @param[in]  DeviceTreeAddress  Pointer to Device Tree (DTB)
**/
VOID
NORETURN
EFIAPI
SecStartup (
  IN  UINTN  BootHartId,
  IN  VOID   *DeviceTreeAddress
  )
{
  EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
  EFI_RISCV_FIRMWARE_CONTEXT  FirmwareContext;
  UINT64                      UefiMemoryBase;
  UINT64                      StackBase;
  UINT32                      StackSize;
  EFI_SEC_PEI_HAND_OFF        SecCoreData;

  SerialPortInitialize ();
  //
  // Report Status Code to indicate entering SEC core
  //

  DEBUG ((
    DEBUG_INFO,
    "%a() SecStartup: 0x%lx BootHartId: 0x%x, DeviceTreeAddress=0x%lx\n",
    __func__,
    SecStartup,
    BootHartId,
    DeviceTreeAddress
    ));

  FirmwareContext.BootHartId          = BootHartId;
  FirmwareContext.FlattenedDeviceTree = (UINT64)DeviceTreeAddress;
  SetFirmwareContextPointer (&FirmwareContext);

  StackBase      = (UINT64)FixedPcdGet32 (PcdTemporaryRamBase);
  StackSize      = FixedPcdGet32 (PcdTemporaryRamSize);
  UefiMemoryBase = FixedPcdGet64 (PcdEfiMemoryBottom);

  /*
   *  --------  --> Stack Top, EfiMemoryTop, EfiFreeMemoryTop (PcdTemporaryRamBase)
   * |        |
   * | Stack  |
   * |        |
   *  --------  --> Stack Base (PcdTemporaryRamSize)
   * |        |
   * |        |
   * | EfiMem |
   * |        |
   * |        |
   *  --------  --> EfiMemoryBottom, EfiFreeMemoryBottom (PcdEfiMemoryBottom)
   * |        |
   * |   FW   |
   * |        |
   *  --------  --> FW_BASE_ADDRESS, typically after opensbi
   */

  // Declare the PI/UEFI memory region
  HobList = HobConstructor (
              (VOID *)UefiMemoryBase + SEC_MEMORY_OFFSET,
              StackBase - UefiMemoryBase - SEC_MEMORY_OFFSET,
              (VOID *)UefiMemoryBase + SEC_MEMORY_OFFSET,
              (VOID *)StackBase // The top of the UEFI Memory is reserved for the stacks
              );
  PrePeiSetHobList (HobList);

  SecInitializePlatform ();

  BuildStackHob (StackBase, StackSize);

  //
  // Process all libraries constructor function linked to SecMain.
  //
  ProcessLibraryConstructorList ();

  //transfer the memory info from SEC to PEI phase
  SecCoreData.StackBase = (VOID *)StackBase;
  SecCoreData.StackSize = StackSize;
  SecCoreData.TemporaryRamBase = (VOID *)UefiMemoryBase;
  SecCoreData.TemporaryRamSize = (StackBase + StackSize - UefiMemoryBase);
  SecCoreData.PeiTemporaryRamBase = SecCoreData.TemporaryRamBase;
  SecCoreData.PeiTemporaryRamSize = (SecCoreData.TemporaryRamSize - StackSize);

  PeiCore (&SecCoreData);

  //
  // Should not come here.
  //
  UNREACHABLE ();
}