#pragma once
#include <ppltasks.h>
#include <memory>
#include <mutex>


namespace cv {
    class Mat;
}
namespace dxgiutil {
    class DeferredDeviceContext;
    class DeviceManager;
    class Texture;
}
class Detector;
ref class ReaderCallback;

namespace aruco_uwp {
    class CaptureRenderer
    {
        Platform::Agile<Windows::Media::Capture::MediaCapture^> m_capture;
        Platform::Agile<Windows::Media::Capture::Frames::MediaFrameReader^> m_reader;
        Platform::Agile<ReaderCallback^> m_callback;

        int m_width=0;
        int m_height=0;
        std::shared_ptr<Detector> m_detector;
        std::shared_ptr<dxgiutil::DeferredDeviceContext> m_deferredRenderer;
        std::shared_ptr<dxgiutil::DeferredDeviceContext> m_deferredCapture;
        std::shared_ptr<dxgiutil::DeferredDeviceContext> m_deferredMarker;
        std::shared_ptr<dxgiutil::DeviceManager> m_deviceManager;
        std::shared_ptr<dxgiutil::Texture> m_captureTexture;
        std::shared_ptr<dxgiutil::Texture> m_renderTexture;
        bool m_loading = false;
        bool m_working = false;
        std::mutex m_workMutex;
    public:

        void InitializeCaptureAsync(const std::shared_ptr<dxgiutil::DeviceManager> &deviceManager
            , const std::shared_ptr<dxgiutil::DeferredDeviceContext> &deferredRenderer);

        void Resize(int w, int h)
        {
            m_width = w;
            m_height = h;
        }

    private:
        concurrency::task<void> StartCaptureAsync();

        void OnFrame(const cv::Mat &input);
    };
}
