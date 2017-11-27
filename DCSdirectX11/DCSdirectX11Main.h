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
			float color[4] = { .0f, .0f, .0f ,1 };
			int sizeX;
			int sizeY;
			enum EscMenuButton {Resume,Exit,Settings,Null};
			class Button
			{
				EscMenuButton type;
			public:
				Button(Point,EscMenuButton);
				~Button() = default;
				void render(ID2D1DeviceContext * context, Point offset) const;
				EscMenuButton OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args, Point offset);
			private:
				float textColor[4]= { .0f, .0f, .0f, 1 };
				float color[4] = { 1.f, 1.f, 1.0f ,1 };
				static wchar_t*enumToChar(EscMenuButton);
				Point position;
				int sizeX=200;
				int sizeY=100;
			};
			std::vector<Button> buttons;
			Point position;
		public:
			enum PressResult {None, ReturnToMain,ExitMenu};
			EscMenu(Point p);
			PressResult OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
			void render(ID2D1DeviceContext * context) const;
		}escMenu = EscMenu(Point(600, 100));

		class ObjectiveScreen
		{
			Point size=Point(300, 200);
			Scenario*objective;
			float textColor[4] = { .0f, .0f, .0f, 1 };
			float color[4] = { 1.f, 1.f, 1.0f ,1 };

		public:
			void render(ID2D1DeviceContext * context, Point offset) const;
			ObjectiveScreen(Scenario* obj);
			void changeScenario(Scenario*obj);
		}objectives = ObjectiveScreen(currentScenario);

		class VictoryScreen
		{
			Point size = Point(400, 400);
			Scenario*objective;
			float textColor[4] = { .0f, .0f, .0f, 1 };
			float color[4] = { 1.f, 1.f, 1.0f ,1 };
			bool hasWon;
		public:
			void render(ID2D1DeviceContext * context, Point offset) const;
			void changeState(bool newState);
		}victoryScreen;
		
		class MainMenu
		{
		public:
			enum PressResult { Nothing, ToGame, ExitApp };

		private:
			Point position;
			float color[4] = { .5f, .5f, .5f ,1 };
			int sizeX = 200;
			int sizeY = 400;
			enum MainMenuButton { NewGame, Continue, Settings, Exit, Null };
			class MainButton
			{
				MainMenuButton type;
			public:
				MainButton(Point p, MainMenuButton b);
				~MainButton() = default;
				void render(ID2D1DeviceContext * context, Point offset) const;
				MainMenuButton OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args, Point offset);
			private:
				float textColor[4] = { .0f, .0f, .0f, 1 };
				float color[4] = { 1.0f, 1.0f, 1.0f ,1 };
				static wchar_t*enumToChar(MainMenuButton);
				Point position;
				int sizeX = 100;
				int sizeY = 20;
			};
			std::vector<MainButton> buttons;
		public:
			PressResult OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
			void render(ID2D1DeviceContext * context) const;
			MainMenu();

		}main;

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