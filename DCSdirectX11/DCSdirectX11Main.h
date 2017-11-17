#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include"../DCSengine/DCSengine.h"
#include <Mmsystem.h>
#include <mciapi.h>

namespace DCS
{
	wchar_t* enumToString(Room::RoomType t);
	wchar_t* enumToString(DamageState t);

	class Dx11Engine :public Game
	{
		unsigned int oddFrame = 0;
		class FireManager
		{
			int oddFrame = 0;
			int remainCount = 0;
			std::vector<Room*>fires;
			Point position;
		public:
			void render(ID2D1DeviceContext * context);
			FireManager(Point position);
			void add(Room* r);
			void remove(Room*r);
		}fManager = FireManager(Point(500, 150));

		class EscMenu
		{
			float color[4] = { .18, .208, .38 ,1 };
			int sizeX;
			int sizeY;
			enum Buttons {Resume,ReturnToMain,Settings};
			class Button
			{
				Buttons type;
			public:
				Button(Point,Buttons);
				~Button();
				void render(ID2D1DeviceContext * context) const;
			private:
				static wchar_t*enumToChar(Buttons);
				Point position;
				int sizeX;
				int sizeY;
			};
			Point position;
		public:
			enum PressResult {None, ReturnToMain,Resume};
			EscMenu(Point p);
			PressResult OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
			void render(ID2D1DeviceContext * context) const;
		}escMenu = EscMenu(Point(100, 100));

	public:
		void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnButtonPress(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args);
		Point shipPosition = Point(100, 100);
		void gameRender(ID2D1DeviceContext * context);
		void renderRoom(ID2D1DeviceContext * context, Room&room);
		void renderMobileEntity(ID2D1DeviceContext * context, MobileEntity*entity);
	};
}

// Renders Direct2D and 3D content on the screen.
namespace DCSdirectX11
{
	class DCSdirectX11Main : public DX::IDeviceNotify
	{
	public:
		DCSdirectX11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~DCSdirectX11Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();
		DCS::Dx11Engine game;
		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();
		

	private:

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}