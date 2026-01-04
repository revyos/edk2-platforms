## @file SG2044Common.dsc.inc
#
#  Shared DSC content for all SG2044 board variants (SD3-10, SD3-10-LB, SD3-12,
#  SRA3-40, SRA3-40-8, SRA3-40-LB, SRM3-C0). Included from each variant DSC
#  after the variant-specific [Defines] block.
#
#  Variant DSCs must define (in [Defines]) at least:
#    PLATFORM_NAME, PLATFORM_GUID, FLASH_DEFINITION, OUTPUT_DIRECTORY
#    SECURE_BOOT_ENABLE, DEBUG_ON_SERIAL_PORT, TPM2_ENABLE
#    NETWORK_SNP_ENABLE, NETWORK_IP6_ENABLE, NETWORK_TLS_ENABLE,
#    NETWORK_HTTP_BOOT_ENABLE, NETWORK_ISCSI_ENABLE
#    FLASH_ENABLE, ETH_ENABLE, ACPI_ENABLE
#    BMC_ENABLE, BMC_SSIF_ENABLE     (BMC_SSIF_ENABLE is irrelevant when BMC_ENABLE=FALSE)
#    RTC_VIRTUAL                    (TRUE: Virtual RTC; FALSE: Ds1307 RTC)
#    SIETIUM_GOP_ENABLE             (TRUE only on SD3-10 / SD3-10-LB)
#    INTEL_GIG_UNDI_ENABLE          (TRUE only on the SRA3-40 family)
#
#  Variant DSCs additionally provide the per-board identity PCDs
#  (McuExistence, IsServerPlatform, BoardName/Version, ProductName/Version/SN/UUID)
#  and the MCU/SSIF I2C bus PCDs in their own appended sections.
#
#  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# x64 Emulator tuning (in [Defines] so MAU_* fields land in the right section).
#
# The four Network DEFINEs below are common defaults shared by every variant
# (none of the variant DSCs override them). Variant DSCs still set
# NETWORK_SNP_ENABLE individually and the SRA3-40 family additionally sets
# NETWORK_ENABLE / NETWORK_IP4_ENABLE / NETWORK_PXE_BOOT_ENABLE.
#
################################################################################
[Defines]
  DEFINE NETWORK_IP6_ENABLE       = FALSE
  DEFINE NETWORK_TLS_ENABLE       = FALSE
  DEFINE NETWORK_HTTP_BOOT_ENABLE = FALSE
  DEFINE NETWORK_ISCSI_ENABLE     = FALSE

  !if $(X64EMU_ENABLE) == TRUE
    #
    # Use a dedicated native stack for handling emulation.
    #
    MAU_ON_PRIVATE_STACK           = NO

    #
    # Attempt some operation on UEFI implementations without
    # an enabled MMU, by relying on the illegal instruction
    # handler. It won't work well and is only supported on RISC-V.
    # Implies MAU_WRAPPED_ENTRY_POINTS=YES.
    #
    # On by default in RISC-V builds (via INF file).
    #
    MAU_TRY_WITHOUT_MMU            = NO

    #
    # Use an emulated entry point, instead of relying on
    # exception-driven thunking of native to emulated code.
    #
    # On by default in RISC-V builds (via INF file).
    #
    MAU_WRAPPED_ENTRY_POINTS       = NO

    #
    # Handle unexpected/non-linear control flow by native code,
    # that can result in a resource leak inside the emulator.
    # On by default in DEBUG builds (via INF file).
    #
    MAU_CHECK_ORPHAN_CONTEXTS      = NO

    #
    # For maximum performance, don't periodically bail out
    # of emulation. This is only useful for situations where
    # you know the executed code won't do tight loops polling
    # on some memory location updated by an event.
    #
    MAU_EMU_TIMEOUT_NONE           = NO

    #
    # If you want to support x64 UEFI boot service drivers
    # and applications, say YES. Saying NO doesn't make sense
    # for the AARCH64 build.
    #
    MAU_SUPPORTS_X64_BINS          = YES

    #
    # If you want to support AArch64 UEFI boot service drivers
    # and applications, say YES. Not available for the AARCH64
    # build.
    #
    MAU_SUPPORTS_AARCH64_BINS      = NO

    #
    # Say YES if you want to ignore all port I/O writes (reads
    # returning zero), instead of forwarding to EFI_CPU_IO2_PROTOCOL.
    #
    # Useful for testing on UEFI DEBUG builds that use the
    # BaseIoLibIntrinsic (IoLibNoIo.c) implementation.
    #
    MAU_EMU_X64_RAZ_WI_PIO         = NO

    #
    # Seems to work well even when building on small machines.
    #
    UC_LTO_JOBS                    = auto
  !endif

[BuildOptions]
!ifdef $(SOURCE_DEBUG_ENABLE)
  GCC:*_*_RISCV64_GENFW_FLAGS    = --keepexceptiontable
!endif

#
# Force PE/COFF sections to be aligned at 4KB boundaries to support page level protection
#
[BuildOptions.common.EDKII.DXE_CORE,BuildOptions.common.EDKII.DXE_DRIVER,BuildOptions.common.EDKII.UEFI_DRIVER,BuildOptions.common.EDKII.UEFI_APPLICATION]
  GCC:*_*_*_DLINK_FLAGS = -z common-page-size=0x1000
  MSFT: *_*_*_DLINK_FLAGS = /ALIGN:4096

[BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER]
  GCC:  *_*_*_DLINK_FLAGS = -z common-page-size=0x1000
  MSFT: *_*_*_DLINK_FLAGS = /ALIGN:4096

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibRiscVSbiLib/BaseSerialPortLibRiscVSbiLibRam.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLibDevicePathProtocol/UefiDevicePathLibDevicePathProtocol.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  SortLib|MdeModulePkg/Library/BaseSortLib/BaseSortLib.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  FileExplorerLib|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  BootLogoLib|MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  PlatformBmPrintScLib|OvmfPkg/Library/PlatformBmPrintScLib/PlatformBmPrintScLib.inf
  FdtLib|MdePkg/Library/BaseFdtLib/BaseFdtLib.inf
  VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  ConfigUtilsLib|Silicon/Sophgo/Library/ConfigUtilsLib/ConfigUtilsLib.inf
  SmbiosInformationLib|Silicon/Sophgo/SG2044Pkg/Library/SmbiosInformation/SmbiosInformationLib.inf
  PcieSlotInfoLib|Platform/Sophgo/SG2044Pkg/Drivers/$(PLATFORM_NAME)/SlotTable/SlotTable.inf
!ifdef $(SOURCE_DEBUG_ENABLE)
  PeCoffExtraActionLib|SourceLevelDebugPkg/Library/PeCoffExtraActionLibDebug/PeCoffExtraActionLibDebug.inf
  DebugCommunicationLib|SourceLevelDebugPkg/Library/DebugCommunicationLibSerialPort/DebugCommunicationLibSerialPort.inf
!else
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
!endif

  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  ImagePropertiesRecordLib|MdeModulePkg/Library/ImagePropertiesRecordLib/ImagePropertiesRecordLib.inf

  VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf

!if $(HTTP_BOOT_ENABLE) == TRUE
  HttpLib|MdeModulePkg/Library/DxeHttpLib/DxeHttpLib.inf
!endif

  # ACPI not supported yet.
  # S3BootScriptLib|MdeModulePkg/Library/PiDxeS3BootScriptLib/DxeS3BootScriptLib.inf
  SmbusLib|MdePkg/Library/BaseSmbusLibNull/BaseSmbusLibNull.inf
  OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  AcpiLib|EmbeddedPkg/Library/AcpiLib/AcpiLib.inf

  #
  # ipmi ssif / serial manageability libraries (BMC)
  #
!if $(BMC_ENABLE) == TRUE
  PlatformBmcReadyLib|ManageabilityPkg/Library/PlatformBmcReadyLibNull/PlatformBmcReadyLibNull.inf
  ManageabilityTransportHelperLib|ManageabilityPkg/Library/BaseManageabilityTransportHelperLib/BaseManageabilityTransportHelper.inf
  SmbusLib|MdePkg/Library/DxeSmbusLib/DxeSmbusLib.inf
  IpmiLib|MdeModulePkg/Library/DxeIpmiLibIpmiProtocol/DxeIpmiLibIpmiProtocol.inf
  IpmiCommandLib|ManageabilityPkg/Library/IpmiCommandLib/IpmiCommandLib.inf
!if $(BMC_SSIF_ENABLE) == TRUE
  ManageabilityTransportLib|ManageabilityPkg/Library/ManageabilityTransportSsifLib/Dxe/DxeManageabilityTransportSsif.inf
!else
  IpmiFruInfoLib|Silicon/Sophgo/SG2044Pkg/Library/IpmiFruInfo/IpmiFruInfoLib.inf
  ManageabilityTransportLib|edk2-platforms/Silicon/Sophgo/Library/SophgoManageabilityTransportSerialLib/Dxe/DxeManageabilityTransportSerial.inf
!endif
  SophgoNs16550Lib|edk2-platforms/Silicon/Sophgo/Library/SophgoNs16550Lib/SophgoNs16550.inf
!endif

!if $(TPM2_ENABLE) == TRUE
  Tpm2CommandLib|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tcg2PpVendorLib|SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
!endif

[LibraryClasses.common]
  #
  # Secure Boot dependencies
  #
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  AuthVariableLib|SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf
  SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
  SecureBootVariableProvisionLib|SecurityPkg/Library/SecureBootVariableProvisionLib/SecureBootVariableProvisionLib.inf
  PlatformPKProtectionLib|SecurityPkg/Library/PlatformPKProtectionLibVarPolicy/PlatformPKProtectionLibVarPolicy.inf
  PlatformSecureLib|OvmfPkg/Library/PlatformSecureLib/PlatformSecureLib.inf
!else
  TpmMeasurementLib|MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
!endif

!ifdef $(DEBUG_ON_SERIAL_PORT)
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!endif

  # RISC-V Architectural Libraries
  RiscVSbiLib|MdePkg/Library/BaseRiscVSbiLib/BaseRiscVSbiLib.inf
  RiscVMmuLib|UefiCpuPkg/Library/BaseRiscVMmuLib/BaseRiscVMmuLib.inf

  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
!if $(RTC_VIRTUAL) == TRUE
  RealTimeClockLib|EmbeddedPkg//Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
!else
  #
  # Ds1307 RTC Library
  #
  RealTimeClockLib|Silicon/Sophgo/Library/Ds1307RealTimeClockLib/Ds1307RealTimeClockLib.inf
!endif

  # Flattened Device Tree (FDT) access library
  FdtLib|MdePkg/Library/BaseFdtLib/BaseFdtLib.inf
  DtPlatformDtbLoaderLib|EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf

  # PCIe dependencies
  PciHostBridgeLib|Silicon/Sophgo/SG2044Pkg/Library/PciHostBridgeLib/PciHostBridgeLib.inf
  PciSegmentLib|Silicon/Sophgo/SG2044Pkg/Library/PciSegmentLib/PciSegmentLib.inf
  PciPlatformLib|Silicon/Sophgo/SG2044Pkg/Library/PciPlatformLib/PciPlatformLib.inf

  # Nor Flash Library
  NorFlashInfoLib|EmbeddedPkg/Library/NorFlashInfoLib/NorFlashInfoLib.inf

  # Hash Password
  HashPasswordLib|Silicon/Sophgo/Library/HashPasswordLib/HashPasswordLib.inf

  EfuseLib|Silicon/Sophgo/Library/EfuseLib/EfuseLib.inf

  #
  # Random Generator Library
  #
  TrngLib|Silicon/Sophgo/Library/TrngLib/TrngLib.inf
  DwGpioLib|Silicon/Sophgo/Library/DwGpioLib/DwGpioLib.inf
  RngLib|Silicon/Sophgo/Library/RngLib/RngLib.inf

  ResetSystemLib|OvmfPkg/RiscVVirt/Library/ResetSystemLib/BaseResetSystemLib.inf
  DmaLib|EmbeddedPkg/Library/NonCoherentDmaLib/NonCoherentDmaLib.inf

[LibraryClasses.common.SEC]
  TimerLib|UefiCpuPkg/Library/BaseRiscV64CpuTimerLib/BaseRiscV64CpuTimerSecLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
  PlatformSecLib|UefiCpuPkg/Library/PlatformSecLibNull/PlatformSecLibNull.inf

!ifdef $(SOURCE_DEBUG_ENABLE)
  DebugAgentLib|SourceLevelDebugPkg/Library/DebugAgent/SecPeiDebugAgentLib.inf
!endif

[LibraryClasses.common.PEI_CORE]
  TimerLib|UefiCpuPkg/Library/BaseRiscV64CpuTimerLib/BaseRiscV64CpuTimerSecLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibRiscVSbiLib/BaseSerialPortLibRiscVSbiLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf

!ifdef $(SOURCE_DEBUG_ENABLE)
  DebugAgentLib|SourceLevelDebugPkg/Library/DebugAgent/SecPeiDebugAgentLib.inf
!endif

[LibraryClasses.common.PEIM]
  TimerLib|UefiCpuPkg/Library/BaseRiscV64CpuTimerLib/BaseRiscV64CpuTimerSecLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibRiscVSbiLib/BaseSerialPortLibRiscVSbiLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

!ifdef $(SOURCE_DEBUG_ENABLE)
  DebugAgentLib|SourceLevelDebugPkg/Library/DebugAgent/SecPeiDebugAgentLib.inf
!endif

[LibraryClasses.common.DXE_CORE,LibraryClasses.common.DXE_DRIVER,LibraryClasses.common.DXE_RUNTIME_DRIVER,LibraryClasses.common.UEFI_DRIVER,LibraryClasses.common.UEFI_APPLICATION]
  TimerLib|UefiCpuPkg/Library/BaseRiscV64CpuTimerLib/BaseRiscV64CpuTimerLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeCpuExceptionHandlerLib.inf

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
!ifdef $(SOURCE_DEBUG_ENABLE)
  DebugAgentLib|SourceLevelDebugPkg/Library/DebugAgent/DxeDebugAgentLib.inf
!endif

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/RuntimeDxeReportStatusCodeLib/RuntimeDxeReportStatusCodeLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf
!endif
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLibRuntimeDxe.inf

[LibraryClasses.common.UEFI_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf

[LibraryClasses.common.DXE_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
!ifdef $(SOURCE_DEBUG_ENABLE)
  DebugAgentLib|SourceLevelDebugPkg/Library/DebugAgent/DxeDebugAgentLib.inf
!endif
  PlatformBootManagerLib|Silicon/Sophgo/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  PlatformMemoryTestLib|Platform/RISC-V/PlatformPkg/Library/PlatformMemoryTestLibNull/PlatformMemoryTestLibNull.inf
  PlatformUpdateProgressLib|Platform/RISC-V/PlatformPkg/Library/PlatformUpdateProgressLibNull/PlatformUpdateProgressLibNull.inf

[LibraryClasses.common.UEFI_APPLICATION]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform.
#
################################################################################
[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSupportUefiDecompress|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  #
  # Activate AcpiSdtProtocol
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE

[PcdsFeatureFlag.common]
  ## Indicates if S3 performance data will be supported in ACPI FPDT table.
  #   TRUE  - S3 performance data will be supported in ACPI FPDT table.
  #   FALSE - S3 performance data will not be supported in ACPI FPDT table.
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwarePerformanceDataTableS3Support|FALSE
  #
  # No CSM on RISC-V: do not downgrade PMem64 BARs to PMem32 for devices with Option ROM.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDegradeResourceForOptionRom|FALSE

[PcdsFixedAtBuild]
!if $(ACPI_ENABLE) == TRUE
  gUefiRiscVPlatformPkgTokenSpaceGuid.PcdForceNoAcpi|FALSE
!else
  gUefiRiscVPlatformPkgTokenSpaceGuid.PcdForceNoAcpi|TRUE
!endif

  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseMemory|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeMemorySize|1
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdRiscVFeatureOverride|0x7
  gEfiMdePkgTokenSpaceGuid.PcdMaximumGuidedExtractHandler|0x10
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxHardwareErrorVariableSize|0x8000
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0xe000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000

!if $(BMC_SSIF_ENABLE) == TRUE
  # ssif bmc slave addr
  gEfiMdePkgTokenSpaceGuid.PcdIpmiSsifSmbusSlaveAddr|0x20
!endif

  gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress|0x0

  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x10000

!if $(TARGET) == RELEASE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000002
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2B
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8000004F
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2B
!endif

!ifdef $(SOURCE_DEBUG_ENABLE)
  gEfiSourceLevelDebugPkgTokenSpaceGuid.PcdDebugLoadImageMethod|0x2
!endif

!if $(SECURE_BOOT_ENABLE) == TRUE
  # override the default values from SecurityPkg to ensure images from all sources are verified in secure boot
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif

  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"1.0 for SOPHGO SG2044"

  #
  # F2 for UI APP
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x21, 0xaa, 0x2c, 0x46, 0x14, 0x76, 0x03, 0x45, 0x83, 0x6e, 0x8a, 0xb6, 0xf4, 0x66, 0x23, 0x31 }

  #
  # Optional feature to help prevent EFI memory map fragments
  # Turned on and off via: PcdPrePiProduceMemoryTypeInformationHob
  # Values are in EFI Pages (4K). DXE Core will make sure that
  # at least this much of each type of memory can be allocated
  # from a single memory range. This way you only end up with
  # maximum of two fragments for each type in the memory map
  # (the memory used, and the free memory that was prereserved
  # but not used).
  #
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
!if $(SECURE_BOOT_ENABLE) == TRUE
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|600
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1500
!else
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|300
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|150
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1000
!endif
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|6000
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|20
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0

  #
  # Sv48 (a RISC-V privileged-architecture paging mode) provides a 48-bit
  # virtual address space. The SG2044 SoC uses 47 bits of physical address
  # space; the max physical address is 0xFFFFFFFFFF in the SoC system map.
  #
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuMemorySize|47
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|40

  #
  # Control the maximum SATP mode that MMU allowed to use.
  # 0 - Bare mode.
  # 8 - 39bit mode.
  # 9 - 48bit mode.
  # 10 - 57bit mode.
  #
  gUefiCpuPkgTokenSpaceGuid.PcdCpuRiscVMmuMaxSatpMode|9

  #
  # Terminal Type
  # 0 - PC-ANSI Serial Console.
  # 1 - VT-100 Serial Console.
  # 2 - VT-100+ Serial Console.
  # 3 - VT-UTF8 Serial Console.
  # 4 - Tty Terminal Serial Console.
  # 5 - Linux Terminal Serial Console.
  # 6 - Xterm R6 Serial Console.
  # 7 - VT-400 Serial Console.
  # 8 - SCO Terminal Serial Console.
  #
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|4

  #
  # Variable store - default values
  # 64KB + 64KB + 64KB
  # Flash Offset: 32MB
  #
!if $(FLASH_ENABLE) == TRUE
  gSophgoTokenSpaceGuid.PcdSPIFMC0Base|0x7001000000
  gSophgoTokenSpaceGuid.PcdSPIFMC1Base|0x7005000000
  gSophgoTokenSpaceGuid.PcdSpifmcDmmrEnable|TRUE
  gSophgoTokenSpaceGuid.PcdFlashPartitionTableAddress|0x80000
  gSophgoTokenSpaceGuid.PcdFdOffset|0x600000
!endif
  gSophgoTokenSpaceGuid.PcdMisa|0x00B4112F
  #
  # MCU / SSIF I2C bus numbers are board-specific: see the variant DSC.
  #
  gSophgoTokenSpaceGuid.PcdRtcI2cBusNum0|2
  gSophgoTokenSpaceGuid.PcdRtcI2cBusNum1|3

  gUefiCpuPkgTokenSpaceGuid.PcdCpuCoreCrystalClockFrequency|50000000

  gSophgoTokenSpaceGuid.PcdEfuseControllerNum|2
  gSophgoTokenSpaceGuid.PcdEfuse0Base|0x7040000000
  gSophgoTokenSpaceGuid.PcdEfuse1Base|0x7040001000
  gSophgoTokenSpaceGuid.PcdEfuseNumAddrBits|8
  gSophgoTokenSpaceGuid.PcdEfuseNumCells|128
  gSophgoTokenSpaceGuid.PcdEfuseCellWidth|4

  #
  # 1. PcdEfuseWriteEnableGpio set to TRUE indicates that writing data to
  #    eFuse requires configuring the GPIO level.
  # 2. PcdEfuseWriteEnableGpioPin indicates the GPIO number that needs to
  #    be configured.
  # 3. PcdEfuseIsGpioHighToEnableWrite set to TRUE indicates that eFuse
  #    writing is enabled when the GPIO level is set to high.
  #
  gSophgoTokenSpaceGuid.PcdEfuseWriteEnableGpio|TRUE
  gSophgoTokenSpaceGuid.PcdEfuseWriteEnableGpioPin|18
  gSophgoTokenSpaceGuid.PcdEfuseIsGpioHighToEnableWrite|TRUE

!if $(ETH_ENABLE) == TRUE
  gSophgoTokenSpaceGuid.PcdPhyResetGpio|TRUE
  gSophgoTokenSpaceGuid.PcdPhyResetGpioPin|28
!endif

[PcdsFixedAtBuild.common]
  gSophgoTokenSpaceGuid.PcdSDIOSourceClockFrequency|400000000
  gSophgoTokenSpaceGuid.PcdSDIOTransmissionClockFrequency|25000000
  gSophgoTokenSpaceGuid.PcdTrngBase|0x7040020000
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x7030001000
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialClockRate|500000000
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialBaudRate|115200
  gSophgoTokenSpaceGuid.PcdServerNamePrefix|L"SR"
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreMaxPeiStackSize|0x10000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMigrateTemporaryRamFirmwareVolumes|TRUE

!if $(TPM2_ENABLE) == TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmPlatformClass|0x02
  gEfiSecurityPkgTokenSpaceGuid.PcdStatusCodeSubClassTpmDevice|0x010E0000
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2InitializationPolicy|1
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2SelfTestPolicy|1
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2ScrtmPolicy|0
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmAutoDetection|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdTcgPfpMeasurementRevision|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdImageProtectionPolicy|0x00000000
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmPhysicalPresence|TRUE
  gSophgoTokenSpaceGuid.PcdTpm2GpioBaseAddress|0x704000b000
  gSophgoTokenSpaceGuid.PcdTpm2SpiBaseAddress|0x7030004000
  gSophgoTokenSpaceGuid.PcdTopBaseAddress|0x7050000000
!endif

# sophgo,sg2044-dwcmshc set
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSdControllerCount|1
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSdBaseAddress|0x703000b000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSdRegSize|0x2000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSdBusWidth|4
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSdClockFrequency|50000000

# snps,designware-i2c set
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdI2cControllerCount|4
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdI2cBaseAddresses|{0x00,0x50,0x00,0x40,0x70,0x00,0x00,0x00, 0x00,0x60,0x00,0x40,0x70,0x00,0x00,0x00, 0x00,0x70,0x00,0x40,0x70,0x00,0x00,0x00, 0x00,0x80,0x00,0x40,0x70,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdI2cFrequencies|{0xA0,0x86,0x01,0x00, 0xA0,0x86,0x01,0x00, 0xA0,0x86,0x01,0x00, 0xA0,0x86,0x01,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdDefaultI2cSpeed|100000

# snps,dw-apb-gpio set
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdGpioControllerCount|3
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdGpioBaseAddresses|{0x00,0x90,0x00,0x40,0x70,0x00,0x00,0x00, 0x00,0xA0,0x00,0x40,0x70,0x00,0x00,0x00, 0x00,0xB0,0x00,0x40,0x70,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdGpioPinsPerController|32

# snps,dw-apb-ssi set
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSpiControllerCount|1
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSpiBaseAddresses|0x7030004000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSpiBaseAddresses|{0x00,0x40,0x00,0x30,0x70,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSpiClockFrequencies[0]|250000000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSpiClockFrequencies|{0x80,0xB2,0xE6,0x0E}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdSpiRxSampleDelays|{0x00,0x00,0x00,0x00}

# sg,efuse set
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseControllerNum|2
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseBase[0]|0x7040000000
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseBase[1]|0x7040001000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseBase|{0x00,0x00,0x00,0x40,0x70,0x00,0x00,0x00, 0x00,0x10,0x00,0x40,0x70,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseNumAddrBits|8
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseNumAddrBits|{0x08,0x00,0x00,0x00, 0x08,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseNumCells|128
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseNumCells|{0xC0,0x1F,0x00,0x00, 0xC0,0x1F,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdEfuseCellWidth|{0x04,0x00,0x00,0x00, 0x04,0x00,0x00,0x00}

# memory
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdMemoryBaseAddress|0x80000000
  # 128G
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdMemorySize|0x2000000000

# Cpu
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuCount|64
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuReg|{0x00,0x00,0x00,0x00, 0x01,0x00,0x00,0x00, 0x02,0x00,0x00,0x00, 0x03,0x00,0x00,0x00, 0x04,0x00,0x00,0x00, 0x05,0x00,0x00,0x00, 0x06,0x00,0x00,0x00, 0x07,0x00,0x00,0x00, 0x08,0x00,0x00,0x00, 0x09,0x00,0x00,0x00, 0x0A,0x00,0x00,0x00, 0x0B,0x00,0x00,0x00, 0x0C,0x00,0x00,0x00, 0x0D,0x00,0x00,0x00, 0x0E,0x00,0x00,0x00, 0x0F,0x00,0x00,0x00, 0x10,0x00,0x00,0x00, 0x11,0x00,0x00,0x00, 0x12,0x00,0x00,0x00, 0x13,0x00,0x00,0x00, 0x14,0x00,0x00,0x00, 0x15,0x00,0x00,0x00, 0x16,0x00,0x00,0x00, 0x17,0x00,0x00,0x00, 0x18,0x00,0x00,0x00, 0x19,0x00,0x00,0x00, 0x1A,0x00,0x00,0x00, 0x1B,0x00,0x00,0x00, 0x1C,0x00,0x00,0x00, 0x1D,0x00,0x00,0x00, 0x1E,0x00,0x00,0x00, 0x1F,0x00,0x00,0x00, 0x20,0x00,0x00,0x00, 0x21,0x00,0x00,0x00, 0x22,0x00,0x00,0x00, 0x23,0x00,0x00,0x00, 0x24,0x00,0x00,0x00, 0x25,0x00,0x00,0x00, 0x26,0x00,0x00,0x00, 0x27,0x00,0x00,0x00, 0x28,0x00,0x00,0x00, 0x29,0x00,0x00,0x00, 0x2A,0x00,0x00,0x00, 0x2B,0x00,0x00,0x00, 0x2C,0x00,0x00,0x00, 0x2D,0x00,0x00,0x00, 0x2E,0x00,0x00,0x00, 0x2F,0x00,0x00,0x00, 0x30,0x00,0x00,0x00, 0x31,0x00,0x00,0x00, 0x32,0x00,0x00,0x00, 0x33,0x00,0x00,0x00, 0x34,0x00,0x00,0x00, 0x35,0x00,0x00,0x00, 0x36,0x00,0x00,0x00, 0x37,0x00,0x00,0x00, 0x38,0x00,0x00,0x00, 0x39,0x00,0x00,0x00, 0x3A,0x00,0x00,0x00, 0x3B,0x00,0x00,0x00, 0x3C,0x00,0x00,0x00, 0x3D,0x00,0x00,0x00, 0x3E,0x00,0x00,0x00, 0x3F,0x00,0x00,0x00}

#
# MCU existence / IsServerPlatform / board+product identity are board-specific:
# they live in each variant DSC's appended [PcdsFixedAtBuild.common] section.
#

#pcie
!include Platform/Sophgo/SG2044Pkg/$(PLATFORM_NAME)/PcieHostBridge.dsc.inc

# Smbios Acpi
  # [BIOS]
  # BIOS Version shown in SMBIOS Type0 and the Setup Information page.
  # Bump this manually on release (keep in sync with the pack -o 0x0 version).
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdBiosVersionString|L"2.0.0"
  # [DDR]
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdDdrType|L"LPDDR5x"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdDdrRate|8533000000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdDdrRank|2
  # [CPU]
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProcessorVersion|L"SG2044"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuFrequencyHz|2600000000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuL1ICacheSizeBytes|0x10000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuL1DCacheSizeBytes|0x10000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuL2CacheSizeBytes|0x200000
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdCpuL3CacheSizeBytes|0x4000000
  # [board]
  # PcdBoardName / PcdBoardVersion - variant DSC
  # [product]
  # PcdProductName / PcdProductVersion / PcdProductSN / PcdProductUUID - variant DSC
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductManufacturer|L"SOPHGO"

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsDynamicDefault]
!if $(FLASH_ENABLE) == FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE
!endif

  #gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion|0x0208
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosDocRev|0x0

  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|10
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOutDefault|10

  #
  # Set video resolution for boot options and for text setup.
  # PlatformDxe can set the former at runtime.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|800
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|640
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|480
  #gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0
  #gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0

!if $(FLASH_ENABLE) == TRUE
  gSophgoTokenSpaceGuid.PcdFlashVariableOffset|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase64|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase64|0x0
!endif

!if $(TPM2_ENABLE) == TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmInitializationPolicy|1
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmInstanceGuid|{0x5a, 0xf2, 0x6b, 0x28, 0xc3, 0xc2, 0x8c, 0x40, 0xb3, 0xb4, 0x25, 0xe6, 0x75, 0x8b, 0x73, 0x17}
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmBaseAddress|0xFED40000
  gEfiSecurityPkgTokenSpaceGuid.PcdTcgPhysicalPresenceInterfaceVer|"1.3"
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2AcpiTableRev|3
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2CurrentIrqNum|0
  gEfiSecurityPkgTokenSpaceGuid.PcdActiveTpmInterfaceType|0x0
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2AcpiTableLaml|0
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2AcpiTableLasa|0
  gEfiSecurityPkgTokenSpaceGuid.PcdFirmwareDebuggerInitialized|FALSE
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2HashMask|0x00000002
  gEfiSecurityPkgTokenSpaceGuid.PcdTcg2HashAlgorithmBitmap|3
!endif

!if $(BMC_SSIF_ENABLE) == TRUE
# ssif
  gManageabilityPkgTokenSpaceGuid.PcdSendSmbiosOnChanged|FALSE
  gManageabilityPkgTokenSpaceGuid.PcdBmcSmbiosBlobTransferId|"/smbios"
!endif

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform.
#
################################################################################
[Components]

  #
  # SEC Phase modules
  #
  Silicon/Sophgo/Core/Sec/SecMain.inf  {
    <LibraryClasses>
      ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
      LzmaDecompressLib|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
      PrePiLib|Silicon/Sophgo/Library/PrePiLib/PrePiLib.inf
      HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
      PrePiHobListPointerLib|OvmfPkg/RiscVVirt/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
      MemoryAllocationLib|EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  }

  #
  # PEI Phase modules
  #
  MdeModulePkg/Core/Pei/PeiMain.inf  {
    <LibraryClasses>
    PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  }

  Silicon/Sophgo/Modules/CpuPei/CpuPei.inf
  Silicon/Sophgo/Modules/DwGpioPei/DwGpioPei.inf
  Silicon/Sophgo/SG2044Pkg/Modules/PcieInitPei/PcieInitPei.inf
  Silicon/Sophgo/Modules/MemoryInitPei/MemoryInitPei.inf {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  Silicon/Sophgo/Modules/PlatformPei/PlatformInitPei.inf {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    PeiResourcePublicationLib|MdePkg/Library/PeiResourcePublicationLib/PeiResourcePublicationLib.inf
  }

  Silicon/Sophgo/Modules/FvParserPei/FvParserPei.inf {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    PrePiLib|Silicon/Sophgo/Library/PrePiLib/PrePiLib.inf
  }

  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf  {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  }

  # MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  Silicon/Sophgo/Modules/SpifmcPei/SpiFlashMasterController.inf
  Silicon/Sophgo/Modules/NorFlashPei/NorFlashPei.inf
  Silicon/Sophgo/Modules/VariablePei/VariablePei.inf
  MdeModulePkg/Universal/FaultTolerantWritePei/FaultTolerantWritePei.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf  {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

!if $(TPM2_ENABLE) == TRUE
  SecurityPkg/Tcg/Tcg2Pei/Tcg2Pei.inf  {
    <LibraryClasses>
    Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterPei.inf
    NULL|Silicon/Sophgo/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpmPei.inf
    HashLib|SecurityPkg/Library/HashLibTpm2/HashLibTpm2.inf
  }
  Silicon/Sophgo/Drivers/Tcg2Config/Tcg2ConfigPei.inf {
    <LibraryClasses>
    MmUnblockMemoryLib|MdePkg/Library/MmUnblockMemoryLib/MmUnblockMemoryLibNull.inf
    Tpm12CommandLib|SecurityPkg/Library/Tpm12CommandLib/Tpm12CommandLib.inf
    Tpm12DeviceLib|SecurityPkg/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
  }
  SecurityPkg/Tcg/PhysicalPresencePei/PhysicalPresencePei.inf
!endif

  #
  # DXE Phase modules
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  }

  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf  {
   <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  Platform/RISC-V/PlatformPkg/Drivers/PlatformHasAcpiDtDxe/PlatformHasAcpiDtDxe.inf
  # EmbeddedPkg/Drivers/FdtClientDxe/FdtClientDxe.inf {
  #   <LibraryClasses>
  #     DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  # }

  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

  UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf {
    <LibraryClasses>
      ResetSystemLib|MdeModulePkg/Library/BaseResetSystemLibNull/BaseResetSystemLibNull.inf
  }
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf {
    <LibraryClasses>
!if $(RTC_VIRTUAL) == TRUE
      # RealTimeClockLib|Silicon/Sophgo/Library/Ds1307RealTimeClockLib/Ds1307RealTimeClockLib.inf
      RealTimeClockLib|EmbeddedPkg//Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
!else
      RealTimeClockLib|Silicon/Sophgo/Library/Ds1307RealTimeClockLib/Ds1307RealTimeClockLib.inf
!endif
  }

  #
  # RISC-V Platform module
  #
!if $(FLASH_ENABLE) == TRUE
  Silicon/Sophgo/SG2044Pkg/Drivers/SpifmcDxe/SpiFlashMasterController.inf
  Silicon/Sophgo/Drivers/NorFlashDxe/NorFlashDxe.inf
  Silicon/Sophgo/Drivers/FlashFvbDxe/FlashFvbDxe.inf
!endif
  Silicon/Sophgo/Drivers/DwI2cDxe/DwI2cDxe.inf
  Silicon/Sophgo/Drivers/MmcDxe/MmcDxe.inf
  Silicon/Sophgo/Drivers/SdHostDxe/SdHostDxe.inf
  Silicon/Sophgo/Drivers/DwSpiDxe/DwSpiDxe.inf
  Silicon/Sophgo/Drivers/DwGpioDxe/DwGpioDxe.inf
!if $(ETH_ENABLE) == TRUE
  Silicon/Sophgo/Drivers/Net/StmmacMdioDxe/StmmacMdioDxe.inf
  Silicon/Sophgo/Drivers/Net/MotorcommPhyDxe/Motorcomm8531PhyDxe.inf
  Silicon/Sophgo/Drivers/Net/DwMac4SnpDxe/DwMac4SnpDxe.inf
!endif

  #
  # RISC-V Core module
  #
  UefiCpuPkg/CpuTimerDxeRiscV64/CpuTimerDxeRiscV64.inf
  UefiCpuPkg/CpuDxeRiscV64/CpuDxeRiscV64.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
      VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  }
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  #
  # Multiple Console IO support
  #
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf

  # Simple TextIn/TextOut for UEFI Terminal
  EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf

  #
  # Graphic Console Support
  #
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
    <LibraryClasses>
       PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }

  #
  # Memory test
  #
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf

  #
  # SMBIOS Support
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  Platform/Sophgo/SG2044Pkg/Drivers/$(PLATFORM_NAME)/SmbiosPlatformDxe.inf

  #
  # PCIe Support
  #
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf {
    <LibraryClasses>
      NULL|Silicon/Sophgo/SG2044Pkg/Library/PciPlatformLib/PciPlatformLib.inf
  }

!if $(INTEL_GIG_UNDI_ENABLE) == TRUE
  #
  # Prebuilt Intel UNDI drivers (native RISCV64 PE32)
  #
  Silicon/Sophgo/SG2044/iPXE/IntelGigUndiRv64/GigUndiDxe.inf
  Silicon/Sophgo/SG2044/iPXE/IntelXGigUndiRv64/XGigUndiDxe.inf
  Silicon/Sophgo/SG2044/iPXE/IntelIceUndiRv64/IceUndiDxe.inf
  Silicon/Sophgo/SG2044/iPXE/IntelI40eUndiRv64/I40eUndiDxe.inf
!endif

  #
  # NVMe Support
  #
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #
  # SATA Support
  #
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Pci/SataControllerDxe/SataControllerDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf

  #
  # Random Number Generator Support
  #
  Silicon/Sophgo/Drivers/RngDxe/RngDxe.inf

  #
  # Hash2 Protocol producer
  #
  SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf

  #
  # Network Support
  #
  !include NetworkPkg/Network.dsc.inc

  #
  # USB Support
  #
  MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Pci/NonDiscoverablePciDeviceDxe/NonDiscoverablePciDeviceDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf

  #
  # Emulator for x64 OpRoms, etc.
  #
  !if $(X64EMU_ENABLE) == TRUE
    !include MultiArchUefiPkg/MultiArchUefiPkg.dsc.inc
  !endif

  #
  # ASPEED AST2500 GOP driver
  #
  Drivers/ASpeed/ASpeedGopBinPkg/ASpeedAst2500GopDxe.inf

!if $(SIETIUM_GOP_ENABLE) == TRUE
  #
  # Sietium GPU GOP driver
  #
  Drivers/Sietium/SietiumGopDxe.inf
!endif

  #
  # iPXE Application
  #
  Silicon/Sophgo/SG2044/iPXE/iPXE.inf
  MdeModulePkg/Universal/LoadFileOnFv2/LoadFileOnFv2.inf

  #
  # ipmi ssif / serial smbus driver (BMC)
  #
!if $(BMC_ENABLE) == TRUE
  Silicon/Sophgo/Drivers/SmbusHcDxe/SmbusHcDxe.inf
  ManageabilityPkg/Universal/IpmiProtocol/Dxe/IpmiProtocolDxe.inf
!if $(BMC_SSIF_ENABLE) == TRUE
  Silicon/Sophgo/SG2044Pkg/Drivers/BmcConfigDxe/BmcConfig.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/IpmiBootDxe/IpmiBootDxe.inf
  ManageabilityPkg/Universal/IpmiSmbiosTransferDxe/IpmiSmbiosTransferDxe.inf
  ManageabilityPkg/Universal/IpmiBlobTransferDxe/IpmiBlobTransferDxe.inf
!else
  Silicon/Sophgo/SG2044Pkg/Drivers/IpmiFruInfoDxe/IpmiFruInfoDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/BmcConfigDxe/BmcConfig.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/HostHwInfoReportToBmcDxe/HostHwInfoReportToBmcDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/IpmiBootDxe/IpmiBootDxe.inf
!endif
!endif

  #
  # FAT filesystem + GPT/MBR partitioning + UDF filesystem
  #
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf

  #
  # UEFI Application (Shell Embedded Boot Loader)
  #
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
      NULL|Silicon/Sophgo/Applications/EfuseTool/EfuseTool.inf
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
!if $(NETWORK_IP6_ENABLE) == TRUE
      NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
!endif

    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
  }

  OvmfPkg/LinuxInitrdDynamicShellCommand/LinuxInitrdDynamicShellCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
    <LibraryClasses>
      ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
      SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  }

  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
!if $(SECURE_BOOT_ENABLE) == TRUE
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!endif
!if $(TPM2_ENABLE) == TRUE
      NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
!endif
  }

!if $(TPM2_ENABLE) == TRUE
  Silicon/Sophgo/Drivers/Tcg2Dxe/Tcg2Dxe.inf  {
    <LibraryClasses>
    Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
    NULL|Silicon/Sophgo/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpmDxe.inf
    HashLib|SecurityPkg/Library/HashLibTpm2/HashLibTpm2.inf
    PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
    Tcg2PhysicalPresenceLib|SecurityPkg/Library/DxeTcg2PhysicalPresenceLib/DxeTcg2PhysicalPresenceLib.inf
  }
  Silicon/Sophgo/Drivers/Tcg2Config/Tcg2ConfigDxe.inf {
    <LibraryClasses>
    Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
    NULL|Silicon/Sophgo/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpmDxe.inf
    PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
    Tcg2PhysicalPresenceLib|SecurityPkg/Library/DxeTcg2PhysicalPresenceLib/DxeTcg2PhysicalPresenceLib.inf
  }
!endif

!if $(SECURE_BOOT_ENABLE) == TRUE
  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
  SecurityPkg/EnrollFromDefaultKeysApp/EnrollFromDefaultKeysApp.inf
  SecurityPkg/VariableAuthenticated/SecureBootDefaultKeysDxe/SecureBootDefaultKeysDxe.inf
!endif

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf {
    <LibraryClasses>
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
  Silicon/Sophgo/Drivers/LogoDxe/LogoDxe.inf
  MdeModulePkg/Application/BootManagerMenuApp/BootManagerMenuApp.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/UiApp/UiApp.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/BootManagerUiLib/BootManagerUiLib.inf
      NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
      NULL|MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
  }
  Silicon/Sophgo/SG2044Pkg/Drivers/SetDateAndTimeDxe/SetDateAndTimeDxe.inf

!if $(FLASH_ENABLE) == TRUE
  Silicon/Sophgo/SG2044Pkg/Drivers/FirmwareManagerUiDxe/FirmwareManagerUiDxe.inf
!endif
  Silicon/Sophgo/SG2044Pkg/Drivers/InformationDxe/InformationDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/PasswordConfigDxe/PasswordConfigUiDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/ReserveMemoryDxe/ReserveMemoryDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/DebugConfigDxe/DebugConfigDxe.inf

  #
  # ACPI Support
  #
!if $(ACPI_ENABLE) == TRUE
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  Silicon/Sophgo/SG2044Pkg/AcpiTables/$(PLATFORM_NAME)/$(PLATFORM_NAME)-AcpiTables.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableDxe/FirmwarePerformanceDxe.inf {
    <LibraryClasses>
      LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  }
!endif
