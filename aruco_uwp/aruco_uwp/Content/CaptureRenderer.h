#pragma once
#include <ppltasks.h>
#include <memory>
#include <mutex>
#include "../Common/DeviceResources.h"
#include "../Common/StepTimer.h"

namespace cv {
    class Mat;
}
namespace dxgiutil {
    class DeferredDeviceContext;
    class DeviceManager;
    class Texture;
    class Shader;
}
class Detector;
ref class ReaderCallback;

namespace aruco_uwp {
    class CaptureRenderer
    {
        int m_width = 0;
        int m_height = 0;

        std::shared_ptr<DX::DeviceResources> m_deviceResources;
        std::shared_ptr<dxgiutil::DeviceManager> m_deviceManager;
        std::shared_ptr<dxgiutil::DeferredDeviceContext> m_deferredRenderer;
        std::shared_ptr<dxgiutil::DeferredDeviceContext> m_deferredCapture;
        std::shared_ptr<dxgiutil::DeferredDeviceContext> m_deferredMarker;
        std::shared_ptr<dxgiutil::Texture> m_captureTexture;
        std::shared_ptr<dxgiutil::Texture> m_renderTexture;
        std::vector<std::shared_ptr<dxgiutil::Shader>> m_markerShaders;
        bool m_loadingComplete = false; // create dx loaded

        Platform::Agile<Windows::Media::Capture::MediaCapture^> m_capture;
        Platform::Agile<Windows::Media::Capture::Frames::MediaFrameReader^> m_reader;
        Platform::Agile<ReaderCallback^> m_callback;
        std::shared_ptr<Detector> m_detector;
        bool m_captureTaskWorking = false; // capture, detect & update texture
        std::mutex m_captureMutex;

    public:
        CaptureRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void Render();

    private:
        concurrency::task<void> StartCaptureAsync();

        void OnFrame(const cv::Mat &input);
    };
}
