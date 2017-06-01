#pragma once

#include <initguid.h>
#include <ntddk.h>
#include <wdf.h>
#include <hidport.h>

#include "Common.h"


#include <bthdef.h>
#include <bthguid.h>
#include <bthioctl.h>
#include <sdpnode.h>
#include <bthddi.h>
#include <bthsdpddi.h>
#include <bthsdpdef.h>




#define HIDMINI_PID             0x0001					//나중에 PID VID 수정할 것.
#define HIDMINI_VID             0x0401
#define HIDMINI_VERSION         0x0101
#define CONTROL_FEATURE_REPORT_ID   0x01

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

EVT_WDF_DRIVER_DEVICE_ADD           EvtDeviceAdd;
DRIVER_INITIALIZE                   DriverEntry;
EVT_WDF_TIMER                       EvtTimerFunc;

typedef struct _BLUETOOTH_DEVICE_CONTEXT
{
	BTH_PROFILE_DRIVER_INTERFACE ProfileDriverInterface;

	BTH_ADDR DeviceAddress;
	L2CAP_CHANNEL_HANDLE ControlChannelHandle;
	L2CAP_CHANNEL_HANDLE InterruptChannelHandle;

	size_t ReadBufferSize;

} BLUETOOTH_DEVICE_CONTEXT, *PBLUETOOTH_DEVICE_CONTEXT;


typedef struct _DEVICE_CONTEXT							//나중에 수정할게 있는지 꼭 확인할 것.
{
	WDFDEVICE               Device;
	WDFQUEUE                DefaultQueue;
	WDFQUEUE                ManualQueue;
	HID_DEVICE_ATTRIBUTES   HidDeviceAttributes;
	BYTE                    DeviceData;
	HID_DESCRIPTOR          HidDescriptor;
	PHID_REPORT_DESCRIPTOR  ReportDescriptor;
	BOOLEAN                 ReadReportDescFromRegistry;
	BLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	WDFIOTARGET IoTarget;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext);





typedef struct _QUEUE_CONTEXT
{
	WDFQUEUE                Queue;
	PDEVICE_CONTEXT         DeviceContext;
	UCHAR                   OutputReport;

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, GetQueueContext);

typedef struct _MANUAL_QUEUE_CONTEXT
{
	WDFQUEUE                Queue;
	PDEVICE_CONTEXT         DeviceContext;
	WDFTIMER                Timer;

} MANUAL_QUEUE_CONTEXT, *PMANUAL_QUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(MANUAL_QUEUE_CONTEXT, GetManualQueueContext);

EVT_WDF_DEVICE_PREPARE_HARDWARE PrepareHardware;
EVT_WDF_DEVICE_D0_ENTRY DeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT DeviceD0Exit;

NTSTATUS
QueueCreate(
	_In_  WDFDEVICE         Device,
	_Out_ WDFQUEUE          *Queue
);

NTSTATUS
ManualQueueCreate(
	_In_  WDFDEVICE         Device,
	_Out_ WDFQUEUE          *Queue
);

NTSTATUS
GetString(
	_In_  WDFREQUEST        Request
);

NTSTATUS
GetIndexedString(
	_In_  WDFREQUEST        Request
);

NTSTATUS
GetStringId(
	_In_  WDFREQUEST        Request,
	_Out_ ULONG            *StringId,
	_Out_ ULONG            *LanguageId
);

NTSTATUS
GetFeature(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
);

NTSTATUS
SetFeature(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
);

NTSTATUS
GetInputReport(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
);

NTSTATUS
SetOutputReport(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
);

NTSTATUS
ReadReport(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request,
	_Always_(_Out_)
	BOOLEAN*          CompleteRequest
);

NTSTATUS
RequestCopyFromBuffer(
	_In_  WDFREQUEST        Request,
	_In_  PVOID             SourceBuffer,
	_When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
	_In_  size_t            NumBytesToCopyFrom
);

NTSTATUS
RequestGetHidXferPacket_ToReadFromDevice(
	_In_  WDFREQUEST        Request,
	_Out_ HID_XFER_PACKET  *Packet
);

NTSTATUS
RequestGetHidXferPacket_ToWriteToDevice(
	_In_  WDFREQUEST        Request,
	_Out_ HID_XFER_PACKET  *Packet
);