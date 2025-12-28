/** @file PciHostBridgeLibConstructor.c

  Copyright (c) 2023, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FdtClient.h>
#include <Include/SophgoPciRegs.h>
#include <Include/PlatformPciLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/UefiBootServicesTableLib.h>

#define PLAT_CPU_TO_BUS_ADDR  0xCFFFFFFFFF

#define UPPER_32_BITS(n)      ((UINT32)((n) >> 32))
#define LOWER_32_BITS(n)      ((UINT32)((n) & 0xffffffff))

STATIC
VOID
PcieHostInitRootPort (
  IN UINT32                   VendorId,
  IN UINT32                   DeviceId,
  IN UINT64                   CfgBase
  )
{
  INT32    Ctrl;
  UINT32   Id;
  UINT32   Value;

  //
  // Set the root complex BAR configuration register:
  // - disable both BAR0 and BAR1.
  // - enable Prefetchable Memory Base and Limit registers in type 1
  //   config space (64 bits).
  // - enable IO Base and Limit registers in type 1 config
  //   space (32 bits).
  //
  Ctrl = PCIE_LM_BAR_CFG_CTRL_DISABLED;
  Value = PCIE_LM_RC_BAR_CFG_BAR0_CTRL(Ctrl) |
    PCIE_LM_RC_BAR_CFG_BAR1_CTRL(Ctrl) |
    PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_ENABLE |
    PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_64BITS |
    PCIE_LM_RC_BAR_CFG_IO_ENABLE |
    PCIE_LM_RC_BAR_CFG_IO_32BITS;
  MmioWrite32 ((UINTN)(CfgBase + PCIE_LM_RC_BAR_CFG), Value);

  //
  // Set root port configuration space
  //
  if (VendorId != 0xffff) {
    Id = PCIE_LM_ID_VENDOR(VendorId) | PCIE_LM_ID_SUBSYS(VendorId);
    MmioWrite32((UINTN)(CfgBase + PCIE_LM_ID), Id);
  }

  if (DeviceId != 0xffff) {
    Value = MmioRead32 ((UINTN)(CfgBase + PCIE_RP_BASE + PCI_VENDOR_ID_OFFSET));
    Value &= 0x0000FFFF;
    Value |= (DeviceId << 16);
    MmioWrite32 ((UINTN)(CfgBase + PCIE_RP_BASE + PCI_VENDOR_ID_OFFSET), Value);
  }
  MmioWrite32 ((UINTN)(CfgBase + PCIE_RP_BASE + PCI_REVISION_ID_OFFSET), PCI_CLASS_BRIDGE_PCI << 16);
}

STATIC
VOID
PcieHostNoBarMatchInboundConfig (
  IN UINT32                   NoBarNbits,
  IN UINT64                   CfgBase
  )
{
  UINT32        Addr0;
  UINT32        Addr1;

  //
  // Set Root Port no BAR match Inbound Translation registers:
  // needed for MSI and DMA.
  // Root Port BAR0 and BAR1 are disabled, hence no need to set
  // their inbound translation registers.
  //
  Addr0 = PCIE_AT_IB_RP_BAR_ADDR0_NBITS (NoBarNbits);
  Addr1 = 0;
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_IB_RP_BAR_ADDR0(RP_NO_BAR)), Addr0);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_IB_RP_BAR_ADDR1(RP_NO_BAR)), Addr1);
}

STATIC
VOID
PcieHostSetOutboundRegionForConfigureSpaceAccess (
  IN UINT64                   CfgBase,
  IN UINT64                   SlvAddr,
  IN INT32                    BusNumber
  )
{
  UINT32        Addr0;
  UINT32        Addr1;
  UINT32        Desc1;

  //
  // Set the PCI Address for Region 0
  // Reserve region 0 for PCI configure space accesses:
  // OB_REGION_PCI_ADDR0 and OB_REGION_DESC0 are updated dynamically by
  // PciMapBus (), other region registers are set here once for all.
  //
  Addr1 = 0; // Should be programmed to zero.
  Desc1 = PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_PCI_ADDR1(0)), Addr1);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_DESC1(0)), Desc1);

  //
  // Set the AXI Address for Region 0
  //
  Addr0 = PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(12) |
          (LOWER_32_BITS(SlvAddr) & GENMASK(31, 8));
  Addr1 = UPPER_32_BITS(SlvAddr);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_CPU_ADDR0(0)), Addr0);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_CPU_ADDR1(0)), Addr1);
}

STATIC
VOID
PcieHostSetOutboundRegion (
  IN UINT64                   CfgBase,
  IN INT32                    BusNumber,
  IN INT32                    RegionNumber,
  IN BOOLEAN                  IsMemory,
  IN UINT64                   PciAddr,
  IN UINT64                   CpuAddr,
  IN UINT64                   RegionSize
  )
{
  UINT32        Addr0;
  UINT32        Addr1;
  UINT32        Desc0;
  UINT32        Desc1;
  UINT32        Nbits;

  //
  // Get shift amount
  //
  Nbits         = 0;
  RegionSize   -= 1;
  while (RegionSize >= 1) {
    RegionSize >>= 1;
    Nbits++;
  }

  if (Nbits < 8) {
    Nbits = 8;
  }

  //
  // Set the PCI address, RegionNumber >= 1
  //
  Addr0 = PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(Nbits) |
          (LOWER_32_BITS(PciAddr) & GENMASK(31, 8));
  Addr1 = UPPER_32_BITS(PciAddr);

  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_PCI_ADDR0(RegionNumber)), Addr0);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_PCI_ADDR1(RegionNumber)), Addr1);

  //
  // Set the PCIe header descriptor
  //
  if (IsMemory) {
    Desc0 = PCIE_AT_OB_REGION_DESC0_TYPE_MEM;
  } else {
    Desc0 = PCIE_AT_OB_REGION_DESC0_TYPE_IO;
  }
  Desc1 = 0;

  //
  // In Root Complex mode, the function number is always 0.
  //
  Desc0 |= PCIE_AT_OB_REGION_DESC0_HARDCODED_RID |
           PCIE_AT_OB_REGION_DESC0_DEVFN(0);
  Desc1 |= PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_DESC0(RegionNumber)), Desc0);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_DESC1(RegionNumber)), Desc1);

  //
  // Set the CPU address
  //
  Addr0 = PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(Nbits) |
          (LOWER_32_BITS(CpuAddr) & GENMASK(31, 8));
  Addr1 = UPPER_32_BITS(CpuAddr);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_CPU_ADDR0(RegionNumber)), Addr0);
  MmioWrite32 ((UINTN)(CfgBase + PCIE_AT_OB_REGION_CPU_ADDR1(RegionNumber)), Addr1);
}

STATIC
EFI_STATUS
GetPcieEnableMask (
  VOID
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  EFI_STATUS           Status;
  EFI_STATUS           FindNodeStatus;
  INT32                Node;
  CONST VOID           *Prop;
  UINT32               PropSize;
  UINT16               PortIndex;
  UINT8                PcieEnableMask;

  PcieEnableMask = PcdGet8 (PcdMangoPcieEnableMask);

  Status = gBS->LocateProtocol (
      &gFdtClientProtocolGuid,
      NULL,
      (VOID **) &FdtClient
      );
  ASSERT_EFI_ERROR (Status);

  for (FindNodeStatus = FdtClient->FindCompatibleNode (
                                     FdtClient,
                                     "sophgo,sg2042-pcie-host",
                                     &Node
                                     );

    !EFI_ERROR (FindNodeStatus);

    FindNodeStatus = FdtClient->FindNextCompatibleNode (
                                     FdtClient,
                                     "sophgo,sg2042-pcie-host",
                                     Node,
                                     &Node
                                     ))
  {
    Status = FdtClient->GetNodeProperty (
                      FdtClient,
                      Node,
                      "status",
                      &Prop,
                      &PropSize
                      );

    if (!EFI_ERROR (Status)) {
        if (AsciiStrCmp ((CHAR8 *)Prop, "okay") != 0) {
            DEBUG ((DEBUG_INFO, "%a: Device is disabled, skipping.\n", __func__));
            break;
        }
    } else {
      DEBUG ((DEBUG_ERROR, "%a: Device is disabled, skipping.\n", __func__));
      return Status;
    }

    Status = FdtClient->GetNodeProperty (
                                FdtClient,
                                Node,
                                "linux,pci-domain",
                                &Prop,
                                &PropSize
                                );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: Get linux,pci-domain failed (Status = %r)\n",
        __func__,
        Status
        ));
      return Status;
    }

    PortIndex = SwapBytes32 (((CONST UINT32 *) Prop)[0]);

    PcieEnableMask |= 1 << PortIndex;
  }

  PcdSet8S (PcdMangoPcieEnableMask, PcieEnableMask);

  DEBUG ((
    DEBUG_ERROR,
    "%a(): PcdMangoPcieEnableMask = 0x%x\n",
    __func__,
    PcieEnableMask
    ));

  return EFI_SUCCESS;
}

STATIC
VOID
ShowPciResource(MANGO_PCI_RESOURCE *Res)
{
  DEBUG (( DEBUG_INFO, "-------------------------------\n" ));
  DEBUG (( DEBUG_INFO, "Controller Address: 0x%010lx\n", Res->ConfigSpaceAddress ));
  DEBUG (( DEBUG_INFO, "Slave Address: 0x%010lx\n", Res->PciSlvAddress ));
  DEBUG (( DEBUG_INFO, "Segment: %d\n", Res->Segment ));
  DEBUG (( DEBUG_INFO, "Bus Number: %d - %d\n", Res->BusBase, Res->BusBase + Res->BusSize - 1 ));
  DEBUG (( DEBUG_INFO, "IO [0x%010lx : 0x%010lx : 0x%010lx]\n", Res->IoBase, Res->IoSize, Res->IoTranslation ));
  DEBUG (( DEBUG_INFO, "Mem32 [0x%010lx : 0x%010lx : 0x%010lx]\n", Res->Mmio32Base, Res->Mmio32Size, Res->Mmio32Translation ));
  DEBUG (( DEBUG_INFO, "Mem64 [0x%010lx : 0x%010lx : 0x%010lx]\n", Res->Mmio64Base, Res->Mmio64Size, Res->Mmio64Translation ));
  DEBUG (( DEBUG_INFO, "-------------------------------\n" ));

}

EFI_STATUS
EFIAPI
MangoPcieHostBridgeLibConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  UINT32  SocketIndex;
  UINT32  PortIndex;
  UINT32  LinkIndex;
  UINT32  VendorId;
  UINT32  DeviceId;
  UINT32  NoBarNbits;
  UINT8   PcieEnableMask;

  VendorId = 0x17CD;
  DeviceId = 0x2042;

  //
  // the number least significant bits kept during
  // inbound (PCIe -> AXI) address translations
  //
  NoBarNbits = 0x30;

  // workaround: ensure the PCIe switch is powered on and fully initialized before enumeration
  // 8s delay to allow the switch to become ready, at least 6s.
  gBS->Stall (8000000);

  //
  // Get the PCIe RC count
  //
  GetPcieEnableMask ();

  PcieEnableMask = PcdGet8 (PcdMangoPcieEnableMask);

  DEBUG ((DEBUG_INFO, "Mango PCIe HostBridgeLib constructor:\n"));
  for (SocketIndex = 0; SocketIndex < PCIE_MAX_SOCKET; SocketIndex++) {
    for (PortIndex = 0; PortIndex < PCIE_MAX_PORT; PortIndex++) {
      for (LinkIndex = 0; LinkIndex < PCIE_MAX_LINK; LinkIndex++) {
        if (!((PcieEnableMask >>
	       ((PCIE_MAX_PORT * PCIE_MAX_LINK * SocketIndex) + (PCIE_MAX_LINK * PortIndex) + LinkIndex)) & 0x01)) {
          continue;
        }

        ShowPciResource (&mPciResource[SocketIndex][PortIndex][LinkIndex]);

        PcieHostInitRootPort (
                              VendorId,
                              DeviceId,
                              mPciResource[SocketIndex][PortIndex][LinkIndex].ConfigSpaceAddress
                             );

        //
        // Inbound: no bar match
        //
        PcieHostNoBarMatchInboundConfig (
                                         NoBarNbits,
                                         mPciResource[SocketIndex][PortIndex][LinkIndex].ConfigSpaceAddress
                                        );

        //
        // Outbound: Region 0 for Slave address (configure space access)
        //
        PcieHostSetOutboundRegionForConfigureSpaceAccess (
                                                          mPciResource[SocketIndex][PortIndex][LinkIndex].ConfigSpaceAddress,
                                                          mPciResource[SocketIndex][PortIndex][LinkIndex].PciSlvAddress,
                                                          mPciResource[SocketIndex][PortIndex][LinkIndex].BusBase
                                                         );

        //
        // Outbound: IO
        // TBD: Workaround for SG2042 to map the IO below 4G to Above 4G.
        //
        PcieHostSetOutboundRegion (
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].ConfigSpaceAddress,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].BusBase,
                                   1,
                                   FALSE,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].IoBase,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].IoBase - mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio32Translation,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].IoSize
                                  );

        //
        // Outbound: Mem32
        //
        PcieHostSetOutboundRegion (
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].ConfigSpaceAddress,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].BusBase,
                                   2,
                                   TRUE,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio32Base,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio32Base - mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio32Translation,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio32Size
                                  );

        //
        // Outbound: MemAbove4G
        //
        PcieHostSetOutboundRegion (
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].ConfigSpaceAddress,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].BusBase,
                                   3,
                                   TRUE,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio64Base,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio64Base - mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio64Translation,
                                   mPciResource[SocketIndex][PortIndex][LinkIndex].Mmio64Size
                                  );

        DEBUG ((
                DEBUG_INFO,
                "PCIe Socket %d, Port %d, Link %d initialization success.\n",
                SocketIndex,
                PortIndex,
                LinkIndex
               ));
      }
    }
  }

  return EFI_SUCCESS;
}
