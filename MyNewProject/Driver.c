#include "Driver.h"






HID_REPORT_DESCRIPTOR HIDReportDescriptor[] = {
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)		
	0x09, 0x04,        // Usage (Joy Stick)							
	0xA1, 0x02,        // Collection (Logical)
	0xA1, 0x01,        //   Collection (Application)				//우리는 물리적 장치가 아니므로 Logical로 감.
	0x85, 0x01,        //     Report ID (1)							// 첫번째 사용자. 둘 이상의 사용자를 지정하려면, 아래 서술된 내용을 복사 붙여넣기 후 이 값을 각각의 숫자로 바꿔주면 된다.
	0x09, 0x30,        //     Usage (X)								
	0x09, 0x31,        //     Usage (Y)								// 왼쪽 스틱의 X,Y 값
	0x09, 0x32,        //     Usage (Rx)
	0x09, 0x35,        //     Usage (RY)							// 오른쪽 스틱의 X,Y 값. 
	0x15, 0x00,        //     Logical Minimum (0)					// 
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)					// X,Y 값의 최소값, 최대값. (0~127은 왼쪽과 아래, 128~256은 오른쪽,위로 가정하고 나중에 수정하자)
	0x75, 0x08,        //     Report Size (8)						
	0x95, 0x04,        //     Report Count (4)						
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)			
	0x05, 0x09,        //     Usage Page (Button)					
	0x19, 0x01,        //     Usage Minimum (0x01)
	0x29, 0x0E,        //     Usage Maximum (0x0E)					//버튼은 1개부터 14개 존재. 동글,세모,네모,X,start,select,R1,L1,R2,L2,R3,L3
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)					//각 버튼은 0 이나 1 값을 가짐.
	0x75, 0x01,        //     Report Size (1)						//1비트로 값을 표현 가능하므로 1
	0x95, 0x0E,        //     Report Count (14)						//버튼이 14개므로 14
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x75, 0x01,        //     Report Size (2)						//1비트로 값을 표현 가능하므로 1
	0x95, 0x0E,        //     Report Count (1)						//버튼이 14개므로 14
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0xC0,              // End Collection
};

/*
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Report ID							    |
|--------|-------|--------|-------|--------|-------|--------|-------|
|								X-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Y-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Rx-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|			 					Ry-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Button 1-8							|
|--------|-------|--------|-------|--------|-------|--------|-------|
|			  		Button 9-13			 		   | xxx Padding xxx|
|--------|-------|--------|-------|--------|-------|--------|-------|

*/



HID_DESCRIPTOR HIDDescriptor = {
	0x09,   // length of HID descriptor
	0x21,   // descriptor type == HID  0x21
	0x0111, // hid spec release
	0x00,   // country code == Not Specified
	0x01,   // number of HID class descriptors
	{
		0x22, // descriptor type 
		sizeof(HIDReportDescriptor) // total length of report descriptor
	}
};









NTSTATUS
DriverEntry(
	_In_  PDRIVER_OBJECT    DriverObject,
	_In_  PUNICODE_STRING   RegistryPath
)
{
	NTSTATUS status;
	WDF_DRIVER_CONFIG config;

	KdPrint(("DriverEntry\n"));

	WDF_DRIVER_CONFIG_INIT(&config, EvtDeviceAdd);

	status = WdfDriverCreate(DriverObject,
		RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		WDF_NO_HANDLE);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Error: WdfDriverCreate failed 0x%x\n", status));
		return status;
	}

	return status;
}

NTSTATUS
EvtDeviceAdd(
	_In_  WDFDRIVER         Driver,
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
{
	NTSTATUS                status;
	WDF_OBJECT_ATTRIBUTES   deviceAttributes;
	WDFDEVICE               device;
	PDEVICE_CONTEXT         deviceContext;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	PHID_DEVICE_ATTRIBUTES  hidAttributes;
	UNREFERENCED_PARAMETER(Driver);

	KdPrint(("Enter EvtDeviceAdd\n"));

	WdfFdoInitSetFilter(DeviceInit);

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

//	pnpPowerCallbacks.EvtDevicePrepareHardware = PrepareHardware;
//	pnpPowerCallbacks.EvtDeviceReleaseHardware = ReleaseHardware;


	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
		&deviceAttributes,
		DEVICE_CONTEXT);

	status = WdfDeviceCreate(&DeviceInit,
		&deviceAttributes,
		&device);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Error: WdfDeviceCreate failed 0x%x\n", status));
		return status;
	}

	deviceContext = GetDeviceContext(device);
	deviceContext->Device = device;
	deviceContext->DeviceData = 0;

	hidAttributes = &deviceContext->HidDeviceAttributes;						//위모트에서는 큐에서 HID컨텍스트를 받아오네..?
	RtlZeroMemory(hidAttributes, sizeof(HID_DEVICE_ATTRIBUTES));
	hidAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
	hidAttributes->VendorID = HIDMINI_VID;
	hidAttributes->ProductID = HIDMINI_PID;
	hidAttributes->VersionNumber = HIDMINI_VERSION;

	status = QueueCreate(device,
		&deviceContext->DefaultQueue);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = ManualQueueCreate(device,
		&deviceContext->ManualQueue);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	deviceContext->HidDescriptor = HIDDescriptor;
	deviceContext->ReportDescriptor = HIDReportDescriptor;

	return status;
}

EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL EvtIoDeviceControl;

NTSTATUS
QueueCreate(										//makes default queue
	_In_  WDFDEVICE         Device,
	_Out_ WDFQUEUE          *Queue
)
{
	NTSTATUS                status;
	WDF_IO_QUEUE_CONFIG     queueConfig;
	WDF_OBJECT_ATTRIBUTES   queueAttributes;
	WDFQUEUE                queue;
	PQUEUE_CONTEXT          queueContext;

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
		&queueConfig,
		WdfIoQueueDispatchParallel);

	queueConfig.EvtIoInternalDeviceControl = EvtIoDeviceControl;

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(			//위모트 코드에는 이 코드가 없네?
		&queueAttributes,
		QUEUE_CONTEXT);

	status = WdfIoQueueCreate(
		Device,
		&queueConfig,
		&queueAttributes,
		&queue);

	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfIoQueueCreate failed 0x%x\n", status));
		return status;
	}

	queueContext = GetQueueContext(queue);
	queueContext->Queue = queue;
	queueContext->DeviceContext = GetDeviceContext(Device);
	queueContext->OutputReport = 0;

	*Queue = queue;
	return status;
}

VOID
EvtIoDeviceControl(
	_In_  WDFQUEUE          Queue,
	_In_  WDFREQUEST        Request,
	_In_  size_t            OutputBufferLength,
	_In_  size_t            InputBufferLength,
	_In_  ULONG             IoControlCode
)
{
	NTSTATUS                status;
	BOOLEAN                 completeRequest = TRUE;
	WDFDEVICE               device = WdfIoQueueGetDevice(Queue);
	PDEVICE_CONTEXT         deviceContext = NULL;
	PQUEUE_CONTEXT          queueContext = GetQueueContext(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	deviceContext = GetDeviceContext(device);

	switch (IoControlCode)
	{

	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:   // METHOD_NEITHER
											//
											// Retrieves the device's HID descriptor.
											//
		_Analysis_assume_(deviceContext->HidDescriptor.bLength != 0);
		status = RequestCopyFromBuffer(Request,
			&deviceContext->HidDescriptor,
			deviceContext->HidDescriptor.bLength);
		break;

	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:   // METHOD_NEITHER
											//
											//Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
											//
		status = RequestCopyFromBuffer(Request,
			&queueContext->DeviceContext->HidDeviceAttributes,
			sizeof(HID_DEVICE_ATTRIBUTES));
		break;

	case IOCTL_HID_GET_REPORT_DESCRIPTOR:   // METHOD_NEITHER
											//
											//Obtains the report descriptor for the HID device.
											//
		status = RequestCopyFromBuffer(Request,
			deviceContext->ReportDescriptor,
			deviceContext->HidDescriptor.DescriptorList[0].wReportLength);
		break;

	case IOCTL_HID_READ_REPORT:             // METHOD_NEITHER
											//
											// Returns a report from the device into a class driver-supplied
											// buffer.
											//
		status = ReadReport(queueContext, Request, &completeRequest);
		break;


	// 이 아래는 필요 없을듯
	/*
	case IOCTL_HID_WRITE_REPORT:            // METHOD_NEITHER
											//
											// Transmits a class driver-supplied report to the device.
											//
		status = WriteReport(queueContext, Request);
		break;


		//아래는 커널모드일 때만 가능한 것들.

	case IOCTL_HID_GET_FEATURE:             // METHOD_OUT_DIRECT

		status = GetFeature(queueContext, Request);
		break;

	case IOCTL_HID_SET_FEATURE:             // METHOD_IN_DIRECT

		status = SetFeature(queueContext, Request);
		break;

	case IOCTL_HID_GET_INPUT_REPORT:        // METHOD_OUT_DIRECT

		status = GetInputReport(queueContext, Request);
		break;

	case IOCTL_HID_SET_OUTPUT_REPORT:       // METHOD_IN_DIRECT

		status = SetOutputReport(queueContext, Request);
		break;

	case IOCTL_HID_GET_STRING:                      // METHOD_NEITHER

		status = GetString(Request);
		break;

	case IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT

		status = GetIndexedString(Request);
		break;

		*/

	default:
		status = STATUS_NOT_IMPLEMENTED;
		break;
	}


	//
	// Complete the request. Information value has already been set by request
	// handlers.
	//
	if (completeRequest) {
		WdfRequestComplete(Request, status);
	}
}

NTSTATUS
RequestCopyFromBuffer(
	_In_  WDFREQUEST        Request,
	_In_  PVOID             SourceBuffer,
	_When_(NumBytesToCopyFrom == 0, __drv_reportError(NumBytesToCopyFrom cannot be zero))
	_In_  size_t            NumBytesToCopyFrom
)
{
	{
		NTSTATUS                status;
		WDFMEMORY               memory;
		size_t                  outputBufferLength;

		status = WdfRequestRetrieveOutputMemory(Request, &memory);
		if (!NT_SUCCESS(status)) {
			KdPrint(("WdfRequestRetrieveOutputMemory failed 0x%x\n", status));
			return status;
		}

		WdfMemoryGetBuffer(memory, &outputBufferLength);
		if (outputBufferLength < NumBytesToCopyFrom) {
			status = STATUS_INVALID_BUFFER_SIZE;
			KdPrint(("RequestCopyFromBuffer: buffer too small. Size %d, expect %d\n",
				(int)outputBufferLength, (int)NumBytesToCopyFrom));
			return status;
		}

		status = WdfMemoryCopyFromBuffer(memory,
			0,
			SourceBuffer,
			NumBytesToCopyFrom);
		if (!NT_SUCCESS(status)) {
			KdPrint(("WdfMemoryCopyFromBuffer failed 0x%x\n", status));
			return status;
		}

		WdfRequestSetInformation(Request, NumBytesToCopyFrom);
		return status;
	}

}

NTSTATUS
ReadReport(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request,
	_Always_(_Out_)
	BOOLEAN*          CompleteRequest
)
{
	NTSTATUS                status;

	KdPrint(("ReadReport\n"));

	//
	// forward the request to manual queue
	//
	status = WdfRequestForwardToIoQueue(
		Request,
		QueueContext->DeviceContext->ManualQueue);
	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfRequestForwardToIoQueue failed with 0x%x\n", status));
		*CompleteRequest = TRUE;
	}
	else {
		*CompleteRequest = FALSE;
	}

	return status;
}

HRESULT
GetFeature(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
)
/*++

Routine Description:

Handles IOCTL_HID_GET_FEATURE for all the collection.

Arguments:

QueueContext - The object context associated with the queue

Request - Pointer to  Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	HID_XFER_PACKET         packet;
	ULONG                   reportSize;
	PMY_DEVICE_ATTRIBUTES   myAttributes;
	PHID_DEVICE_ATTRIBUTES  hidAttributes = &QueueContext->DeviceContext->HidDeviceAttributes;

	KdPrint(("GetFeature\n"));

	status = RequestGetHidXferPacket_ToReadFromDevice(
		Request,
		&packet);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	if (packet.reportId != CONTROL_COLLECTION_REPORT_ID) {
		//
		// If collection ID is not for control collection then handle
		// this request just as you would for a regular collection.
		//
		status = STATUS_INVALID_PARAMETER;
		KdPrint(("GetFeature: invalid report id %d\n", packet.reportId));
		return status;
	}

	//
	// Since output buffer is for write only (no read allowed by UMDF in output
	// buffer), any read from output buffer would be reading garbage), so don't
	// let app embed custom control code in output buffer. The minidriver can
	// support multiple features using separate report ID instead of using
	// custom control code. Since this is targeted at report ID 1, we know it
	// is a request for getting attributes.
	//
	// While KMDF does not enforce the rule (disallow read from output buffer),
	// it is good practice to not do so.
	//

	reportSize = sizeof(MY_DEVICE_ATTRIBUTES) + sizeof(packet.reportId);
	if (packet.reportBufferLen < reportSize) {
		status = STATUS_INVALID_BUFFER_SIZE;
		KdPrint(("GetFeature: output buffer too small. Size %d, expect %d\n",
			packet.reportBufferLen, reportSize));
		return status;
	}

	//
	// Since this device has one report ID, hidclass would pass on the report
	// ID in the buffer (it wouldn't if report descriptor did not have any report
	// ID). However, since UMDF allows only writes to an output buffer, we can't
	// "read" the report ID from "output" buffer. There is no need to read the
	// report ID since we get it other way as shown above, however this is
	// something to keep in mind.
	//
	myAttributes = (PMY_DEVICE_ATTRIBUTES)(packet.reportBuffer + sizeof(packet.reportId));
	myAttributes->ProductID = hidAttributes->ProductID;
	myAttributes->VendorID = hidAttributes->VendorID;
	myAttributes->VersionNumber = hidAttributes->VersionNumber;

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, reportSize);
	return status;
}

NTSTATUS
SetFeature(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
)
/*++

Routine Description:

Handles IOCTL_HID_SET_FEATURE for all the collection.
For control collection (custom defined collection) it handles
the user-defined control codes for sideband communication

Arguments:

QueueContext - The object context associated with the queue

Request - Pointer to Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	HID_XFER_PACKET         packet;
	ULONG                   reportSize;
	PHIDMINI_CONTROL_INFO   controlInfo;
	PHID_DEVICE_ATTRIBUTES  hidAttributes = &QueueContext->DeviceContext->HidDeviceAttributes;

	KdPrint(("SetFeature\n"));

	status = RequestGetHidXferPacket_ToWriteToDevice(
		Request,
		&packet);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	if (packet.reportId != CONTROL_COLLECTION_REPORT_ID) {
		//
		// If collection ID is not for control collection then handle
		// this request just as you would for a regular collection.
		//
		status = STATUS_INVALID_PARAMETER;
		KdPrint(("SetFeature: invalid report id %d\n", packet.reportId));
		return status;
	}

	//
	// before touching control code make sure buffer is big enough.
	//
	reportSize = sizeof(HIDMINI_CONTROL_INFO);

	if (packet.reportBufferLen < reportSize) {
		status = STATUS_INVALID_BUFFER_SIZE;
		KdPrint(("SetFeature: invalid input buffer. size %d, expect %d\n",
			packet.reportBufferLen, reportSize));
		return status;
	}

	controlInfo = (PHIDMINI_CONTROL_INFO)packet.reportBuffer;

	switch (controlInfo->ControlCode)
	{
	case HIDMINI_CONTROL_CODE_SET_ATTRIBUTES:
		//
		// Store the device attributes in device extension
		//
		hidAttributes->ProductID = controlInfo->u.Attributes.ProductID;
		hidAttributes->VendorID = controlInfo->u.Attributes.VendorID;
		hidAttributes->VersionNumber = controlInfo->u.Attributes.VersionNumber;

		//
		// set status and information
		//
		WdfRequestSetInformation(Request, reportSize);
		break;

	case HIDMINI_CONTROL_CODE_DUMMY1:
		status = STATUS_NOT_IMPLEMENTED;
		KdPrint(("SetFeature: HIDMINI_CONTROL_CODE_DUMMY1\n"));
		break;

	case HIDMINI_CONTROL_CODE_DUMMY2:
		status = STATUS_NOT_IMPLEMENTED;
		KdPrint(("SetFeature: HIDMINI_CONTROL_CODE_DUMMY2\n"));
		break;

	default:
		status = STATUS_NOT_IMPLEMENTED;
		KdPrint(("SetFeature: Unknown control Code 0x%x\n",
			controlInfo->ControlCode));
		break;
	}

	return status;
}

NTSTATUS
GetInputReport(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
)
/*++

Routine Description:

Handles IOCTL_HID_GET_INPUT_REPORT for all the collection.

Arguments:

QueueContext - The object context associated with the queue

Request - Pointer to Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	HID_XFER_PACKET         packet;
	ULONG                   reportSize;
	PHIDMINI_INPUT_REPORT   reportBuffer;

	KdPrint(("GetInputReport\n"));

	status = RequestGetHidXferPacket_ToReadFromDevice(
		Request,
		&packet);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	if (packet.reportId != CONTROL_COLLECTION_REPORT_ID) {
		//
		// If collection ID is not for control collection then handle
		// this request just as you would for a regular collection.
		//
		status = STATUS_INVALID_PARAMETER;
		KdPrint(("GetInputReport: invalid report id %d\n", packet.reportId));
		return status;
	}

	reportSize = sizeof(HIDMINI_INPUT_REPORT);
	if (packet.reportBufferLen < reportSize) {
		status = STATUS_INVALID_BUFFER_SIZE;
		KdPrint(("GetInputReport: output buffer too small. Size %d, expect %d\n",
			packet.reportBufferLen, reportSize));
		return status;
	}

	reportBuffer = (PHIDMINI_INPUT_REPORT)(packet.reportBuffer);

	reportBuffer->ReportId = CONTROL_COLLECTION_REPORT_ID;
	reportBuffer->Data = QueueContext->OutputReport;

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, reportSize);
	return status;
}


NTSTATUS
SetOutputReport(
	_In_  PQUEUE_CONTEXT    QueueContext,
	_In_  WDFREQUEST        Request
)
/*++

Routine Description:

Handles IOCTL_HID_SET_OUTPUT_REPORT for all the collection.

Arguments:

QueueContext - The object context associated with the queue

Request - Pointer to Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	HID_XFER_PACKET         packet;
	ULONG                   reportSize;
	PHIDMINI_OUTPUT_REPORT  reportBuffer;

	KdPrint(("SetOutputReport\n"));

	status = RequestGetHidXferPacket_ToWriteToDevice(
		Request,
		&packet);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	if (packet.reportId != CONTROL_COLLECTION_REPORT_ID) {
		//
		// If collection ID is not for control collection then handle
		// this request just as you would for a regular collection.
		//
		status = STATUS_INVALID_PARAMETER;
		KdPrint(("SetOutputReport: unkown report id %d\n", packet.reportId));
		return status;
	}

	//
	// before touching buffer make sure buffer is big enough.
	//
	reportSize = sizeof(HIDMINI_OUTPUT_REPORT);

	if (packet.reportBufferLen < reportSize) {
		status = STATUS_INVALID_BUFFER_SIZE;
		KdPrint(("SetOutputReport: invalid input buffer. size %d, expect %d\n",
			packet.reportBufferLen, reportSize));
		return status;
	}

	reportBuffer = (PHIDMINI_OUTPUT_REPORT)packet.reportBuffer;

	QueueContext->OutputReport = reportBuffer->Data;

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, reportSize);
	return status;
}


NTSTATUS
GetStringId(
	_In_  WDFREQUEST        Request,
	_Out_ ULONG            *StringId,
	_Out_ ULONG            *LanguageId
)
/*++

Routine Description:

Helper routine to decode IOCTL_HID_GET_INDEXED_STRING and IOCTL_HID_GET_STRING.

Arguments:

Request - Pointer to Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	ULONG                   inputValue;

	WDF_REQUEST_PARAMETERS  requestParameters;

	//
	// IOCTL_HID_GET_STRING:                      // METHOD_NEITHER
	// IOCTL_HID_GET_INDEXED_STRING:              // METHOD_OUT_DIRECT
	//
	// The string id (or string index) is passed in Parameters.DeviceIoControl.
	// Type3InputBuffer. However, Parameters.DeviceIoControl.InputBufferLength
	// was not initialized by hidclass.sys, therefore trying to access the
	// buffer with WdfRequestRetrieveInputMemory will fail
	//
	// Another problem with IOCTL_HID_GET_INDEXED_STRING is that METHOD_OUT_DIRECT
	// expects the input buffer to be Irp->AssociatedIrp.SystemBuffer instead of
	// Type3InputBuffer. That will also fail WdfRequestRetrieveInputMemory.
	//
	// The solution to the above two problems is to get Type3InputBuffer directly
	//
	// Also note that instead of the buffer's content, it is the buffer address
	// that was used to store the string id (or index)
	//

	WDF_REQUEST_PARAMETERS_INIT(&requestParameters);
	WdfRequestGetParameters(Request, &requestParameters);

	inputValue = PtrToUlong(
		requestParameters.Parameters.DeviceIoControl.Type3InputBuffer);

	status = STATUS_SUCCESS;

	//
	// The least significant two bytes of the INT value contain the string id.
	//
	*StringId = (inputValue & 0x0ffff);

	//
	// The most significant two bytes of the INT value contain the language
	// ID (for example, a value of 1033 indicates English).
	//
	*LanguageId = (inputValue >> 16);

	return status;
}


NTSTATUS
GetIndexedString(
	_In_  WDFREQUEST        Request
)
/*++

Routine Description:

Handles IOCTL_HID_GET_INDEXED_STRING

Arguments:

Request - Pointer to Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	ULONG                   languageId, stringIndex;

	status = GetStringId(Request, &stringIndex, &languageId);

	// While we don't use the language id, some minidrivers might.
	//
	UNREFERENCED_PARAMETER(languageId);

	if (NT_SUCCESS(status)) {

		if (stringIndex != VHIDMINI_DEVICE_STRING_INDEX)
		{
			status = STATUS_INVALID_PARAMETER;
			KdPrint(("GetString: unkown string index %d\n", stringIndex));
			return status;
		}

		status = RequestCopyFromBuffer(Request, VHIDMINI_DEVICE_STRING, sizeof(VHIDMINI_DEVICE_STRING));
	}
	return status;
}


NTSTATUS
GetString(
	_In_  WDFREQUEST        Request
)
/*++

Routine Description:

Handles IOCTL_HID_GET_STRING.

Arguments:

Request - Pointer to Request Packet.

Return Value:

NT status code.

--*/
{
	NTSTATUS                status;
	ULONG                   languageId, stringId;
	size_t                  stringSizeCb;
	PWSTR                   string;

	status = GetStringId(Request, &stringId, &languageId);

	// While we don't use the language id, some minidrivers might.
	//
	UNREFERENCED_PARAMETER(languageId);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	switch (stringId) {
	case HID_STRING_ID_IMANUFACTURER:
		stringSizeCb = sizeof(VHIDMINI_MANUFACTURER_STRING);
		string = VHIDMINI_MANUFACTURER_STRING;
		break;
	case HID_STRING_ID_IPRODUCT:
		stringSizeCb = sizeof(VHIDMINI_PRODUCT_STRING);
		string = VHIDMINI_PRODUCT_STRING;
		break;
	case HID_STRING_ID_ISERIALNUMBER:
		stringSizeCb = sizeof(VHIDMINI_SERIAL_NUMBER_STRING);
		string = VHIDMINI_SERIAL_NUMBER_STRING;
		break;
	default:
		status = STATUS_INVALID_PARAMETER;
		KdPrint(("GetString: unkown string id %d\n", stringId));
		return status;
	}

	status = RequestCopyFromBuffer(Request, string, stringSizeCb);
	return status;
}



NTSTATUS
ManualQueueCreate(
	_In_  WDFDEVICE         Device,
	_Out_ WDFQUEUE          *Queue
)
/*++
Routine Description:

This function creates a manual I/O queue to receive IOCTL_HID_READ_REPORT
forwarded from the device's default queue handler.

It also creates a periodic timer to check the queue and complete any pending
request with data from the device. Here timer expiring is used to simulate
a hardware event that new data is ready.

The workflow is like this:

- Hidclass.sys sends an ioctl to the miniport to read input report.

- The request reaches the driver's default queue. As data may not be avaiable
yet, the request is forwarded to a second manual queue temporarily.

- Later when data is ready (as simulated by timer expiring), the driver
checks for any pending request in the manual queue, and then completes it.

- Hidclass gets notified for the read request completion and return data to
the caller.

On the other hand, for IOCTL_HID_WRITE_REPORT request, the driver simply
sends the request to the hardware (as simulated by storing the data at
DeviceContext->DeviceData) and completes the request immediately. There is
no need to use another queue for write operation.

Arguments:

Device - Handle to a framework device object.

Queue - Output pointer to a framework I/O queue handle, on success.

Return Value:

NTSTATUS

--*/
{
	NTSTATUS                status;
	WDF_IO_QUEUE_CONFIG     queueConfig;
	WDF_OBJECT_ATTRIBUTES   queueAttributes;
	WDFQUEUE                queue;
	PMANUAL_QUEUE_CONTEXT   queueContext;
	WDF_TIMER_CONFIG        timerConfig;
	WDF_OBJECT_ATTRIBUTES   timerAttributes;
	ULONG                   timerPeriodInSeconds = 5;

	WDF_IO_QUEUE_CONFIG_INIT(
		&queueConfig,
		WdfIoQueueDispatchManual);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
		&queueAttributes,
		MANUAL_QUEUE_CONTEXT);

	status = WdfIoQueueCreate(
		Device,
		&queueConfig,
		&queueAttributes,
		&queue);

	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfIoQueueCreate failed 0x%x\n", status));
		return status;
	}

	queueContext = GetManualQueueContext(queue);
	queueContext->Queue = queue;
	queueContext->DeviceContext = GetDeviceContext(Device);

	WDF_TIMER_CONFIG_INIT_PERIODIC(
		&timerConfig,
		EvtTimerFunc,
		timerPeriodInSeconds * 1000);

	WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
	timerAttributes.ParentObject = queue;
	status = WdfTimerCreate(&timerConfig,
		&timerAttributes,
		&queueContext->Timer);

	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfTimerCreate failed 0x%x\n", status));
		return status;
	}

	WdfTimerStart(queueContext->Timer, WDF_REL_TIMEOUT_IN_SEC(1));

	*Queue = queue;

	return status;
}

void
EvtTimerFunc(
	_In_  WDFTIMER          Timer
)
/*++
Routine Description:

This periodic timer callback routine checks the device's manual queue and
completes any pending request with data from the device.

Arguments:

Timer - Handle to a timer object that was obtained from WdfTimerCreate.

Return Value:

VOID

--*/
{
	NTSTATUS                status;
	WDFQUEUE                queue;
	PMANUAL_QUEUE_CONTEXT   queueContext;
	WDFREQUEST              request;
	HIDMINI_INPUT_REPORT    readReport;

	KdPrint(("EvtTimerFunc\n"));

	queue = (WDFQUEUE)WdfTimerGetParentObject(Timer);
	queueContext = GetManualQueueContext(queue);

	//
	// see if we have a request in manual queue
	//
	status = WdfIoQueueRetrieveNextRequest(
		queueContext->Queue,
		&request);

	if (NT_SUCCESS(status)) {

		readReport.ReportId = CONTROL_FEATURE_REPORT_ID;
		readReport.Data = queueContext->DeviceContext->DeviceData;

		status = RequestCopyFromBuffer(request,
			&readReport,
			sizeof(readReport));

		WdfRequestComplete(request, status);
	}
}





//
// First let's review Buffer Descriptions for I/O Control Codes
//
//   METHOD_BUFFERED
//    - Input buffer:  Irp->AssociatedIrp.SystemBuffer
//    - Output buffer: Irp->AssociatedIrp.SystemBuffer
//
//   METHOD_IN_DIRECT or METHOD_OUT_DIRECT
//    - Input buffer:  Irp->AssociatedIrp.SystemBuffer
//    - Second buffer: Irp->MdlAddress
//
//   METHOD_NEITHER
//    - Input buffer:  Parameters.DeviceIoControl.Type3InputBuffer;
//    - Output buffer: Irp->UserBuffer
//
// HID minidriver IOCTL stores a pointer to HID_XFER_PACKET in Irp->UserBuffer.
// For IOCTLs like IOCTL_HID_GET_FEATURE (which is METHOD_OUT_DIRECT) this is
// not the expected buffer location. So we cannot retrieve UserBuffer from the
// IRP using WdfRequestXxx functions. Instead, we have to escape to WDM.
//


NTSTATUS
RequestGetHidXferPacket_ToReadFromDevice(
	_In_  WDFREQUEST        Request,
	_Out_ HID_XFER_PACKET  *Packet
)
{
	NTSTATUS                status;
	WDF_REQUEST_PARAMETERS  params;

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request, &params);

	if (params.Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_XFER_PACKET)) {
		status = STATUS_BUFFER_TOO_SMALL;
		KdPrint(("RequestGetHidXferPacket: invalid HID_XFER_PACKET\n"));
		return status;
	}

	RtlCopyMemory(Packet, WdfRequestWdmGetIrp(Request)->UserBuffer, sizeof(HID_XFER_PACKET));
	return STATUS_SUCCESS;
}

NTSTATUS
RequestGetHidXferPacket_ToWriteToDevice(
	_In_  WDFREQUEST        Request,
	_Out_ HID_XFER_PACKET  *Packet
)
{
	NTSTATUS                status;
	WDF_REQUEST_PARAMETERS  params;

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request, &params);

	if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET)) {
		status = STATUS_BUFFER_TOO_SMALL;
		KdPrint(("RequestGetHidXferPacket: invalid HID_XFER_PACKET\n"));
		return status;
	}

	RtlCopyMemory(Packet, WdfRequestWdmGetIrp(Request)->UserBuffer, sizeof(HID_XFER_PACKET));
	return STATUS_SUCCESS;
}
