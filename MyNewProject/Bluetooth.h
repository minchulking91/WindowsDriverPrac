
#pragma once

#include "Driver.h"

#define BLUETOOTH_POOL_TAG '_htB'
#define BUFFER_POOL_TAG '_fuB'
#define SYNCHRONOUS_CALL_TIMEOUT (-1000000000) // 1 Second


typedef struct _BRB_L2CA_OPEN_CHANNEL * PBRB_L2CA_OPEN_CHANNEL;
typedef struct _BRB_L2CA_CLOSE_CHANNEL  * PBRB_L2CA_CLOSE_CHANNEL;
typedef struct _BRB_L2CA_ACL_TRANSFER  * PBRB_L2CA_ACL_TRANSFER;

NTSTATUS PrepareBluetooth(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS OpenChannels(_In_ PDEVICE_CONTEXT DeviceContext);
EVT_WDF_REQUEST_COMPLETION_ROUTINE ControlChannelCompletion;
EVT_WDF_REQUEST_COMPLETION_ROUTINE InterruptChannelCompletion;

NTSTATUS CloseChannels(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS CreateRequestAndBuffer(_In_ WDFDEVICE Device, _In_ WDFIOTARGET IoTarget, _In_ SIZE_T BufferSize, _Outptr_ WDFREQUEST * Request, _Outptr_ WDFMEMORY * Memory, _Outptr_opt_result_buffer_(BufferSize) PVOID * Buffer);

NTSTATUS StartContiniousReader(_In_ PDEVICE_CONTEXT DeviceContext);
EVT_WDF_REQUEST_COMPLETION_ROUTINE ReadFromDeviceCompletion;

