#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include"../DCSengine/DCSengine.h"
#include <Mmsystem.h>
#include <mciapi.h>
#include <functional>

namespace DCS
{
	std::wstring enumToString(Room::RoomType t);
	std::wstring enumToString(DamageState t);

	class Dx11Engine :public Game
	{

		unsigned int oddFrame = 0;

		template<typename T> class Button
		{
		protected:
			T type;
			T nullValue;
			Point size;
			Point position;
			std::wstring text;
			float textColor[4];
			float color[4];
			void basicRender(const float c[4], ID2D1DeviceContext * context, Point offset) const;
		public:
			Button(T type,T nullValue, std::wstring name, Point size, Point position, float textColor[4],float color[4]);
			virtual void render(ID2D1DeviceContext * context, Point offset);
			T virtual OnPointerPressed(Point position);
			T whatResult() const;
		};
		
		template <typename T> class ActiveButton: public Button<T>
		{
			bool isActive;
			float activeColor[4];
			std::unique_ptr<std::function<bool(bool)>> condition = std::unique_ptr<std::function<bool(bool)>>(new std::function<bool(bool)>([](bool b) { return b; }));
			std::unique_ptr<std::function<bool(bool)>> onPress = std::unique_ptr<std::function<bool(bool)>>(new std::function<bool(bool)>([](bool b) { return !b; }));
		public:
			ActiveButton(T type, T nullValue, std::wstring name, Point size, Point position, float textColor[4], float color[4], float activeColor[4]);
			ActiveButton(T type, T nullValue, std::wstring name, Point size, Point position, float textColor[4], float color[4], float activeColor[4], std::unique_ptr<std::function<bool(bool)>>& condition,std::unique_ptr<std::function<bool(bool)>>& onPress);
			virtual void render(ID2D1DeviceContext * context, Point offset);
			T virtual OnPointerPressed(Point position);
		};

		class SmartWString
		{
		public:
			typedef std::unique_ptr<std::function<std::wstring()>> StorageType;
		private:
			std::vector<StorageType>* argVector;
			std::wstring format;
		public:
			std::wstring operator*();
			SmartWString(std::wstring whatFormat, std::vector<StorageType> *v);
			~SmartWString();
		};

		template<typename ButtonResult, typename HoverResult> class HoverSection
		{
			Point position;
			Point size;
			ButtonResult nullResult;
			HoverResult result;
			HoverResult nullHover;
			std::vector<std::pair<SmartWString, Point>>texts;
			std::vector < Button<ButtonResult>*>buttons;
			float backColor[4];
			float textColor[4];
			float activeBackColor[4];
		public:
			HoverResult render(DCS::Point offset, ID2D1DeviceContext * context, Point);
			ButtonResult pointerPress(Point);
			HoverSection(Point size, Point relativePosition, std::vector<std::pair<SmartWString, Point>>textVector, std::vector < Button<ButtonResult>*>buttons, float backColor[4], float textColor[4], float activeBackColor[4], ButtonResult nullResult, HoverResult result, HoverResult nullHover);
			ButtonResult whatResult(int) const;
			HoverResult whatResult() const;
			Point location();
		};

		class DraggableWindow
		{
			bool isDragged;
			D2D1::ColorF::Enum color = D2D1::ColorF::White;
			std::wstring name;
			Button<bool>close = Button<bool>(true, false, std::wstring(L"X"), Point(20, 20), Point(size.second - 20, 0), []() { float a[4] = { 0,0,0,1 }; return a; } ( ), []() { float a[4] = { 1,1,1,1 }; return a; } ( ));
		protected:
			bool isVisible;
			Point dragArea;
			Point size;
			Point position;
			Point grabPoint;
			virtual void privateRender(ID2D1DeviceContext * context)=0;
			virtual  void press(DCS::Point)=0;
			DraggableWindow(D2D1::ColorF::Enum, Point size, Point position, std::wstring name);
		public:
			void render(DCS::Point offset, ID2D1DeviceContext * context);
			void pointerPress(Point);
			void pointerRelease();
		};

		class FireManager : public DraggableWindow
		{
			int oddFrame = 0;
			int remainCount = 0;
			virtual void press(Point);
			virtual void privateRender(ID2D1DeviceContext * context);
			std::vector<HoverSection<Room*,Room*>>fires;
		public:
			FireManager(Point position);
			void add(  Room *  r);
			void remove(const Room * const r);
		}fManager = FireManager(Point(500, 100));

		class EscMenu
		{
			float color[4] = { .0f, .0f, .0f ,1 };
			int sizeX;
			int sizeY;
			enum EscMenuButton {Resume, Save,Exit,Settings,Null};
			std::vector<Button<EscMenuButton>> buttons;
			Point position;
		public:
			enum PressResult {None, ReturnToMain,ExitMenu};
			EscMenu(Point p);
			PressResult OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
			void render(ID2D1DeviceContext * context) const;
		}escMenu = EscMenu(Point(600, 300));

		class ObjectiveScreen : public DraggableWindow
		{
			Scenario*objective;
			float textColor[4] = { .0f, .0f, .0f, 1 };
			float color[4] = { 1.f, 1.f, 1.0f ,1 };
			virtual void privateRender(ID2D1DeviceContext * context);
			virtual void press(Point);
		public:
			ObjectiveScreen(Scenario* obj, Point position);
			void changeScenario(Scenario*obj);
		}objectives = ObjectiveScreen(currentScenario,Point(400,400));

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

		class BreachScreen : public DraggableWindow
		{
			int remainCount;
			int oddFrame;
			std::vector<Room*>rooms;
			virtual void privateRender(ID2D1DeviceContext * context);
			virtual  void press(DCS::Point);
		public:
			BreachScreen(Point p);
			void addBreach(Room*);
		} breaches = BreachScreen(Point(600, 600));

		class RoomManager : public DraggableWindow
		{
			std::vector<HoverSection<Room*, Room*>>rooms;
			virtual void privateRender(ID2D1DeviceContext * context);
			virtual  void press(DCS::Point);
		public:
			RoomManager(std::vector<Room*>&r, Point position);
		}rManager = RoomManager(std::vector<Room*>(), Point(100, 200));

		class WindowManager
		{
			std::vector<DraggableWindow*>windows;
			float color[4];
		public:
			void render(ID2D1DeviceContext*context);
			void onPress(Point p);
			void addWindow(DraggableWindow*);
			WindowManager(float color[4]);
			~WindowManager();
		};

	public:
		void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnButtonPress(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args);
		static Point shipPosition;
		void gameRender(ID2D1DeviceContext * context);
		void renderRoom(ID2D1DeviceContext * context, const Room&room);
		void renderMobileEntity(ID2D1DeviceContext * context, MobileEntity*entity);
		void renderDoor(ID2D1DeviceContext * context, Door&toRender);
		void renderBreach(ID2D1DeviceContext * context, const HullBreach&toRender);
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