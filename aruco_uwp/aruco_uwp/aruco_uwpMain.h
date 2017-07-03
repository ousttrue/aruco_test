﻿#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

// Direct2D および 3D コンテンツを画面上でレンダリングします。
namespace aruco_uwp
{
	class aruco_uwpMain : public DX::IDeviceNotify
	{
	public:
		aruco_uwpMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~aruco_uwpMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// デバイス リソースへのキャッシュされたポインター。
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: これを独自のコンテンツ レンダラーで置き換えます。
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// ループ タイマーをレンダリングしています。
		DX::StepTimer m_timer;
	};
}