#pragma once
#include <ppltasks.h>


namespace aruco_uwp {
    ref class CaptureRenderer
    {
        Platform::Agile<Windows::Media::Capture::MediaCapture^> m_capture;

    public:
        void InitializeCaptureAsync();

    private:
        concurrency::task<void> StartCaptureAsync();

        void FrameReader_FrameArrived(Windows::Media::Capture::Frames::MediaFrameReader^ sender
            , Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs^ args);

    };
}
