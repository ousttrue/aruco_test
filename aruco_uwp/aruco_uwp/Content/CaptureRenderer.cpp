#include "pch.h"
#include "CaptureRenderer.h"
#include <algorithm>
#include <collection.h>


namespace aruco_uwp
{
    static concurrency::task<Windows::Devices::Enumeration::DeviceInformation^> GetVideoDeviceAsync()
    {
        return
            concurrency::create_task(Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
                Windows::Devices::Enumeration::DeviceClass::VideoCapture))
            .then([](Windows::Devices::Enumeration::DeviceInformationCollection^ devices)
                ->Windows::Devices::Enumeration::DeviceInformation^ {

            for (size_t i = 0; i < devices->Size; ++i)
            {
                auto device = devices->GetAt(i);
                std::wstring name(device->Name->Data());
                if (name == L"HTC Vive") {
                    continue;
                }
                return device;
            }

            return nullptr;
        });
    }


    static concurrency::task<Platform::Agile<Windows::Media::Capture::MediaCapture^>> CreateMediaCaptureAsync(Windows::Devices::Enumeration::DeviceInformation^ device)
    {
        Platform::Agile<Windows::Media::Capture::MediaCapture^> mediaCapture(ref new Windows::Media::Capture::MediaCapture());

        auto settings = ref new Windows::Media::Capture::MediaCaptureInitializationSettings();

        // Select the source we will be reading from.
        settings->VideoDeviceId = device->Id;

        // This media capture can share streaming with other apps.
        settings->SharingMode = Windows::Media::Capture::MediaCaptureSharingMode::SharedReadOnly;

        // Only stream video and don't initialize audio capture devices.
        settings->StreamingCaptureMode = Windows::Media::Capture::StreamingCaptureMode::Video;

        // Set to CPU to ensure frames always contain CPU SoftwareBitmap images,
        // instead of preferring GPU D3DSurface images.
        settings->MemoryPreference = Windows::Media::Capture::MediaCaptureMemoryPreference::Cpu;

        return
            concurrency::create_task(mediaCapture->InitializeAsync(settings))
            .then([mediaCapture](concurrency::task<void> initializeMediaCaptureTask) {

            try {
                // Get the result of the initialization. This call will throw if initialization failed
                // This pattern is docuemnted at https://msdn.microsoft.com/en-us/library/dd997692.aspx
                initializeMediaCaptureTask.get();
                return mediaCapture;
            }
            catch (Platform::Exception^ exception)
            {
                return Platform::Agile<Windows::Media::Capture::MediaCapture^>();
            }
        });
    }


    static Windows::Media::Capture::Frames::MediaFrameSourceInfo^ GetFirstSourceInfoOfKind(
        Windows::Media::Capture::Frames::MediaFrameSourceGroup^ group
        , Windows::Media::Capture::Frames::MediaFrameSourceKind kind)
    {
        auto matchingInfo = std::find_if(begin(group->SourceInfos), end(group->SourceInfos),
            [kind](Windows::Media::Capture::Frames::MediaFrameSourceInfo^ sourceInfo)
        {
            return sourceInfo->SourceKind == kind;
        });
        return (matchingInfo != end(group->SourceInfos)) ? *matchingInfo : nullptr;
    }


    void CaptureRenderer::InitializeCaptureAsync()
    {
        GetVideoDeviceAsync()
            .then([this](Windows::Devices::Enumeration::DeviceInformation^ device) {

            CreateMediaCaptureAsync(device)
                .then([this](Platform::Agile<Windows::Media::Capture::MediaCapture^> capture) {

                m_capture = capture;
                StartCaptureAsync();

            });

        });
        ;
    }

    concurrency::task<void> CaptureRenderer::StartCaptureAsync()
    {
        if (!m_capture.Get()) {
            return concurrency::task_from_result();
        }

        return
            concurrency::create_task(Windows::Media::Capture::Frames::MediaFrameSourceGroup::FindAllAsync())
            .then([](Windows::Foundation::Collections::IVectorView<Windows::Media::Capture::Frames::MediaFrameSourceGroup^>^ allGroups)
                ->Windows::Media::Capture::Frames::MediaFrameSourceInfo^
        {
            for (auto group : allGroups)
            {
                auto sourceInfo = GetFirstSourceInfoOfKind(group, Windows::Media::Capture::Frames::MediaFrameSourceKind::Color);
                if (sourceInfo) {
                    return sourceInfo;
                }
            }
            return nullptr;
        })
            .then([this](Windows::Media::Capture::Frames::MediaFrameSourceInfo^ info)
        {
            if (info) {
                concurrency::create_task(m_capture->CreateFrameReaderAsync(m_capture->FrameSources->Lookup(info->Id)))
                    .then([this](Windows::Media::Capture::Frames::MediaFrameReader^ frameReader) {

                    frameReader->FrameArrived += ref new Windows::Foundation::TypedEventHandler<
                        Windows::Media::Capture::Frames::MediaFrameReader^
                        , Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs^>(this, &CaptureRenderer::FrameReader_FrameArrived);

                    frameReader->StartAsync();

                });
            }
        })
            ;
    }

    void CaptureRenderer::FrameReader_FrameArrived(Windows::Media::Capture::Frames::MediaFrameReader^ sender
        , Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs^ args)
    {
        if (auto frame = sender->TryAcquireLatestFrame())
        {
            if (frame)
            {
                auto input = frame->VideoMediaFrame;
                if (input) {
                    auto inputBitmap = input->SoftwareBitmap;
                    auto format = inputBitmap->BitmapPixelFormat;

                    try
                    {
                        // Convert to Bgra8 Premultiplied SoftwareBitmap, so xaml can display in UI.
                        auto converted = Windows::Graphics::Imaging::SoftwareBitmap::Convert(inputBitmap
                            , Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8
                            , Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied
                        );

                        auto newFormat = converted->BitmapPixelFormat;
                        int a = 0;
                    }
                    catch (Platform::InvalidArgumentException^ exception)
                    {
                        // Conversion of software bitmap format is not supported.  Drop this frame.
                        OutputDebugStringW(exception->Message->Data());
                        OutputDebugStringW(L"\r\n");
                    }
                }
            }
        }
    }
}
