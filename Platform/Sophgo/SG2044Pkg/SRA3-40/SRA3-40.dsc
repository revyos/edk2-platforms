## @file SRA3-40.dsc
#
#  RISC-V EFI on SOPHGO SRA3-40 RISC-V platform
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
  PLATFORM_NAME                  = SRA3-40
  PLATFORM_GUID                  = D98574D9-110C-4804-BA4E-0DE396A17A0E
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001c
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = RISCV64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Sophgo/SG2044Pkg/SRA3-40/SRA3-40.fdf

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
  # Network definition
  #
  DEFINE NETWORK_SNP_ENABLE       = FALSE
  DEFINE NETWORK_IP6_ENABLE       = FALSE
  DEFINE NETWORK_TLS_ENABLE       = FALSE
  DEFINE NETWORK_HTTP_BOOT_ENABLE = FALSE
  DEFINE NETWORK_ISCSI_ENABLE     = FALSE

  DEFINE FLASH_ENABLE             = TRUE
  DEFINE ETH_ENABLE               = FALSE
  DEFINE ACPI_ENABLE              = FALSE

  #
  # BMC
  #
  DEFINE BMC_ENABLE               = TRUE


  #
  # x64 Emulator
  #
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
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
  VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  IniParserLib|Silicon/Sophgo/Library/IniParserLib/IniParserLib.inf
  ConfigUtilsLib|Silicon/Sophgo/Library/ConfigUtilsLib/ConfigUtilsLib.inf
  SmbiosInformationLib|Silicon/Sophgo/SG2044Pkg/Library/SmbiosInformation/SmbiosInformationLib.inf
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

  # ipmi ssif smbus lib
  PlatformBmcReadyLib|Features/ManageabilityPkg/Library/PlatformBmcReadyLibNull/PlatformBmcReadyLibNull.inf
  ManageabilityTransportHelperLib|Features/ManageabilityPkg/Library/BaseManageabilityTransportHelperLib/BaseManageabilityTransportHelper.inf
  SmbusLib|MdePkg/Library/DxeSmbusLib/DxeSmbusLib.inf
  IpmiLib|MdeModulePkg/Library/DxeIpmiLibIpmiProtocol/DxeIpmiLibIpmiProtocol.inf
  IpmiCommandLib|Features/ManageabilityPkg/Library/IpmiCommandLib/IpmiCommandLib.inf
  # ManageabilityTransportLib|Features/ManageabilityPkg/Library/ManageabilityTransportSsifLib/Dxe/DxeManageabilityTransportSsif.inf
  ManageabilityTransportLib|edk2-platforms/Silicon/Sophgo/Library/SophgoManageabilityTransportSerialLib/Dxe/DxeManageabilityTransportSerial.inf
  SophgoNs16550Lib|edk2-platforms/Silicon/Sophgo/Library/SophgoNs16550Lib/SophgoNs16550.inf

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
  RiscVFpuLib|UefiCpuPkg/Library/BaseRiscVFpuLib/BaseRiscVFpuLib.inf
  RiscVMmuLib|UefiCpuPkg/Library/BaseRiscVMmuLib/BaseRiscVMmuLib.inf
  RiscVCpuLib|Silicon/RISC-V/ProcessorPkg/Library/RiscVCpuLib/RiscVCpuLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/BaseRiscV64CpuExceptionHandlerLib/BaseRiscV64CpuExceptionHandlerLib.inf

  TimerLib|UefiCpuPkg/Library/BaseRiscV64CpuTimerLib/BaseRiscV64CpuTimerLib.inf
  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
  RealTimeClockLib|EmbeddedPkg//Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf

  # Flattened Device Tree (FDT) access library
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
  DtPlatformDtbLoaderLib|EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf

  # PCIe dependencies
  PciHostBridgeLib|Silicon/Sophgo/SG2044Pkg/Library/PciHostBridgeLib/PciHostBridgeLib.inf
  PciSegmentLib|Silicon/Sophgo/SG2044Pkg/Library/PciSegmentLib/PciSegmentLib.inf
  PciPlatformLib|Silicon/Sophgo/SG2044Pkg/Library/PciPlatformLib/PciPlatformLib.inf

  # Nor Flash Library
  NorFlashInfoLib|EmbeddedPkg/Library/NorFlashInfoLib/NorFlashInfoLib.inf

  # Hash Password
  HashPasswordLib|Silicon/Sophgo/Library/HashPasswordLib/HashPasswordLib.inf

  # Ds1307 RTC Library
  # RealTimeClockLib|Silicon/Sophgo/Library/Ds1307RealTimeClockLib/Ds1307RealTimeClockLib.inf

  IniParserLib|Silicon/Sophgo/Library/IniParserLib/IniParserLib.inf

  EfuseLib|Silicon/Sophgo/Library/EfuseLib/EfuseLib.inf

  #
  # Random Generator Library
  #
  TrngLib|Silicon/Sophgo/Library/TrngLib/TrngLib.inf
  RngLib|Silicon/Sophgo/Library/RngLib/RngLib.inf

  ResetSystemLib|OvmfPkg/RiscVVirt/Library/ResetSystemLib/BaseResetSystemLib.inf
  DmaLib|EmbeddedPkg/Library/NonCoherentDmaLib/NonCoherentDmaLib.inf

[LibraryClasses.common.SEC]
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
  PlatformSecLib|UefiCpuPkg/Library/PlatformSecLibNull/PlatformSecLibNull.inf
  PlatformPeiLib|Silicon/Sophgo/Modules/PlatformPei/PlatformPeiLib.inf

!ifdef $(SOURCE_DEBUG_ENABLE)
  DebugAgentLib|SourceLevelDebugPkg/Library/DebugAgent/SecPeiDebugAgentLib.inf
!endif

[LibraryClasses.common.PEI_CORE]
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
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutUgaSupport|FALSE
  #
  # Activate AcpiSdtProtocol
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE

[PcdsFeatureFlag.common]
  ## Indicates if S3 performance data will be supported in ACPI FPDT table.
  #   TRUE  - S3 performance data will be supported in ACPI FPDT table.
  #   FALSE - S3 performance data will not be supported in ACPI FPDT table.
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwarePerformanceDataTableS3Support|FALSE

[PcdsFixedAtBuild]
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
  # SiFive’s Sv48 implementation provides a 48-bit virtual address space
  # using 47-bits of physical address space.
  # The max physical address is 0xFFFFFFFFFF in the SoC System map.
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
  gSophgoTokenSpaceGuid.PcdIniFileRamAddress|0x89000000
  gSophgoTokenSpaceGuid.PcdIniFileMaxSize|8192
  gSophgoTokenSpaceGuid.PcdMisa|0x00B4112F
  gSophgoTokenSpaceGuid.PcdMCUI2cBus|1
  gSophgoTokenSpaceGuid.PcdRtcI2cBusNum0|2
  gSophgoTokenSpaceGuid.PcdRtcI2cBusNum1|3
  gSophgoTokenSpaceGuid.PcdSsifI2cBusNum|3

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
  gSophgoTokenSpaceGuid.PcdDwMac4DefaultMacAddress|0x12345678ABCD
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

# McuExistence evb
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdMcuExistence|TRUE


#pcie evb/server_4_slot
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.NumOfControllers|5

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieSupportFlag[0]|{TRUE,TRUE,TRUE,TRUE,TRUE}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieSupportFlag[1]|{TRUE,TRUE,TRUE,TRUE,TRUE}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieSupportFlag[2]|{TRUE,TRUE,TRUE,TRUE,TRUE}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieSupportFlag[3]|{TRUE,TRUE,TRUE,TRUE,TRUE}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieSupportFlag[4]|{TRUE,TRUE,TRUE,TRUE,TRUE}

  # UINT32
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieDomain[0]|{0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieDomain[1]|{0x02,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieDomain[2]|{0x04,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieDomain[3]|{0x06,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieDomain[4]|{0x08,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.RootBusConfig[0]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.RootBusConfig[1]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.RootBusConfig[2]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.RootBusConfig[3]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.RootBusConfig[4]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieReg[0]|{0x00,0x00,0x40,0x00,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x78,0x00,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x70,0x00,0x6c,0x00,0x00,0x00, 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieReg[1]|{0x00,0x00,0x00,0x00,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x0c,0x00,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x30,0x00,0x6c,0x00,0x00,0x00, 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x50,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieReg[2]|{0x00,0x00,0x40,0x04,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x78,0x04,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x70,0x04,0x6c,0x00,0x00,0x00, 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieReg[3]|{0x00,0x00,0x00,0x04,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x0c,0x04,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x30,0x04,0x6c,0x00,0x00,0x00, 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieReg[4]|{0x00,0x00,0xc0,0x04,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x78,0x08,0x6c,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x70,0x08,0x6c,0x00,0x00,0x00, 0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00, 0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00}

  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieRanges[0]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x48,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x48,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x40,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieRanges[1]|{0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x58,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x58,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x50,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieRanges[2]|{0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x24,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x24,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7A,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7A,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x79,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x79,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x78,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieRanges[3]|{0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7D,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7D,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x7C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieRanges[4]|{0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x60,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[0]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[1]|{0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[2]|{0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[3]|{0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[4]|{0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[0]|{0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[1]|{0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[2]|{0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[3]|{0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[4]|{0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x48,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}

  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[0]|{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[1]|{0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[2]|{0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[3]|{0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem32Ranges[4]|{0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}

  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[0]|{0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[1]|{0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[2]|{0x00,0x00,0x00,0x24,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x24,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[3]|{0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00}
  # gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem32Ranges[4]|{0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem64Ranges[0]|{0x00,0x00,0x00,0x00,0x48,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x48,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem64Ranges[1]|{0x00,0x00,0x00,0x00,0x58,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x58,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem64Ranges[2]|{0x00,0x00,0x00,0x00,0x7A,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7A,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem64Ranges[3]|{0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PciePmem64Ranges[4]|{0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem64Ranges[0]|{0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem64Ranges[1]|{0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x54,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem64Ranges[2]|{0x00,0x00,0x00,0x00,0x79,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x79,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem64Ranges[3]|{0x00,0x00,0x00,0x00,0x7D,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x7D,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieMem64Ranges[4]|{0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x62,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieIoRanges[0]|{0x00,0x00,0x00,0x10,0x40,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieIoRanges[1]|{0x00,0x00,0x00,0x10,0x50,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieIoRanges[2]|{0x00,0x00,0x00,0x10,0x78,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieIoRanges[3]|{0x00,0x00,0x00,0x10,0x7C,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.PcieIoRanges[4]|{0x00,0x00,0x00,0x10,0x60,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceStartAddr[0]|{0x00,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceEndAddr[0]|{0xFF,0xFF,0xFF,0x0F}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceStartAddr[0]|{0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceEndAddr[0]|{0xFF,0xFF,0xFF,0xFF,0x4F,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceStartAddr[1]|{0x00,0x00,0x00,0x10}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceEndAddr[1]|{0xFF,0xFF,0xFF,0x1F}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceStartAddr[1]|{0x00,0x00,0x00,0x00,0x50,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceEndAddr[1]|{0xFF,0xFF,0xFF,0xFF,0x5F,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceStartAddr[2]|{0x00,0x00,0x00,0x20}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceEndAddr[2]|{0xFF,0xFF,0xFF,0x2F}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceStartAddr[2]|{0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceEndAddr[2]|{0xFF,0xFF,0xFF,0xFF,0x7B,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceStartAddr[3]|{0x00,0x00,0x00,0x30}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceEndAddr[3]|{0xFF,0xFF,0xFF,0x3F}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceStartAddr[3]|{0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceEndAddr[3]|{0xFF,0xFF,0xFF,0xFF,0x7F,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceStartAddr[4]|{0x00,0x00,0x00,0x40}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie32BitSpaceEndAddr[4]|{0xFF,0xFF,0xFF,0x4F}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceStartAddr[4]|{0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00}
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdPcieHostBridgeTable.Pcie64BitSpaceEndAddr[4]|{0xFF,0xFF,0xFF,0xFF,0x67,0x00,0x00,0x00}

  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdIsServerPlatform|TRUE
# Smbios Acpi
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
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdBoardName|L"SRA3"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdBoardVersion|L"1.0"
  # [product]
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductName|L"SRA3-40"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductVersion|L"1.0"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductManufacturer|L"SOPHGO"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductSN|L"CYKJR32BDJFJF0001"
  gSophgoSG2044PlatformPkgTokenSpaceGuid.PcdProductUUID|L"2FD7AA46-0C9C-4F92-9462-B8B7B9BEECA2"

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsDynamicDefault]
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE

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

[PcdsDynamicHii]
!if $(ACPI_ENABLE) == TRUE
  gUefiOvmfPkgTokenSpaceGuid.PcdForceNoAcpi|L"ForceNoAcpi"|gOvmfVariableGuid|0x0|FALSE|NV,BS
!else
  gUefiOvmfPkgTokenSpaceGuid.PcdForceNoAcpi|L"ForceNoAcpi"|gOvmfVariableGuid|0x0|TRUE|NV,BS
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
  Silicon/Sophgo/Core/Sec/PeilessSec.inf  {
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

  ArmVirtPkg/CloudHvPlatformHasAcpiDtDxe/CloudHvHasAcpiDtDxe.inf
  EmbeddedPkg/Drivers/FdtClientDxe/FdtClientDxe.inf {
    <LibraryClasses>
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  }

  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

  UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf {
    <LibraryClasses>
      ResetSystemLib|MdeModulePkg/Library/BaseResetSystemLibNull/BaseResetSystemLibNull.inf
  }
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf {
    <LibraryClasses>
      RealTimeClockLib|EmbeddedPkg//Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
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
  Silicon/RISC-V/ProcessorPkg/Universal/FdtDxe/FdtDxe.inf
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
  Platform/Sophgo/SG2044Pkg/Drivers/SmbiosPlatformDxe/SmbiosPlatformDxe.inf

  #
  # PCIe Support
  #
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf {
    <LibraryClasses>
      NULL|Silicon/Sophgo/SG2044Pkg/Library/PciPlatformLib/PciPlatformLib.inf
  }

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

  #
  # iPXE Application
  #
  Silicon/Sophgo/SG2044/iPXE/iPXE.inf
  MdeModulePkg/Universal/LoadFileOnFv2/LoadFileOnFv2.inf

  #
  # ipmi ssif smbus driver
  #
  Silicon/Sophgo/Drivers/SmbusHcDxe/SmbusHcDxe.inf
  Features/ManageabilityPkg/Universal/IpmiProtocol/Dxe/IpmiProtocolDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/BmcConfigDxe/BmcConfig.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/IpmiBootDxe/IpmiBootDxe.inf

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

  #
  # ACPI Support
  #
!if $(ACPI_ENABLE) == TRUE
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  Silicon/Sophgo/SG2044Pkg/Drivers/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  Silicon/Sophgo/SG2044Pkg/AcpiTables/SG2044AcpiTables.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableDxe/FirmwarePerformanceDxe.inf {
    <LibraryClasses>
      LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  }
!endif
