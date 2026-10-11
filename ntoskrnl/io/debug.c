/*
 * PROJECT:         ReactOS Kernel
 * COPYRIGHT:       GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * FILE:            ntoskrnl/io/debug.c
 * PURPOSE:         Useful functions for debugging IO and PNP managers
 * PROGRAMMERS:     Copyright 2020 Vadim Galyant <vgal@rambler.ru>
 */

#include <ntoskrnl.h>
#include "pnpio.h"

#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

extern PDEVICE_NODE IopRootDeviceNode;


VOID
NTAPI
PipDumpCmResourceDescriptor(
    _In_ PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    _In_ ULONG DebugLevel)
{
    PAGED_CODE();

    if (DebugLevel != 0)
    {
#ifdef NDEBUG
        return;
#endif
    }

    if (Descriptor == NULL)
    {
        DPRINT1("Dump CmDescriptor: Descriptor == NULL\n");
        return;
    }

    switch (Descriptor->Type)
    {
        case CmResourceTypePort:
            DPRINT1("[%p:%X:%X] IO:  Start %X:%X, Len %X\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.Port.Start.HighPart, Descriptor->u.Port.Start.LowPart, Descriptor->u.Port.Length);
            break;

        case CmResourceTypeInterrupt:
            DPRINT1("[%p:%X:%X] INT: Lev %X Vec %X Aff %IX\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.Interrupt.Level, Descriptor->u.Interrupt.Vector, Descriptor->u.Interrupt.Affinity);
            break;

        case CmResourceTypeMemory:
            DPRINT1("[%p:%X:%X] MEM: %X:%X Len %X\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.Memory.Start.HighPart, Descriptor->u.Memory.Start.LowPart, Descriptor->u.Memory.Length);
            break;

        case CmResourceTypeDma:
            DPRINT1("[%p:%X:%X] DMA: Channel %X Port %X\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.Dma.Channel, Descriptor->u.Dma.Port);
            break;

        case CmResourceTypeDeviceSpecific:
            DPRINT1("[%p:%X:%X] DAT: DataSize %X\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.DeviceSpecificData.DataSize);
            break;

        case CmResourceTypeBusNumber:
            DPRINT1("[%p:%X:%X] BUS: Start %X Len %X Reserv %X\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.BusNumber.Start, Descriptor->u.BusNumber.Length, Descriptor->u.BusNumber.Reserved);
            break;

        case CmResourceTypeDevicePrivate:
            DPRINT1("[%p:%X:%X] PVT: D[0] %X D[1] %X D[2] %X\n", Descriptor, Descriptor->ShareDisposition, Descriptor->Flags, Descriptor->u.DevicePrivate.Data[0], Descriptor->u.DevicePrivate.Data[1], Descriptor->u.DevicePrivate.Data[2]);
            break;

        default:
            DPRINT1("[%p] Unknown type %X\n", Descriptor, Descriptor->Type);
            break;
    }
}


PCM_PARTIAL_RESOURCE_DESCRIPTOR
NTAPI
PipGetNextCmPartialDescriptor(
    _In_ PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor)
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR NextDescriptor;

    NextDescriptor = CmDescriptor + 1;

    if (CmDescriptor->Type == CmResourceTypeDeviceSpecific)
    {
        NextDescriptor = (PVOID)((ULONG_PTR)NextDescriptor +
                                 CmDescriptor->u.DeviceSpecificData.DataSize);
    }

    return NextDescriptor;
}


VOID
NTAPI
PipDumpCmResourceList(
    _In_ PCM_RESOURCE_LIST CmResource,
    _In_ ULONG DebugLevel)
{
    PCM_FULL_RESOURCE_DESCRIPTOR FullList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    ULONG ix;
    ULONG jx;

    PAGED_CODE();

    if (DebugLevel != 0)
    {
#ifdef NDEBUG
        return;
#endif
    }

    DPRINT1("Dump CmList: CmResource %p\n", CmResource);

    if (CmResource == NULL)
    {
        DPRINT1("PipDumpCmResourceList: CmResource == NULL\n");
        return;
    }

    if (CmResource->Count == 0)
    {
        DPRINT1("PipDumpCmResourceList: CmResource->Count == 0\n");
        return;
    }

    DPRINT1("FullList Count %x\n", CmResource->Count);

    FullList = &CmResource->List[0];

    for (ix = 0; ix < CmResource->Count; ix++)
    {
        DPRINT1("List #%X Iface %X Bus #%X Ver.%X Rev.%X Count %X\n",
                ix,
                FullList->InterfaceType,
                FullList->BusNumber,
                FullList->PartialResourceList.Version,
                FullList->PartialResourceList.Revision,
                FullList->PartialResourceList.Count);

        Descriptor = FullList->PartialResourceList.PartialDescriptors;

        for (jx = 0; jx < FullList->PartialResourceList.Count; jx++)
        {
            PipDumpCmResourceDescriptor(Descriptor, DebugLevel);
            Descriptor = PipGetNextCmPartialDescriptor(Descriptor);
        }

        FullList = (PCM_FULL_RESOURCE_DESCRIPTOR)Descriptor;
    }
}

/* IoResource */

/* PipDumpIoResourceDescriptor() displays information about a Io Descriptor

   DebugLevel: 0 - always dump
               1 - dump if not defined NDEBUG
*/
VOID
NTAPI
PipDumpIoResourceDescriptor(
    _In_ PIO_RESOURCE_DESCRIPTOR Descriptor,
    _In_ ULONG DebugLevel)
{
    PAGED_CODE();

    if (DebugLevel != 0)
    {
#ifdef NDEBUG
        return;
#endif
    }

    if (Descriptor == NULL)
    {
        DPRINT1("DumpResourceDescriptor: Descriptor == 0\n");
        return;
    }

    switch (Descriptor->Type)
    {
        case CmResourceTypeNull:
            DPRINT1("[%p:%X:%X] O: Len %X Align %X Min %I64X, Max %I64X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.Generic.Length, Descriptor->u.Generic.Alignment, Descriptor->u.Generic.MinimumAddress.QuadPart, Descriptor->u.Generic.MaximumAddress.QuadPart);
            break;

        case CmResourceTypePort:
            DPRINT1("[%p:%X:%X] IO: Min %X:%X, Max %X:%X, Align %X Len %X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.Port.MinimumAddress.HighPart, Descriptor->u.Port.MinimumAddress.LowPart, Descriptor->u.Port.MaximumAddress.HighPart, Descriptor->u.Port.MaximumAddress.LowPart, Descriptor->u.Port.Alignment, Descriptor->u.Port.Length);
            break;

        case CmResourceTypeInterrupt:
            DPRINT1("[%p:%X:%X] INT: Min %X Max %X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.Interrupt.MinimumVector, Descriptor->u.Interrupt.MaximumVector);
            break;

        case CmResourceTypeMemory:
            DPRINT1("[%p:%X:%X] MEM: Min %X:%X, Max %X:%X, Align %X Len %X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.Memory.MinimumAddress.HighPart, Descriptor->u.Memory.MinimumAddress.LowPart, Descriptor->u.Memory.MaximumAddress.HighPart, Descriptor->u.Memory.MaximumAddress.LowPart, Descriptor->u.Memory.Alignment, Descriptor->u.Memory.Length);
            break;

        case CmResourceTypeDma:
            DPRINT1("[%p:%X:%X] DMA: Min %X Max %X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.Dma.MinimumChannel, Descriptor->u.Dma.MaximumChannel);
            break;

        case CmResourceTypeBusNumber:
            DPRINT1("[%p:%X:%X] BUS: Min %X Max %X Len %X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.BusNumber.MinBusNumber, Descriptor->u.BusNumber.MaxBusNumber, Descriptor->u.BusNumber.Length);
            break;

        case CmResourceTypeConfigData:
            DPRINT1("[%p:%X:%X] CFG: Priority %X\n", Descriptor, Descriptor->Option, Descriptor->ShareDisposition, Descriptor->u.DeviceSpecificData.DataSize);
            break;

        default:
            DPRINT1("[%p] Unknown type %X\n", Descriptor, Descriptor->Type);
            break;
    }
}
