/** @file
  DW Mac4 SNP driver.

  Copyright (c) 2024, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

  The original software modules are licensed as follows:

  Copyright (c) 2012 - 2020, Arm Limited. All rights reserved.<BR>
  Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Protocol/Cpu.h>
#include <Library/IoLib.h>
#include <Library/NetLib.h>
#include <Library/DmaLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IniParserLib.h>
#include <Protocol/FdtClient.h>

#include "DwMac4SnpDxe.h"
#include "DwMac4DxeUtil.h"

STATIC
SOPHGO_SIMPLE_NETWORK_DEVICE_PATH PathTemplate = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_MAC_ADDR_DP,
      {
        (UINT8)(sizeof (MAC_ADDR_DEVICE_PATH)),
        (UINT8)((sizeof (MAC_ADDR_DEVICE_PATH)) >> 8)
      }
    },
    {
      {
        0
      }
    },
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

/**
  Change the state of a network interface from "stopped" to "started."

  This function starts a network interface. If the network interface successfully
  starts, then EFI_SUCCESS will be returned.

  @param  Snp                    A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @retval EFI_SUCCESS            The network interface was started.
  @retval EFI_ALREADY_STARTED    The network interface is already in the started state.
  @retval EFI_INVALID_PARAMETER  This parameter was NULL or did not point to a valid
                                 EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR       The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED        This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
SnpStart (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This
 )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER    *DwMac4Driver;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStarted) {
    return EFI_ALREADY_STARTED;
  } else if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkStopped) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Change state
  //
  DwMac4Driver->SnpMode.State = EfiSimpleNetworkStarted;

  return EFI_SUCCESS;
}

/**
  Changes the state of a network interface from "started" to "stopped."

  This function stops a network interface. This call is only valid if the network
  interface is in the started state. If the network interface was successfully
  stopped, then EFI_SUCCESS will be returned.

  @param  Snp                     A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL
                                  instance.


  @retval EFI_SUCCESS             The network interface was stopped.
  @retval EFI_NOT_STARTED         The network interface has not been started.
  @retval EFI_INVALID_PARAMETER   This parameter was NULL or did not point to a
                                  valid EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR        The command could not be sent to the network
                                  interface.
  @retval EFI_UNSUPPORTED         This function is not supported by the network
                                  interface.

**/
EFI_STATUS
EFIAPI
SnpStop (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL*   This
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER    *DwMac4Driver;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  } else if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkStarted) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Stop all RX and TX DMA channels
  //
  StmmacStopAllDma (DwMac4Driver);

  //
  // Change the state
  //
  DwMac4Driver->SnpMode.State = EfiSimpleNetworkStopped;

  return EFI_SUCCESS;
}

/**
  Resets a network adapter and allocates the transmit and receive buffers
  required by the network interface; optionally, also requests allocation of
  additional transmit and receive buffers.

  This function allocates the transmit and receive buffers required by the network
  interface. If this allocation fails, then EFI_OUT_OF_RESOURCES is returned.
  If the allocation succeeds and the network interface is successfully initialized,
  then EFI_SUCCESS will be returned.

  @param Snp                A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @param ExtraRxBufferSize  The size, in bytes, of the extra receive buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.
  @param ExtraTxBufferSize  The size, in bytes, of the extra transmit buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the
                            extra buffer, and the caller will not know if it is
                            actually being used.

  @retval EFI_SUCCESS           The network interface was initialized.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_OUT_OF_RESOURCES  There was not enough memory for the transmit and
                                receive buffers.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       The increased buffer size feature is not supported.

**/
EFI_STATUS
EFIAPI
SnpInitialize (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN  UINTN                         ExtraRxBufferSize OPTIONAL,
  IN  UINTN                         ExtraTxBufferSize OPTIONAL
  )
{
  EFI_STATUS                         Status;
  SOPHGO_SIMPLE_NETWORK_DRIVER       *DwMac4Driver;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  } else if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkStarted) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Init PHY
  //
  Status = gBS->LocateProtocol (
                  &gSophgoPhyProtocolGuid,
                  NULL,
                  (VOID **) &DwMac4Driver->Phy
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Locate SOPHGO_PHY_PROTOCOL failed (Status=%r)\n",
      __func__,
      Status
      ));
    return Status;
  }
#if 1
  Status = DwMac4Driver->Phy->Init (DwMac4Driver->Phy,
                 PHY_INTERFACE_MODE_RGMII_ID,
                  &DwMac4Driver->PhyDev
                  );
  if (EFI_ERROR (Status) && Status != EFI_TIMEOUT) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): PHY initialization failed (Status=%r)\n",
      __func__,
      Status
      ));
    return Status;
  }

  gBS->Stall (1000000);
#else
  StmmacMacLinkUp (PHY_INTERFACE_MODE_RGMII_ID, DUPLEX_FULL, DwMac4Driver);
#endif
  //
  // DMA initialization and SW reset
  //
  Status = StmmacInitDmaEngine (DwMac4Driver);
  if (EFI_ERROR (Status)) {
    DEBUG (( DEBUG_ERROR, "%a(): DMA initialization failed (Status=%r)\n",  __func__, Status));
    return Status;
  }

  //
  // Configure flow control
  //
  StmmacMacFlowControl (DwMac4Driver, DwMac4Driver->PhyDev->Duplex, FLOW_AUTO);
  StmmacMtlConfiguration (DwMac4Driver);

  //
  // Copy the MAC addr into the HW
  //
  StmmacSetUmacAddr (&DwMac4Driver->SnpMode.CurrentAddress, DwMac4Driver, 0);
  StmmacGetMacAddr (&DwMac4Driver->SnpMode.CurrentAddress, DwMac4Driver, 0);

  //
  // Declare the driver as initialized
  //
  DwMac4Driver->SnpMode.State = EfiSimpleNetworkInitialized;

  return EFI_SUCCESS;
}

/**
  Resets a network adapter and reinitializes it with the parameters that were
  provided in the previous call to Initialize().

  This function resets a network adapter and reinitializes it with the parameters
  that were provided in the previous call to Initialize(). The transmit and
  receive queues are emptied and all pending interrupts are cleared.
  Receive filters, the station address, the statistics, and the multicast-IP-to-HW
  MAC addresses are not reset by this call. If the network interface was
  successfully reset, then EFI_SUCCESS will be returned. If the driver has not
  been initialized, EFI_DEVICE_ERROR will be returned.

  @param Snp                  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param ExtendedVerification Indicates that the driver may perform a more
                              exhaustive verification operation of the device
                              during reset.

  @retval EFI_SUCCESS           The network interface was reset.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
SnpReset (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN  BOOLEAN                       ExtendedVerification
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER       *DwMac4Driver;

  DEBUG ((
    DEBUG_INFO,
    "%a()\r\n",
    __func__
    ));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Resets a network adapter and leaves it in a state that is safe for another
  driver to initialize.

  This function releases the memory buffers assigned in the Initialize() call.
  Pending transmits and receives are lost, and interrupts are cleared and disabled.
  After this call, only the Initialize() and Stop() calls may be used. If the
  network interface was successfully shutdown, then EFI_SUCCESS will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param  This  A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.

  @retval EFI_SUCCESS           The network interface was shutdown.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.

**/
EFI_STATUS
EFIAPI
SnpShutdown (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL*   This
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER     *DwMac4Driver;

  DEBUG ((
    DEBUG_INFO,
    "%a ()\r\n",
    __func__
    ));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Stop all RX and TX DMA channels
  //
  StmmacStopAllDma (DwMac4Driver);

  DwMac4Driver->SnpMode.State = EfiSimpleNetworkStarted;

  return EFI_SUCCESS;
}

/**
  Manages the multicast receive filters of a network interface.

  This function is used enable and disable the hardware and software receive
  filters for the underlying network device.
  The receive filter change is broken down into three steps:
  * The filter mask bits that are set (ON) in the Enable parameter are added to
    the current receive filter settings.
  * The filter mask bits that are set (ON) in the Disable parameter are subtracted
    from the updated receive filter settings.
  * If the resulting receive filter setting is not supported by the hardware a
    more liberal setting is selected.
  If the same bits are set in the Enable and Disable parameters, then the bits
  in the Disable parameter takes precedence.
  If the ResetMCastFilter parameter is TRUE, then the multicast address list
  filter is disabled (irregardless of what other multicast bits are set in the
  Enable and Disable parameters). The SNP->Mode->MCastFilterCount field is set
  to zero. The Snp->Mode->MCastFilter contents are undefined.
  After enabling or disabling receive filter settings, software should verify
  the new settings by checking the Snp->Mode->ReceiveFilterSettings,
  Snp->Mode->MCastFilterCount and Snp->Mode->MCastFilter fields.
  Note: Some network drivers and/or devices will automatically promote receive
    filter settings if the requested setting can not be honored. For example, if
    a request for four multicast addresses is made and the underlying hardware
    only supports two multicast addresses the driver might set the promiscuous
    or promiscuous multicast receive filters instead. The receiving software is
    responsible for discarding any extra packets that get through the hardware
    receive filters.
    Note: Note: To disable all receive filter hardware, the network driver must
      be Shutdown() and Stopped(). Calling ReceiveFilters() with Disable set to
      Snp->Mode->ReceiveFilterSettings will make it so no more packets are
      returned by the Receive() function, but the receive hardware may still be
      moving packets into system memory before inspecting and discarding them.
      Unexpected system errors, reboots and hangs can occur if an OS is loaded
      and the network devices are not Shutdown() and Stopped().
  If ResetMCastFilter is TRUE, then the multicast receive filter list on the
  network interface will be reset to the default multicast receive filter list.
  If ResetMCastFilter is FALSE, and this network interface allows the multicast
  receive filter list to be modified, then the MCastFilterCnt and MCastFilter
  are used to update the current multicast receive filter list. The modified
  receive filter list settings can be found in the MCastFilter field of
  EFI_SIMPLE_NETWORK_MODE. If the network interface does not allow the multicast
  receive filter list to be modified, then EFI_INVALID_PARAMETER will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.
  If the receive filter mask and multicast receive filter list have been
  successfully updated on the network interface, EFI_SUCCESS will be returned.

  @param Snp              A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param Enable           A bit mask of receive filters to enable on the network
                          interface.
  @param Disable          A bit mask of receive filters to disable on the network
                          interface. For backward compatibility with EFI 1.1
                          platforms, the EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit
                          must be set when the ResetMCastFilter parameter is TRUE.
  @param ResetMCastFilter Set to TRUE to reset the contents of the multicast
                          receive filters on the network interface to their
                          default values.
  @param MCastFilterCnt   Number of multicast HW MAC addresses in the new MCastFilter
                          list. This value must be less than or equal to the
                          MCastFilterCnt field of EFI_SIMPLE_NETWORK_MODE.
                          This field is optional if ResetMCastFilter is TRUE.
  @param MCastFilter      A pointer to a list of new multicast receive filter HW
                          MAC addresses. This list will replace any existing
                          multicast HW MAC address list. This field is optional
                          if ResetMCastFilter is TRUE.

  @retval EFI_SUCCESS            The multicast receive filter list was updated.
  @retval EFI_NOT_STARTED        The network interface has not been started.
  @retval EFI_INVALID_PARAMETER  One or more of the following conditions is TRUE:
                                 * This is NULL
                                 * There are bits set in Enable that are not set
                                   in Snp->Mode->ReceiveFilterMask
                                 * There are bits set in Disable that are not set
                                   in Snp->Mode->ReceiveFilterMask
                                 * Multicast is being enabled (the
                                   EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST bit is
                                   set in Enable, it is not set in Disable, and
                                   ResetMCastFilter is FALSE) and MCastFilterCount
                                   is zero
                                 * Multicast is being enabled and MCastFilterCount
                                   is greater than Snp->Mode->MaxMCastFilterCount
                                 * Multicast is being enabled and MCastFilter is NULL
                                 * Multicast is being enabled and one or more of
                                   the addresses in the MCastFilter list are not
                                   valid multicast MAC addresses
  @retval EFI_DEVICE_ERROR       One or more of the following conditions is TRUE:
                                 * The network interface has been started but has
                                   not been initialized
                                 * An unexpected error was returned by the
                                   underlying network driver or device
  @retval EFI_UNSUPPORTED        This function is not supported by the network
                                 interface.

**/
EFI_STATUS
EFIAPI
SnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN  UINT32                        Enable,
  IN  UINT32                        Disable,
  IN  BOOLEAN                       ResetMCastFilter,
  IN  UINTN                         MCastFilterCnt  OPTIONAL,
  IN  EFI_MAC_ADDRESS               *MCastFilter    OPTIONAL
  )
{
  UINT32                         ReceiveFilterSetting;
  SOPHGO_SIMPLE_NETWORK_DRIVER   *DwMac4Driver;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Check that bits set in Enable/Disable are set in ReceiveFilterMask
  //
   if ((Enable  & (~DwMac4Driver->SnpMode.ReceiveFilterMask)) ||
       (Disable & (~DwMac4Driver->SnpMode.ReceiveFilterMask))) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get the filter mask bits that are set in Enable parameter or Disable Parameter
  // Same bits that are set in Enable/Disable parameters, then bits in the Disable parameter takes precedance
  //
  ReceiveFilterSetting = (DwMac4Driver->SnpMode.ReceiveFilterSetting | Enable) & (~Disable);

  StmmacSetFilters (ReceiveFilterSetting, ResetMCastFilter, MCastFilterCnt, MCastFilter, DwMac4Driver);

  return EFI_SUCCESS;
}

/**
  Modifies or resets the current station address, if supported.

  This function modifies or resets the current station address of a network
  interface, if supported. If Reset is TRUE, then the current station address is
  set to the network interface's permanent address. If Reset is FALSE, and the
  network interface allows its station address to be modified, then the current
  station address is changed to the address specified by New. If the network
  interface does not allow its station address to be modified, then
  EFI_INVALID_PARAMETER will be returned. If the station address is successfully
  updated on the network interface, EFI_SUCCESS will be returned. If the driver
  has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param Snp      A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param Reset    Flag used to reset the station address to the network interface's
                  permanent address.
  @param NewMac   New station address to be used for the network interface.


  @retval EFI_SUCCESS           The network interface's station address was updated.
  @retval EFI_NOT_STARTED       The Simple Network Protocol interface has not been
                                started by calling Start().
  @retval EFI_INVALID_PARAMETER The New station address was not accepted by the NIC.
  @retval EFI_INVALID_PARAMETER Reset is FALSE and New is NULL.
  @retval EFI_DEVICE_ERROR      The Simple Network Protocol interface has not
                                been initialized by calling Initialize().
  @retval EFI_DEVICE_ERROR      An error occurred attempting to set the new
                                station address.
  @retval EFI_UNSUPPORTED       The NIC does not support changing the network
                                interface's station address.

**/
EFI_STATUS
EFIAPI
SnpStationAddress (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN  BOOLEAN                       Reset,
  IN  EFI_MAC_ADDRESS               *NewMac
  )
{
  return EFI_UNSUPPORTED;
  SOPHGO_SIMPLE_NETWORK_DRIVER   *DwMac4Driver;

  DEBUG ((
    DEBUG_INFO,
    "%a()\r\n",
    __func__
    ));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Get the Permanent MAC address if need reset
  //
  if (Reset) {
    //
    // Try parse conf.ini first to get mac address
    //
    if (IsIniFileExist ()) {
      MacAddrIniParser ();
      NewMac = (EFI_MAC_ADDRESS *) (MacConfig.Mac0Addr);
    } else {
      DEBUG ((
        DEBUG_WARN,
        "%a() Warning: using driver-default MAC address\n",
        __func__
        ));
      NewMac = (EFI_MAC_ADDRESS *) (FixedPcdGet64 (PcdDwMac4DefaultMacAddress));
    }
  } else {
    //
    // Otherwise use the specified new MAC address
    //
    if (NewMac == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // If it is a multicast address, it is not valid.
    //
    if (NewMac->Addr[0] & 0x01) {
      return EFI_INVALID_PARAMETER;
    }
  }

  CopyMem (&DwMac4Driver->SnpMode.CurrentAddress, NewMac, NET_ETHER_ADDR_LEN);
  StmmacSetUmacAddr (&DwMac4Driver->SnpMode.CurrentAddress, DwMac4Driver, 0);

  return EFI_SUCCESS;
}

/**
  Resets or collects the statistics on a network interface.

  This function resets or collects the statistics on a network interface. If the
  size of the statistics table specified by StatisticsSize is not big enough for
  all the statistics that are collected by the network interface, then a partial
  buffer of statistics is returned in StatisticsTable, StatisticsSize is set to
  the size required to collect all the available statistics, and
  EFI_BUFFER_TOO_SMALL is returned.
  If StatisticsSize is big enough for all the statistics, then StatisticsTable
  will be filled, StatisticsSize will be set to the size of the returned
  StatisticsTable structure, and EFI_SUCCESS is returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.
  If Reset is FALSE, and both StatisticsSize and StatisticsTable are NULL, then
  no operations will be performed, and EFI_SUCCESS will be returned.
  If Reset is TRUE, then all of the supported statistics counters on this network
  interface will be reset to zero.

  @param Snp             A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param Reset           Set to TRUE to reset the statistics for the network interface.
  @param StatSize        On input the size, in bytes, of StatisticsTable. On output
                         the size, in bytes, of the resulting table of statistics.
  @param Statistics      A pointer to the EFI_NETWORK_STATISTICS structure that
                         contains the statistics. Type EFI_NETWORK_STATISTICS is
                         defined in "Related Definitions" below.

  @retval EFI_SUCCESS           The requested operation succeeded.
  @retval EFI_NOT_STARTED       The Simple Network Protocol interface has not been
                                started by calling Start().
  @retval EFI_BUFFER_TOO_SMALL  StatisticsSize is not NULL and StatisticsTable is
                                NULL. The current buffer size that is needed to
                                hold all the statistics is returned in StatisticsSize.
  @retval EFI_BUFFER_TOO_SMALL  StatisticsSize is not NULL and StatisticsTable is
                                not NULL. The current buffer size that is needed
                                to hold all the statistics is returned in
                                StatisticsSize. A partial set of statistics is
                                returned in StatisticsTable.
  @retval EFI_INVALID_PARAMETER StatisticsSize is NULL and StatisticsTable is not
                                NULL.
  @retval EFI_DEVICE_ERROR      The Simple Network Protocol interface has not
                                been initialized by calling Initialize().
  @retval EFI_DEVICE_ERROR      An error was encountered collecting statistics
                                from the NIC.
  @retval EFI_UNSUPPORTED       The NIC does not support collecting statistics
                                from the network interface.

**/
EFI_STATUS
EFIAPI
SnpStatistics (
  IN       EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN       BOOLEAN                       Reset,
  IN  OUT  UINTN                         *StatSize,
      OUT  EFI_NETWORK_STATISTICS        *Statistics
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER   *DwMac4Driver;

  DEBUG ((
    DEBUG_INFO,
    "%a()\r\n",
    __func__
    ));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Check the parameters
  //
  if ((StatSize == NULL) && (Statistics != NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Do a reset if required
  //
  if (Reset) {
    ZeroMem (&DwMac4Driver->Stats, sizeof(EFI_NETWORK_STATISTICS));
  }

  //
  // Check buffer size
  //
  if (*StatSize < sizeof(EFI_NETWORK_STATISTICS)) {
    *StatSize = sizeof(EFI_NETWORK_STATISTICS);
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Read statistic counters
  //
  StmmacGetStatistic (&DwMac4Driver->Stats, DwMac4Driver);

  //
  // Fill in the statistics
  //
  CopyMem (&Statistics, &DwMac4Driver->Stats, sizeof(EFI_NETWORK_STATISTICS));

  return EFI_SUCCESS;
}

/**
  Converts a multicast IP address to a multicast HW MAC address.

  This function converts a multicast IP address to a multicast HW MAC address
  for all packet transactions. If the mapping is accepted, then EFI_SUCCESS will
  be returned.

  @param Snp         A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param IsIpv6      Set to TRUE if the multicast IP address is IPv6 [RFC 2460].
                     Set to FALSE if the multicast IP address is IPv4 [RFC 791].
  @param Ip          The multicast IP address that is to be converted to a multicast
                     HW MAC address.
  @param McastMac    The multicast HW MAC address that is to be generated from IP.

  @retval EFI_SUCCESS           The multicast IP address was mapped to the
                                multicast HW MAC address.
  @retval EFI_NOT_STARTED       The Simple Network Protocol interface has not
                                been started by calling Start().
  @retval EFI_INVALID_PARAMETER IP is NULL.
  @retval EFI_INVALID_PARAMETER MAC is NULL.
  @retval EFI_INVALID_PARAMETER IP does not point to a valid IPv4 or IPv6
                                multicast address.
  @retval EFI_DEVICE_ERROR      The Simple Network Protocol interface has not
                                been initialized by calling Initialize().
  @retval EFI_UNSUPPORTED       IPv6 is TRUE and the implementation does not
                                support IPv6 multicast to MAC address conversion.

**/
EFI_STATUS
EFIAPI
SnpMcastIptoMac (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN   BOOLEAN                       IsIpv6,
  IN   EFI_IP_ADDRESS                *Ip,
  OUT  EFI_MAC_ADDRESS               *McastMac
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER  *DwMac4Driver;

  DEBUG ((
    DEBUG_INFO,
    "%a()\r\n",
    __func__
    ));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStarted) {
    return EFI_DEVICE_ERROR;
  } else if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  //
  // Check parameters
  //
  if ((McastMac == NULL) || (Ip == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Make sure MAC address is empty
  //
  ZeroMem (McastMac, sizeof(EFI_MAC_ADDRESS));

  //
  // If we need ipv4 address
  //
  if (!IsIpv6) {
    //
    // Most significant 25 bits of a multicast HW address are set.
    // 01-00-5E is the IPv4 Ethernet Multicast Address (see RFC 1112)
    //
    McastMac->Addr[0] = 0x01;
    McastMac->Addr[1] = 0x00;
    McastMac->Addr[2] = 0x5E;

    //
    // Lower 23 bits from ipv4 address
    //
    McastMac->Addr[3] = (Ip->v4.Addr[1] & 0x7F); // Clear the most significant bit (25th bit of MAC must be 0)
    McastMac->Addr[4] = Ip->v4.Addr[2];
    McastMac->Addr[5] = Ip->v4.Addr[3];
  } else {
    //
    // Most significant 16 bits of multicast v6 HW address are set
    // 33-33 is the IPv6 Ethernet Multicast Address (see RFC 2464)
    //
    McastMac->Addr[0] = 0x33;
    McastMac->Addr[1] = 0x33;

    //
    // lower four octets are taken from ipv6 address
    //
    McastMac->Addr[2] = Ip->v6.Addr[8];
    McastMac->Addr[3] = Ip->v6.Addr[9];
    McastMac->Addr[4] = Ip->v6.Addr[10];
    McastMac->Addr[5] = Ip->v6.Addr[11];
  }

  return EFI_SUCCESS;
}

/**
  Performs read and write operations on the NVRAM device attached to a network
  interface.

  This function performs read and write operations on the NVRAM device attached
  to a network interface. If ReadWrite is TRUE, a read operation is performed.
  If ReadWrite is FALSE, a write operation is performed. Offset specifies the
  byte offset at which to start either operation. Offset must be a multiple of
  NvRamAccessSize , and it must have a value between zero and NvRamSize.
  BufferSize specifies the length of the read or write operation. BufferSize must
  also be a multiple of NvRamAccessSize, and Offset + BufferSize must not exceed
  NvRamSize.
  If any of the above conditions is not met, then EFI_INVALID_PARAMETER will be
  returned.
  If all the conditions are met and the operation is "read," the NVRAM device
  attached to the network interface will be read into Buffer and EFI_SUCCESS
  will be returned. If this is a write operation, the contents of Buffer will be
  used to update the contents of the NVRAM device attached to the network
  interface and EFI_SUCCESS will be returned.

  It does the basic checking on the input parameters and retrieves snp structure
  and then calls the read_nvdata() call which does the actual reading

  @param Snp        A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param ReadWrite  TRUE for read operations, FALSE for write operations.
  @param Offset     Byte offset in the NVRAM device at which to start the read or
                    write operation. This must be a multiple of NvRamAccessSize
                    and less than NvRamSize. (See EFI_SIMPLE_NETWORK_MODE)
  @param BufferSize The number of bytes to read or write from the NVRAM device.
                    This must also be a multiple of NvramAccessSize.
  @param Buffer     A pointer to the data buffer.

  @retval EFI_SUCCESS           The NVRAM access was performed.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the following conditions is TRUE:
                                * The This parameter is NULL
                                * The This parameter does not point to a valid
                                  EFI_SIMPLE_NETWORK_PROTOCOL  structure
                                * The Offset parameter is not a multiple of
                                  EFI_SIMPLE_NETWORK_MODE.NvRamAccessSize
                                * The Offset parameter is not less than
                                  EFI_SIMPLE_NETWORK_MODE.NvRamSize
                                * The BufferSize parameter is not a multiple of
                                  EFI_SIMPLE_NETWORK_MODE.NvRamAccessSize
                                * The Buffer parameter is NULL
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network
                                interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network
                                interface.

**/
EFI_STATUS
EFIAPI
SnpNvData (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN  BOOLEAN                       ReadWrite,
  IN  UINTN                         Offset,
  IN  UINTN                         BufferSize,
  IN  OUT VOID                      *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Reads the current interrupt status and recycled transmit buffer status from a
  network interface.

  This function gets the current interrupt and recycled transmit buffer status
  from the network interface. The interrupt status is returned as a bit mask in
  InterruptStatus. If InterruptStatus is NULL, the interrupt status will not be
  read. If TxBuf is not NULL, a recycled transmit buffer address will be retrieved.
  If a recycled transmit buffer address is returned in TxBuf, then the buffer has
  been successfully transmitted, and the status for that buffer is cleared. If
  the status of the network interface is successfully collected, EFI_SUCCESS
  will be returned. If the driver has not been initialized, EFI_DEVICE_ERROR will
  be returned.

  @param Snp             A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param IrqStat         A pointer to the bit mask of the currently active
                         interrupts (see "Related Definitions"). If this is NULL,
                         the interrupt status will not be read from the device.
                         If this is not NULL, the interrupt status will be read
                         from the device. When the interrupt status is read, it
                         will also be cleared. Clearing the transmit interrupt does
                         not empty the recycled transmit buffer array.
  @param TxBuff          Recycled transmit buffer address. The network interface
                         will not transmit if its internal recycled transmit
                         buffer array is full. Reading the transmit buffer does
                         not clear the transmit interrupt. If this is NULL, then
                         the transmit buffer status will not be read. If there
                         are no transmit buffers to recycle and TxBuf is not NULL,
                         TxBuf will be set to NULL.

  @retval EFI_SUCCESS           The status of the network interface was retrieved.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER This parameter was NULL or did not point to a valid
                                EFI_SIMPLE_NETWORK_PROTOCOL structure.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network
                                interface.

**/
EFI_STATUS
EFIAPI
SnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  OUT  UINT32                        *IrqStat  OPTIONAL,
  OUT  VOID                          **TxBuff  OPTIONAL
  )
{
  EFI_STATUS                        Status;
  SOPHGO_SIMPLE_NETWORK_DRIVER      *DwMac4Driver;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  //
  // Update the media status
  //
  Status = PhyLinkAdjustGmacConfig (DwMac4Driver);
  if (EFI_ERROR (Status)) {
    DwMac4Driver->SnpMode.MediaPresent = FALSE;
  } else {
    DwMac4Driver->SnpMode.MediaPresent = TRUE;
  }

  if (TxBuff != NULL) {
    //
    // Get a recycled buf from DwMac4Driver->RecycledTxBuf
    //
   if (DwMac4Driver->RecycledTxBufCount == 0) {
      *TxBuff = NULL;
    } else {
      DwMac4Driver->RecycledTxBufCount--;
      *TxBuff = (VOID *)(UINTN) DwMac4Driver->RecycledTxBuf[DwMac4Driver->RecycledTxBufCount];
    }
  }

  return EFI_SUCCESS;
}

/**
  Places a packet in the transmit queue of a network interface.

  This function places the packet specified by Header and Buffer on the transmit
  queue. If HeaderSize is nonzero and HeaderSize is not equal to
  This->Mode->MediaHeaderSize, then EFI_INVALID_PARAMETER will be returned. If
  BufferSize is less than This->Mode->MediaHeaderSize, then EFI_BUFFER_TOO_SMALL
  will be returned. If Buffer is NULL, then EFI_INVALID_PARAMETER will be
  returned. If HeaderSize is nonzero and tAddr or Protocol is NULL, then
  EFI_INVALID_PARAMETER will be returned. If the transmit engine of the network
  interface is busy, then EFI_NOT_READY will be returned. If this packet can be
  accepted by the transmit engine of the network interface, the packet contents
  specified by Buffer will be placed on the transmit queue of the network
  interface, and EFI_SUCCESS will be returned. GetStatus() can be used to
  determine when the packet has actually been transmitted. The contents of the
  Buffer must not be modified until the packet has actually been transmitted.
  The Transmit() function performs nonblocking I/O. A caller who wants to perform
  blocking I/O, should call Transmit(), and then GetStatus() until the
  transmitted buffer shows up in the recycled transmit buffer.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param Snp        A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param HeadSize   The size, in bytes, of the media header to be filled in by the
                    Transmit() function. If HeaderSize is nonzero, then it must
                    be equal to This->Mode->MediaHeaderSize and the tAddr and
                    Protocol parameters must not be NULL.
  @param    The size, in bytes, of the entire packet (media header and
                    data) to be transmitted through the network interface.
  @param Data       A pointer to the packet (media header followed by data) to be
                    transmitted. This parameter cannot be NULL. If HeaderSize is
                    zero, then the media header in Buffer must already be filled
                    in by the caller. If HeaderSize is nonzero, then the media
                    header will be filled in by the Transmit() function.
  @param SrcAddr    The source HW MAC address. If HeaderSize is zero, then this
                    parameter is ignored. If HeaderSize is nonzero and SrcAddr
                    is NULL, then This->Mode->CurrentAddress is used for the
                    source HW MAC address.
  @param DstAddr    The tination HW MAC address. If HeaderSize is zero, then
                    this parameter is ignored.
  @param Protocol   The type of header to build. If HeaderSize is zero, then this
                    parameter is ignored. See RFC 1700, section "Ether Types,"
                    for examples.

  @retval EFI_SUCCESS           The packet was placed on the transmit queue.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_NOT_READY         The network interface is too busy to accept this
                                transmit request.
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported
                                value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.
  @retval EFI_ACCESS_DENIED     Error acquire global lock for operation.

**/
EFI_STATUS
EFIAPI
SnpTransmit (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *This,
  IN  UINTN                         HeadSize,
  IN  UINTN                         BufferSize,
  IN  VOID                          *Data,
  IN  EFI_MAC_ADDRESS               *SrcAddr  OPTIONAL,
  IN  EFI_MAC_ADDRESS               *DstAddr  OPTIONAL,
  IN  UINT16                        *Protocol OPTIONAL
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER      *DwMac4Driver;
  UINT32                            TxDescIndex;
  DMA_DESCRIPTOR                    *TxDescriptor;
  UINT8                             *EthernetPacket;
  UINT8                             *TxBuffer;
  VOID                              *MappingTxbuf;
  UINT64                            *Tmp;
  EFI_STATUS                        Status;
  UINTN                             DmaNumberOfBytes;
  UINT32                            Retries;

  MappingTxbuf = NULL;
  //
  // Setup DMA descriptor
  //
  EthernetPacket = Data;

  if ((This == NULL) || (Data == NULL)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid parameter (missing handle or buffer)\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  if (!DwMac4Driver->SnpMode.MediaPresent) {
    //
    // We should only really get here if the link was up
    // and is now down due to a stop/shutdown sequence, and
    // the app (grub) doesn't bother to check link state
    // because it was up a moment before.
    // Lets wait a bit for the link to resume, rather than
    // failing to send. In the case of grub it works either way
    // but we can't be sure that is universally true, and
    // hanging for a couple seconds is nicer than a screen of
    // grub send failure messages.
    //
    Retries = 1000;
    DEBUG ((DEBUG_INFO, "%a(): Waiting 5s for link\n", __func__));
    do {
      gBS->Stall (5000);
      Status = PhyLinkAdjustGmacConfig (DwMac4Driver);
    } while ((EFI_ERROR (Status)) && Retries-- > 0);

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a(): no link\n", __func__));
      return EFI_NOT_READY;
    } else {
      DwMac4Driver->SnpMode.MediaPresent = TRUE;
    }
  }

  //
  // Ensure header is correct size if non-zero
  //
  if (HeadSize) {
    if (HeadSize != DwMac4Driver->SnpMode.MediaHeaderSize) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Invalid parameter (header size mismatch; HeaderSize 0x%X, SnpMode.MediaHeaderSize 0x%X))\n",
        __func__,
        HeadSize,
        DwMac4Driver->SnpMode.MediaHeaderSize
      ));
      return EFI_INVALID_PARAMETER;
    }

    if ((DstAddr == NULL) || (Protocol == NULL)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Invalid parameter (dest addr or protocol missing)\n",
        __func__
      ));
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Check validity of BufferSize
  //
  if (BufferSize < DwMac4Driver->SnpMode.MediaHeaderSize) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Buffer too small\n",
      __func__
      ));
    return EFI_BUFFER_TOO_SMALL;
  }

  if ((DwMac4Driver->MaxRecycledTxBuf + SNP_TX_BUFFER_INCREASE) >= SNP_MAX_TX_BUFFER_NUM) {
    return EFI_NOT_READY;
  }

  //
  // Check preliminaries
  //
  Status = EfiAcquireLockOrFail (&DwMac4Driver->Lock);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Couldn't get lock: %r\n",
      __func__,
      Status
      ));
    return EFI_ACCESS_DENIED;
  }

  DwMac4Driver->MacDriver.TxCurrentDescriptorNum = DwMac4Driver->MacDriver.TxNextDescriptorNum;
  TxDescIndex = DwMac4Driver->MacDriver.TxCurrentDescriptorNum;

  TxDescriptor = DwMac4Driver->MacDriver.TxDescRing + TxDescIndex;
  TxBuffer = DwMac4Driver->MacDriver.TxBuffer + TxDescIndex * ETH_BUFFER_SIZE;

  if (HeadSize) {
    EthernetPacket[0] = DstAddr->Addr[0];
    EthernetPacket[1] = DstAddr->Addr[1];
    EthernetPacket[2] = DstAddr->Addr[2];
    EthernetPacket[3] = DstAddr->Addr[3];
    EthernetPacket[4] = DstAddr->Addr[4];
    EthernetPacket[5] = DstAddr->Addr[5];

    EthernetPacket[6] = SrcAddr->Addr[0];
    EthernetPacket[7] = SrcAddr->Addr[1];
    EthernetPacket[8] = SrcAddr->Addr[2];
    EthernetPacket[9] = SrcAddr->Addr[3];
    EthernetPacket[10] = SrcAddr->Addr[4];
    EthernetPacket[11] = SrcAddr->Addr[5];

    EthernetPacket[12] = (*Protocol & 0xFF00) >> 8;
    EthernetPacket[13] = *Protocol & 0xFF;
  }

  DmaNumberOfBytes = BufferSize;
  DEBUG ((DEBUG_VERBOSE, "%a(): Packet=0x%p, Length=0x%x\n", __func__, EthernetPacket, BufferSize));
  CopyMem ((VOID *)(UINTN)TxBuffer, EthernetPacket, BufferSize);
  Status = DmaMap (
    MapOperationBusMasterRead,
    (VOID *)(UINTN)TxBuffer,
    &DmaNumberOfBytes,
    &DwMac4Driver->MacDriver.TxBufNum[TxDescIndex].PhysAddress,
    &DwMac4Driver->MacDriver.TxBufNum[TxDescIndex].Mapping
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a() for Txbuffer: %r\n", __func__, Status));
    goto ReleaseLock;
  }
  MappingTxbuf = DwMac4Driver->MacDriver.TxBufNum[TxDescIndex].Mapping;

  TxDescriptor->Des0 = LOWER_32_BITS(DwMac4Driver->MacDriver.TxBufNum[TxDescIndex].PhysAddress);
  TxDescriptor->Des1 = UPPER_32_BITS(DwMac4Driver->MacDriver.TxBufNum[TxDescIndex].PhysAddress);
  TxDescriptor->Des2 = BufferSize;
  MemoryFence();

  //
  // Make sure that if HW sees the _OWN write below, it will see all the
  // writes to the rest of the Descriptor too.
  //
  TxDescriptor->Des3 |= (TDES3_OWN | TDES3_FIRST_DESCRIPTOR | TDES3_LAST_DESCRIPTOR);
  //
  // Increase Descriptor number
  //
  TxDescIndex++;
  TxDescIndex %= TX_DESC_NUM;
  DwMac4Driver->MacDriver.TxNextDescriptorNum = TxDescIndex;

  Status = EFI_SUCCESS;
  if (DwMac4Driver->RecycledTxBufCount < DwMac4Driver->MaxRecycledTxBuf) {
    DwMac4Driver->RecycledTxBuf[DwMac4Driver->RecycledTxBufCount] = (UINT64)(UINTN)Data;
    DwMac4Driver->RecycledTxBufCount ++;
  } else {
    Tmp = AllocatePool (sizeof (UINT64) * (DwMac4Driver->MaxRecycledTxBuf + SNP_TX_BUFFER_INCREASE));
    if (Tmp == NULL) {
      Status = EFI_DEVICE_ERROR;
    } else {
      CopyMem (Tmp, DwMac4Driver->RecycledTxBuf, sizeof (UINT64) * DwMac4Driver->RecycledTxBufCount);
      FreePool (DwMac4Driver->RecycledTxBuf);
      DwMac4Driver->RecycledTxBuf = Tmp;
      DwMac4Driver->MaxRecycledTxBuf += SNP_TX_BUFFER_INCREASE;
    }
  }

  StmmacSetTxTailPtr (
    DwMac4Driver,
    (UINTN)DwMac4Driver->MacDriver.TxDescRingMap[TX_DESC_NUM - 1].PhysAddress,
    0
  );

  if (MappingTxbuf != NULL) {
    DmaUnmap (MappingTxbuf);
  }

ReleaseLock:
  EfiReleaseLock (&DwMac4Driver->Lock);

  return Status;
}

/**
  Receives a packet from a network interface.

  This function retrieves one packet from the receive queue of a network interface.
  If there are no packets on the receive queue, then EFI_NOT_READY will be
  returned. If there is a packet on the receive queue, and the size of the packet
  is smaller than BufferSize, then the contents of the packet will be placed in
  Buffer, and BufferSize will be updated with the actual size of the packet.
  In addition, if SrcAddr, tAddr, and Protocol are not NULL, then these values
  will be extracted from the media header and returned. EFI_SUCCESS will be
  returned if a packet was successfully received.
  If BufferSize is smaller than the received packet, then the size of the receive
  packet will be placed in BufferSize and EFI_BUFFER_TOO_SMALL will be returned.
  If the driver has not been initialized, EFI_DEVICE_ERROR will be returned.

  @param Snp        A pointer to the EFI_SIMPLE_NETWORK_PROTOCOL instance.
  @param HeadSize   The size, in bytes, of the media header received on the network
                    interface. If this parameter is NULL, then the media header size
                    will not be returned.
  @param    On entry, the size, in bytes, of Buffer. On exit, the size, in
                    bytes, of the packet that was received on the network interface.
  @param Data       A pointer to the data buffer to receive both the media
                    header and the data.
  @param SrcAddr    The source HW MAC address. If this parameter is NULL, the HW
                    MAC source address will not be extracted from the media header.
  @param DstAddr   The tination HW MAC address. If this parameter is NULL,
                    the HW MAC tination address will not be extracted from
                    the media header.
  @param Protocol   The media header type. If this parameter is NULL, then the
                    protocol will not be extracted from the media header. See
                    RFC 1700 section "Ether Types" for examples.

  @retval EFI_SUCCESS           The received data was stored in Buffer, and
                                BufferSize has been updated to the number of
                                bytes received.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_NOT_READY         No packets have been received on the network interface.
  @retval EFI_BUFFER_TOO_SMALL  BufferSize is too small for the received packets.
                                BufferSize has been updated to the required size.
  @retval EFI_INVALID_PARAMETER One or more of the following conditions is TRUE:
                                * The This parameter is NULL
                                * The This parameter does not point to a valid
                                  EFI_SIMPLE_NETWORK_PROTOCOL structure.
                                * The BufferSize parameter is NULL
                                * The Buffer parameter is NULL
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_ACCESS_DENIED     Error acquire global lock for operation.

**/
EFI_STATUS
EFIAPI
SnpReceive (
  IN       EFI_SIMPLE_NETWORK_PROTOCOL   *This,
      OUT  UINTN                         *HeadSize      OPTIONAL,
  IN  OUT  UINTN                         *BufferSize,
      OUT  VOID                          *Data,
      OUT  EFI_MAC_ADDRESS               *SrcAddr      OPTIONAL,
      OUT  EFI_MAC_ADDRESS               *DstAddr      OPTIONAL,
      OUT  UINT16                        *Protocol     OPTIONAL
  )
{
  SOPHGO_SIMPLE_NETWORK_DRIVER      *DwMac4Driver;
  EFI_MAC_ADDRESS                   Dst;
  EFI_MAC_ADDRESS                   Src;
  UINT32                            Length;
  UINT32                            RxDescriptorStatus;
  UINT8                             *RawData;
  UINT32                            RxDescIndex;
  DMA_DESCRIPTOR                    *RxDescriptor;
  UINTN                             BufferSizeBuf;
  UINTN                             *RxBufferAddr;
  EFI_STATUS                        Status;

  BufferSizeBuf = ETH_BUFFER_SIZE;

  //
  // Check preliminaries
  //
  if ((This == NULL) || (Data == NULL)) {
    DEBUG ((DEBUG_ERROR, "%a(): Invalid parameter (missing handle or buffer)\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  DwMac4Driver = INSTANCE_FROM_SNP_THIS (This);
  if (DwMac4Driver->SnpMode.State != EfiSimpleNetworkInitialized) {
    return EFI_DEVICE_ERROR;
  }

  if (DwMac4Driver->SnpMode.State == EfiSimpleNetworkStopped) {
    return EFI_NOT_STARTED;
  }

  Status = EfiAcquireLockOrFail (&DwMac4Driver->Lock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a(): Couldn't get lock: %r\n", __func__, Status));
    return EFI_ACCESS_DENIED;
  }

  //
  // Get Rx descriptor
  //
  DwMac4Driver->MacDriver.RxCurrentDescriptorNum = DwMac4Driver->MacDriver.RxNextDescriptorNum;
  RxDescIndex = DwMac4Driver->MacDriver.RxCurrentDescriptorNum;
  RxDescriptor = DwMac4Driver->MacDriver.RxDescRing + RxDescIndex;

  RxBufferAddr = (UINTN *)(DwMac4Driver->MacDriver.RxBuffer + RxDescIndex * BufferSizeBuf);

  RawData = (UINT8 *) Data;

  //
  // Write-Back: Get Rx Status
  //
  RxDescriptorStatus = RxDescriptor->Des3;
  if (RxDescriptorStatus & RDES3_OWN) {
    DEBUG ((DEBUG_VERBOSE, "%a(): RX packet not available!\n", __func__));
    Status = EFI_NOT_READY;
    goto ReleaseLock;
  }


  if (RxDescriptorStatus & RDES3_ERROR_SUMMARY) {
    //
    // Check for errors
    //
    if (RxDescriptorStatus & RDES3_CRC_ERROR) {
      DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: CRC Error\n", __func__));
    }

    if (RxDescriptorStatus & RDES3_DRIBBLE_ERROR) {
      DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: Dribble Bit Error\n", __func__));
    }

    if (RxDescriptorStatus & RDES3_RECEIVE_ERROR) {
      DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: Receive Error\n", __func__));
    }

    if (RxDescriptorStatus & RDES3_RECEIVE_WATCHDOG) {
      DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: Watchdog Timeout\n", __func__));
    }

    if (RxDescriptorStatus & RDES3_OVERFLOW_ERROR) {
      DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: Overflow Error\n", __func__));
    }

    if (RxDescriptorStatus & RDES3_GIANT_PACKET) {
      DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: Giant Packet\n",  __func__));
    }

    Status = EFI_DEVICE_ERROR;
    goto ReleaseLock;
  }

  Length = RxDescriptorStatus & RDES3_PACKET_SIZE_MASK;
  DEBUG ((DEBUG_VERBOSE, "%a(): RxBufferAddr=0x%lx, Length = %d\n", __func__, RxBufferAddr, Length));
  if (!Length) {
    DEBUG ((DEBUG_WARN, "%a(): Error: Invalid Frame Packet length\n", __func__));
    Status = EFI_NOT_READY;
    goto ReleaseLock;
  }

  //
  // Check buffer size
  //
  if (*BufferSize < Length) {
    DEBUG ((
      DEBUG_WARN,
      "%a(): Error: Buffer size is too small\n",
      __func__
      ));
    Status = EFI_BUFFER_TOO_SMALL;
    goto ReleaseLock;
  }

  *BufferSize = Length;

  if (RxDescriptor->Des1 & RDES1_IP_CSUM_ERROR) {
    DEBUG ((DEBUG_WARN, "%a(): Rx decritpor Status Error: IP Payload Error\n",  __func__));

    Status = EFI_DEVICE_ERROR;
    goto ReleaseLock;
  }

  if (HeadSize != NULL) {
    *HeadSize = DwMac4Driver->SnpMode.MediaHeaderSize;
  }

  //
  // Given an RX buffer descriptor index, undo the DmaMap operation on the buffer.
  //
  DmaUnmap (DwMac4Driver->MacDriver.RxBufNum[RxDescIndex].Mapping);
  CopyMem (RawData, (VOID *)RxBufferAddr, *BufferSize);

  if (DstAddr != NULL) {
    Dst.Addr[0] = RawData[0];
    Dst.Addr[1] = RawData[1];
    Dst.Addr[2] = RawData[2];
    Dst.Addr[3] = RawData[3];
    Dst.Addr[4] = RawData[4];
    Dst.Addr[5] = RawData[5];

    CopyMem (DstAddr, &Dst, NET_ETHER_ADDR_LEN);

    DEBUG ((
      DEBUG_INFO,
      "%a(): Received from source address %x %x\r\n",
      __func__,
      DstAddr,
      &Dst
      ));
  }

  //
  // Get the source address
  //
  if (SrcAddr != NULL) {
    Src.Addr[0] = RawData[6];
    Src.Addr[1] = RawData[7];
    Src.Addr[2] = RawData[8];
    Src.Addr[3] = RawData[9];
    Src.Addr[4] = RawData[10];
    Src.Addr[5] = RawData[11];

    DEBUG ((
      DEBUG_INFO,
      "%a(): Received from source address %x %x\r\n",
      __func__,
      SrcAddr,
      &Src
      ));

    CopyMem (SrcAddr, &Src, NET_ETHER_ADDR_LEN);
  }

  //
  // Get the protocol
  //
  if (Protocol != NULL) {
    *Protocol = NTOHS (RawData[12] | (RawData[13] >> 8) |
                      (RawData[14] >> 16) | (RawData[15] >> 24));
  }

  //
  // DMA map for the current receive buffer
  //
  Status = DmaMap (
    MapOperationBusMasterWrite,
    (VOID *)RxBufferAddr,
    &BufferSizeBuf,
    &DwMac4Driver->MacDriver.RxBufNum[RxDescIndex].PhysAddress,
    &DwMac4Driver->MacDriver.RxBufNum[RxDescIndex].Mapping
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a() for Rxbuffer: %r\n", __func__, Status));

    goto ReleaseLock;
  }

  RxDescriptor->Des0 = LOWER_32_BITS(DwMac4Driver->MacDriver.RxBufNum[RxDescIndex].PhysAddress);
  RxDescriptor->Des1 = UPPER_32_BITS(DwMac4Driver->MacDriver.RxBufNum[RxDescIndex].PhysAddress);
  RxDescriptor->Des3 |= (RDES3_OWN | RDES3_BUFFER1_VALID_ADDR);

  //
  // Increase descriptor number
  //
  RxDescIndex++;
  RxDescIndex %= RX_DESC_NUM;
  DwMac4Driver->MacDriver.RxNextDescriptorNum = RxDescIndex;
  StmmacSetRxTailPtr (
    DwMac4Driver,
    (UINTN)DwMac4Driver->MacDriver.RxDescRingMap[RX_DESC_NUM - 1].PhysAddress,
    0
  );

  Status = EFI_SUCCESS;

ReleaseLock:
  EfiReleaseLock (&DwMac4Driver->Lock);

  return Status;
}

/*
 * Entry point for DwMac4 driver.
 */
EFI_STATUS
EFIAPI
DwMac4SnpDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                        Status;
  SOPHGO_SIMPLE_NETWORK_DRIVER      *DwMac4Driver;
  EFI_SIMPLE_NETWORK_PROTOCOL       *Snp;
  EFI_SIMPLE_NETWORK_MODE           *SnpMode;
  SOPHGO_SIMPLE_NETWORK_DEVICE_PATH *DevicePath;
  UINT64                            DefaultMacAddress;
  EFI_MAC_ADDRESS                   *SwapMacAddressPtr;
  EFI_HANDLE                        Handle;
  INT32                             Node;
  CONST VOID                        *Prop;
  FDT_CLIENT_PROTOCOL               *FdtClient;

  Handle = NULL;

  //
  // Extract reg addr from device tree
  //
  Status = gBS->LocateProtocol (
    &gFdtClientProtocolGuid,
    NULL,
    (VOID **) &FdtClient
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a(): Failed to locate FDT_CLIENT_PROTOCOL\n", __func__));
    return Status;
  }

  Status = FdtClient->FindCompatibleNode (
    FdtClient,
    "sophgo,sg2042-dwmac",
    &Node
  );
  if (Status == EFI_NOT_FOUND) {
    DEBUG ((DEBUG_ERROR, "%a(): Failed to find ethernet node\n", __func__));
    return Status;
  }

  Status = FdtClient->GetNodeProperty (
    FdtClient,
    Node,
    "reg",
    &Prop,
    NULL
  );
  if (Status == EFI_NOT_FOUND) {
    DEBUG ((DEBUG_ERROR, "%a(): Failed to get reg's base addr\n", __func__));
    return Status;
  }

  //
  // Allocate Resources
  //
  DwMac4Driver = AllocatePages (EFI_SIZE_TO_PAGES (sizeof (SOPHGO_SIMPLE_NETWORK_DRIVER)));
  if (DwMac4Driver == NULL) {
    DEBUG ((DEBUG_ERROR, "%a() for Snp is NULL!\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  DevicePath = (SOPHGO_SIMPLE_NETWORK_DEVICE_PATH *)AllocateCopyPool (
    sizeof (SOPHGO_SIMPLE_NETWORK_DEVICE_PATH),
    &PathTemplate
  );

  if (DevicePath == NULL) {
    DEBUG ((DEBUG_ERROR, "%a() for DeivcePath is NULL!\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = StmmacAllocDesc(DwMac4Driver);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Initialized signature (used by INSTANCE_FROM_SNP_THIS macro)
  //
  DwMac4Driver->Signature = SNP_DRIVER_SIGNATURE;

  EfiInitializeLock (&DwMac4Driver->Lock, TPL_CALLBACK);

  //
  // Initialize pointers
  //
  Snp = &DwMac4Driver->Snp;
  SnpMode = &DwMac4Driver->SnpMode;
  Snp->Mode = SnpMode;

  //
  // Get MAC controller base address
  //
  DwMac4Driver->RegBase = SwapBytes64 (((CONST UINT64 *) Prop)[0]);

  //
  // Assign fields and func pointers
  //
  Snp->Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  Snp->WaitForPacket  = NULL;
  Snp->Initialize     = SnpInitialize;
  Snp->Start          = SnpStart;
  Snp->Stop           = SnpStop;
  Snp->Reset          = SnpReset;
  Snp->Shutdown       = SnpShutdown;
  Snp->ReceiveFilters = SnpReceiveFilters;
  Snp->StationAddress = SnpStationAddress;
  Snp->Statistics     = SnpStatistics;
  Snp->MCastIpToMac   = SnpMcastIptoMac;
  Snp->NvData         = SnpNvData;
  Snp->GetStatus      = SnpGetStatus;
  Snp->Transmit       = SnpTransmit;
  Snp->Receive        = SnpReceive;

  DwMac4Driver->RecycledTxBuf = AllocatePool (sizeof (UINT64) * SNP_TX_BUFFER_INCREASE);
  if (DwMac4Driver->RecycledTxBuf == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  }

  DwMac4Driver->MaxRecycledTxBuf = SNP_TX_BUFFER_INCREASE;
  DwMac4Driver->RecycledTxBufCount = 0;

  //
  // Start completing simple network mode structure
  //
  SnpMode->State           = EfiSimpleNetworkStopped;
  SnpMode->HwAddressSize   = NET_ETHER_ADDR_LEN;    // HW address is 6 bytes
  SnpMode->MediaHeaderSize = sizeof (ETHER_HEAD);
  SnpMode->MaxPacketSize   = EFI_PAGE_SIZE;         // Preamble + SOF + Ether Frame (with VLAN tag +4bytes)
  SnpMode->NvRamSize       = 0;                     // No NVRAM with this device
  SnpMode->NvRamAccessSize = 0;                     // No NVRAM with this device

  //
  // Update network mode information
  //
  SnpMode->ReceiveFilterMask = \
    EFI_SIMPLE_NETWORK_RECEIVE_UNICAST     |
    EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST   |
    EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST   |
    EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
    EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  //
  // We do not intend to receive anything for the time being.
  //
  SnpMode->ReceiveFilterSetting = \
    EFI_SIMPLE_NETWORK_RECEIVE_UNICAST   |
    EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
    EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;

  //
  // GMAC has 64bit hash table, can filter 64 MCast MAC Addresses
  //
  SnpMode->MaxMCastFilterCount = MAX_MCAST_FILTER_CNT;
  SnpMode->MCastFilterCount    = 0;
  ZeroMem (&SnpMode->MCastFilter, MAX_MCAST_FILTER_CNT * sizeof (EFI_MAC_ADDRESS));

  //
  // Set the interface type (1: Ethernet or 6: IEEE 802 Networks)
  //
  SnpMode->IfType = NET_IFTYPE_ETHERNET;

  //
  // Mac address is changeable as it is loaded from erasable memory
  //
  SnpMode->MacAddressChangeable = TRUE;

  //
  // Can only transmit one packet at a time
  //
  SnpMode->MultipleTxSupported = FALSE;

  //
  // MediaPresent checks for cable connection and partner link
  //
  SnpMode->MediaPresentSupported = TRUE;
  SnpMode->MediaPresent = FALSE;

  //
  // Set broadcast address
  //
  SetMem (&SnpMode->BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  //
  // Set current address.
  // Try parse conf.ini first to get mac address
  //
  if (IsIniFileExist ()) {
    MacAddrIniParser ();
    DefaultMacAddress = MacConfig.Mac0Addr;
  } else {
    DEBUG ((DEBUG_WARN, "%a() Warning: using driver-default MAC address\n", __func__));
    DefaultMacAddress = FixedPcdGet64 (PcdDwMac4DefaultMacAddress);
  }

  CopyMem (&Snp->Mode->CurrentAddress, &DefaultMacAddress, NET_ETHER_ADDR_LEN);

  //
  // Swap PCD human readable form to correct endianess
  //
  SwapMacAddressPtr = (EFI_MAC_ADDRESS *) &DefaultMacAddress;
  SnpMode->CurrentAddress.Addr[0] = SwapMacAddressPtr->Addr[5];
  SnpMode->CurrentAddress.Addr[1] = SwapMacAddressPtr->Addr[4];
  SnpMode->CurrentAddress.Addr[2] = SwapMacAddressPtr->Addr[3];
  SnpMode->CurrentAddress.Addr[3] = SwapMacAddressPtr->Addr[2];
  SnpMode->CurrentAddress.Addr[4] = SwapMacAddressPtr->Addr[1];
  SnpMode->CurrentAddress.Addr[5] = SwapMacAddressPtr->Addr[0];

  //
  // Assign fields for device path
  //
  CopyMem (&DevicePath->MacAddrDP.MacAddress, &Snp->Mode->CurrentAddress, NET_ETHER_ADDR_LEN);
  DevicePath->MacAddrDP.IfType = Snp->Mode->IfType;

  Status = gBS->InstallMultipleProtocolInterfaces (
    &Handle,
    &gEfiSimpleNetworkProtocolGuid,
    Snp,
    &gEfiDevicePathProtocolGuid,
    DevicePath,
    NULL
  );

  if (EFI_ERROR(Status)) {
    FreePages (DwMac4Driver, EFI_SIZE_TO_PAGES (sizeof (SOPHGO_SIMPLE_NETWORK_DRIVER)));
    DEBUG ((DEBUG_ERROR, "%a(): Install SIMPLE_NETWORK_PROTOCOL failed!\n", __func__));
  }

  return Status;
}
