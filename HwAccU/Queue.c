/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "queue.tmh"
#include <acpiioct.h>
//#include <stdlib.h>

NTSTATUS
HwAccUQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:

     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = HwAccUEvtIoDeviceControl;
    queueConfig.EvtIoStop = HwAccUEvtIoStop;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if(!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }

    return status;
}

VOID
HwAccUEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{

    NTSTATUS            Status = STATUS_SUCCESS;// Assume success
    HANDLE              hDriver = NULL;
    // L"\\\\?\\GLOBALROOT\\Device\\0000001b" also 1c

    hDriver = CreateFile(
        L"\\\\?\\GLOBALROOT\\Device\\0000001b", 
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hDriver != INVALID_HANDLE_VALUE) {

        switch (IoControlCode) {
        case IOCTL_GPD_OPEN_ACPI:
        {
            BOOL                     IoctlResult;
            DWORD                       ReturnedLength, LastError;

            ACPI_EVAL_INPUT_BUFFER      pMethodWithoutInputEx = {0};
            ACPI_EVAL_OUTPUT_BUFFER     outbuf;

            //pMethodWithoutInputEx = (PACPI_EVAL_INPUT_BUFFER)  malloc (sizeof (ACPI_EVAL_INPUT_BUFFER));
            //(ACPI_EVAL_INPUT_BUFFER_EX*)ExAllocatePoolWithTag(0, sizeof(ACPI_EVAL_INPUT_BUFFER_EX), MY_TAG);

            pMethodWithoutInputEx.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE_EX;
            pMethodWithoutInputEx.MethodName[0] = 'W';
            pMethodWithoutInputEx.MethodName[1] = 'M';
            pMethodWithoutInputEx.MethodName[2] = 'A';
            pMethodWithoutInputEx.MethodName[3] = 'X';
            pMethodWithoutInputEx.MethodName[4] = 0;

            IoctlResult = DeviceIoControl(
                hDriver,           // Handle to device
                IOCTL_ACPI_EVAL_METHOD,    // IO Control code for Read
                &pMethodWithoutInputEx,        // Buffer to driver.
                sizeof(ACPI_EVAL_INPUT_BUFFER), // Length of buffer in bytes.
                &outbuf,     // Buffer from driver.
                sizeof(ACPI_EVAL_OUTPUT_BUFFER),
                &ReturnedLength,    // Bytes placed in DataBuffer.
                NULL                // NULL means wait till op. completes.
            );
            if (!IoctlResult)
                LastError = GetLastError();
            //    //pDevObj = IoGetLowerDeviceObject (DeviceObject);
            //    Status = OpenAcpiDevice(Request, InputBufferLength, OutputBufferLength);
        } break;
            //case IOCTL_GPD_READ_ACPI_MEMORY:
            //    Status = ReadAcpiMemory(pLDI, pIrp, pIrpStack);
            //    break;
            //case IOCTL_GPD_EVAL_ACPI_WITHOUT_PARAMETER:
            //    Status = EvalAcpiWithoutInput(pLDI, pIrp, pIrpStack);
            //    break;
            //case IOCTL_GPD_EVAL_ACPI_WITH_PARAMETER:
            //    Status = EvalAcpiWithInput(pLDI, pIrp, pIrpStack);
            //    break;
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
        CloseHandle(hDriver);
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);

    WdfRequestComplete(Request, Status);

    return;
}

VOID
HwAccUEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d", 
                Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it calls WdfRequestUnmarkCancelable
    //   (if the request is cancelable) and either calls WdfRequestStopAcknowledge
    //   with a Requeue value of TRUE, or it calls WdfRequestComplete with a
    //   completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //
    //   Before it can call these methods safely, the driver must make sure that
    //   its implementation of EvtIoStop has exclusive access to the request.
    //
    //   In order to do that, the driver must synchronize access to the request
    //   to prevent other threads from manipulating the request concurrently.
    //   The synchronization method you choose will depend on your driver's design.
    //
    //   For example, if the request is held in a shared context, the EvtIoStop callback
    //   might acquire an internal driver lock, take the request from the shared context,
    //   and then release the lock. At this point, the EvtIoStop callback owns the request
    //   and can safely complete or requeue the request.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge with
    //   a Requeue value of FALSE.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time.
    //
    // In this case, the framework waits until the specified request is complete
    // before moving the device (or system) to a lower power state or removing the device.
    // Potentially, this inaction can prevent a system from entering its hibernation state
    // or another low system power state. In extreme cases, it can cause the system
    // to crash with bugcheck code 9F.
    //

    return;
}
