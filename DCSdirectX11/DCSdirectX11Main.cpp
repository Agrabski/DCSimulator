#include "pch.h"
#include "DCSdirectX11Main.h"
#include "Common\DirectXHelper.h"
#include "../DCSengine/DCSengine.h"
#include "Common\DeviceResources.h"

using namespace DCSdirectX11;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;


DCS::Point DCS::Dx11Engine::shipPosition = DCS::Point(100, 100);


// Loads and initializes application assets when the application is loaded.
DCSdirectX11Main::DCSdirectX11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

DCSdirectX11Main::~DCSdirectX11Main()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DCSdirectX11Main::CreateWindowSizeDependentResources()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void DCSdirectX11Main::Update()
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
	});
	this->game.gameTick();
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DCSdirectX11Main::Render()
{
	// Don't try to render anything before the first Update.
	if ( m_timer.GetFrameCount() == 0 )
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	auto testcontext = m_deviceResources->GetD2DDeviceContext();




	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	float tmp[4] = { .18f, .208f, .38f ,1.0f };
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), tmp);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	testcontext->BeginDraw();
	game.gameRender(testcontext);
	testcontext->EndDraw();

	return true;
}

// Notifies renderers that device resources need to be released.
void DCSdirectX11Main::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DCSdirectX11Main::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}


void DCS::Dx11Engine::OnPointerPressed(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	std::vector<std::pair<Room*, DCS::DamageState>> rooms;
	Ship*vessel;
	std::vector<Objective*> tmp;
	std::vector<Event*> tmpEvent;
	std::vector<std::pair<Room*, int>>setOnFire;
	std::vector<std::pair<Room*, int>>toBoard;
	std::vector<Room*>toDepressurise;
	std::vector<std::pair<Room*, int>>toDamage;
	if ( CurrentScreen == ScreenType::MainMenu )
		switch ( main.OnPointerPressed(sender, args) )
		{
		case MainMenu::PressResult::ToGame:
			isPaused = false;
			vessel = new Ship();
			rooms.emplace_back(*vessel->roomCbegin(), Damaged);
			tmp.push_back(new Timed(rooms, vessel, 10000));
			setOnFire.push_back(std::pair<Room*, int>(rooms[0].first, 80));
			tmpEvent.push_back(new Event(new TimeTrigger(1200), setOnFire, toBoard, toDepressurise, toDamage));
			currentScenario = new Scenario(tmp, vessel, tmpEvent);
			CurrentScreen = ScreenType::InGame;
			state = Continue;
			objectives.changeScenario(currentScenario);
			fManager = FireManager(DCS::Point(300, 300));
			breaches = BreachScreen(DCS::Point(300, 500));
			rManager = RoomManager(currentScenario->ship->room(), Point(500, 500));
			break;
		case MainMenu::PressResult::ExitApp:
			Windows::ApplicationModel::Core::CoreApplication::Exit();
		default:
			break;
		}
	else
		if ( isPaused )
		{
			switch ( escMenu.OnPointerPressed(sender, args) )
			{
			case DCS::Dx11Engine::EscMenu::ExitMenu:
				isPaused = false;
				break;
			case DCS::Dx11Engine::EscMenu::None:
				break;
			case DCS::Dx11Engine::EscMenu::ReturnToMain:
				CurrentScreen = ScreenType::MainMenu;
				delete currentScenario;
			}
		}

		else
		{
			DCS::Point position = DCS::Point(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
			fManager.pointerPress(position);
			objectives.pointerPress(position);
			breaches.pointerPress(position);
			rManager.pointerPress(position);
			wManager.pointerPress(position);
			if ( Windows::System::VirtualKeyModifiers::Shift == args->KeyModifiers )
				for ( std::vector<DCS::MobileEntity*>::iterator i = currentScenario->ship->mobileEntities.begin(); i != currentScenario->ship->mobileEntities.end(); i++ )
					if ( magnitude(DCS::operator+(DCS::operator+(( *i )->position, ( *i )->currentRoom->position), shipPosition) - position) < 30 )
					{
						( *i )->selected = true;
						selected.emplace_back(*i);
						break;
					}

			if ( Windows::System::VirtualKeyModifiers::Control == args->KeyModifiers )
			{
				if ( Windows::System::VirtualKeyModifiers::Shift != args->KeyModifiers )
				{
					for ( int i = 0; i < selected.size(); i++ )
						selected[i]->selected = false;
				}
				selected.clear();
				//long press
				for ( std::vector<DCS::MobileEntity*>::iterator i = currentScenario->ship->mobileEntities.begin(); i != currentScenario->ship->mobileEntities.end(); i++ )
					if ( magnitude(DCS::operator+(DCS::operator+(( *i )->position, ( *i )->currentRoom->position), shipPosition) - position) < 15 )
					{
						( *i )->selected = true;
						selected.emplace_back(*i);
						break;
					}
			}
			else
			{
				//short press

				if ( Windows::System::VirtualKeyModifiers::Shift != args->KeyModifiers )
				{
					Room *tmp = currentScenario->ship->findRoom(position - shipPosition);
					if ( tmp != nullptr )
						for ( int i = 0; i < selected.size(); i++ )
							selected[i]->changeDestination(std::pair<DCS::Point, DCS::Room*>(position - shipPosition - tmp->position, tmp));
				}
			}

		}
}

void DCS::Dx11Engine::OnPointerReleased(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	fManager.pointerRelease();
	objectives.pointerRelease();
	breaches.pointerRelease();
	rManager.pointerRelease();
}

void DCS::Dx11Engine::OnButtonPress(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args)
{
	if ( args->VirtualKey == Windows::System::VirtualKey::Escape )
		switchPause();
	else
		if ( args->VirtualKey == Windows::System::VirtualKey::Space )
			this->currentScenario->ship->addBreach(new HullBreach(*currentScenario->ship->roomBegin(), 50, Point(0,50)));
}

void DCS::Dx11Engine::gameRender(ID2D1DeviceContext * context)
{
	oddFrame++;
	switch ( CurrentScreen )
	{
	case InGame:
		ID2D1SolidColorBrush *brush;
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
		for ( int i = 0; i < currentScenario->ship->silvete.size() - 1; i++ )
		{
			context->DrawLine(D2D1::Point2F((float)( currentScenario->ship->silvete[i] + shipPosition ).first, (float)( currentScenario->ship->silvete[i] + shipPosition ).second), D2D1::Point2F((float)( currentScenario->ship->silvete[i + 1] + shipPosition ).first, (float)( currentScenario->ship->silvete[i + 1] + shipPosition ).second), brush);
		}
		context->DrawLine(D2D1::Point2F((float)( currentScenario->ship->silvete[0] + shipPosition ).first, (float)( currentScenario->ship->silvete[0] + shipPosition ).second), D2D1::Point2F((float)( currentScenario->ship->silvete[currentScenario->ship->silvete.size() - 1] + shipPosition ).first, (float)( currentScenario->ship->silvete[currentScenario->ship->silvete.size() - 1] + shipPosition ).second), brush);
		for ( auto i = currentScenario->ship->roomCbegin(); i != currentScenario->ship->roomCend(); i++ )
		{
			renderRoom(context, **i);
			if ( ( *i )->isOnFire() )
				fManager.add(*i);
		}
		for ( auto k = currentScenario->ship->entityCbegin(); k != currentScenario->ship->entityCend(); k++ )
			renderMobileEntity(context, *k);
		for ( auto var = currentScenario->ship->doorBegin(); var != currentScenario->ship->doorEnd(); var++ )
			renderDoor(context, **var);

		for ( auto i = currentScenario->ship->breachBegin(); i != currentScenario->ship->breachEnd(); i++ )
			renderBreach(context, **i);

		Windows::Foundation::Point p= Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerPosition;
		auto t = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Bounds;
		breaches.addBreach(currentScenario->ship->breachesTop());
		objectives.render(Point(p.X, p.Y) - Point(t.Left, t.Top),context);
		fManager.render(Point(p.X,p.Y)- Point(t.Left, t.Top),context);
		breaches.render(Point(p.X, p.Y) - Point(t.Left, t.Top), context);
		rManager.render(Point(p.X, p.Y) - Point(t.Left, t.Top), context);
		wManager.render(context);

		if ( state != Continue )
		{
			victoryScreen.changeState(state == Won ? true : false);
			victoryScreen.render(context, Point(500, 400));
		}
		if ( isPaused )
			escMenu.render(context);
		brush->Release();
		break;
	case ScreenType::MainMenu:
		main.render(context);
		break;
	}
}

void DCS::Dx11Engine::renderRoom(ID2D1DeviceContext * context, const Room & room)
{
	ID2D1SolidColorBrush *brush;
	context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange, 1.0f), &brush);
	if ( !room.isOnFire() || ( room.isOnFire() && this->oddFrame % 60 < 30 ) )
		switch ( room.currentState().second )
		{
		case Operational:
			brush->Release();
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &brush);
			break;
		case Damaged:
			brush->Release();
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &brush);
			break;
		case OutOfAction:
			brush->Release();
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &brush);
			break;
		case Destroyed:
			brush->Release();
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
			break;
		default:
			break;
		}
	for ( int i = 0; i < room.silvete.size() - 1; i++ )
	{
		context->DrawLine(D2D1::Point2F((float)( room.silvete[i] + shipPosition + room.position ).first, (float)( room.silvete[i] + shipPosition + room.position ).second), D2D1::Point2F((float)( room.silvete[i + 1] + shipPosition + room.position ).first, (float)( room.silvete[i + 1] + shipPosition + room.position ).second), brush);
	}
	context->DrawLine(D2D1::Point2F((float)( room.silvete[0] + shipPosition + room.position ).first, (float)( room.silvete[0] + shipPosition + room.position ).second), D2D1::Point2F((float)( room.silvete[room.silvete.size() - 1] + shipPosition + room.position ).first, (float)( room.silvete[room.silvete.size() - 1] + shipPosition + room.position ).second), brush);
	if ( !room.isOnFire() )
		fManager.remove(&room);
	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, L"whatever", &c);
	wchar_t *buffer = new wchar_t[4];
	swprintf(buffer, 4, L"%03i", room.currentOxygenLevel());
	std::wstring k(buffer);
	context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)( room.position + shipPosition ).first, (float)( room.position + shipPosition ).second, (float)( room.position + shipPosition ).first + 20, (float)( room.position + shipPosition ).second + 30), brush);
	t->Release();
	c->Release();
	delete[]buffer;
	brush->Release();
}

void DCS::Dx11Engine::renderMobileEntity(ID2D1DeviceContext * context, MobileEntity * entity)
{
	ID2D1SolidColorBrush *brush;
	if ( !entity->selected || ( entity->selected && ( oddFrame % 60 ) < 45 ) )
	{
		switch ( entity->type )
		{
		case MobileEntity::Engineer:
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkOrange, 1.0f), &brush);
			break;
		case MobileEntity::Boarder:
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SaddleBrown, 1.0f), &brush);
			break;
		case MobileEntity::CrewMember:
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OrangeRed, 1.0f), &brush);
			break;
		case MobileEntity::Marine:
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &brush);
			break;
		default:
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue, 1.0f), &brush);
		}
		if ( entity->currentStatus() == entity->Nominal )
			context->FillEllipse(D2D1::Ellipse(D2D1::Point2F((float)( entity->position + shipPosition + entity->currentRoom->position ).first, (float)( entity->position + shipPosition + entity->currentRoom->position ).second), 5, 5), brush);
		else
			context->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((float)( entity->position + shipPosition + entity->currentRoom->position ).first, (float)( entity->position + shipPosition + entity->currentRoom->position ).second), 5, 5), brush);
		brush->Release();
	}
	if ( entity->selected )
	{
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);

		auto t = entity->currentPath();
		Point p1 = entity->position + entity->currentRoom->position + shipPosition;
		Point p2 = t.size() != 0 ? entity->currentRoom->findDoor(t[0]).otherSide(t[0]).second + entity->currentRoom->position + shipPosition : entity->destination.first + entity->currentRoom->position + shipPosition;
		context->DrawLine(D2D1::Point2F((float)p1.first, (float)p1.second), D2D1::Point2F((float)p2.first, (float)p2.second), brush);
		p1 = p2;
		for ( int i = 0; i < t.size(); i++ )
		{
			if ( i + 1 == t.size() )
				p2 = entity->destination.first + entity->destination.second->position + shipPosition;
			else
				p2 = t[i]->findDoor(t[i + 1]).otherSide(t[i + 1]).second + t[i]->position + shipPosition;
			context->DrawLine(D2D1::Point2F((float)p1.first, (float)p1.second), D2D1::Point2F((float)p2.first, (float)p2.second), brush);
			p1 = p2;
		}
		brush->Release();
	}
}

void DCS::Dx11Engine::renderDoor(ID2D1DeviceContext * context, Door & toRender)
{
	ID2D1SolidColorBrush *brush;
	context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush);
	Point tmp = toRender.position(toRender.rooms().first) + toRender.rooms().first->position + shipPosition;

	if ( toRender.isItOpen() )
		context->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((float)tmp.first, (float)tmp.second), 4.0f, 4.0f), brush);
	else
		context->FillEllipse(D2D1::Ellipse(D2D1::Point2F((float)tmp.first, (float)tmp.second), 4.0f, 4.0f), brush);
	brush->Release();
}

void DCS::Dx11Engine::renderBreach(ID2D1DeviceContext * context, const HullBreach & toRender)
{
	ID2D1SolidColorBrush *brush;
	if ( oddFrame % 60 < 30 )
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush);
	else
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush);
	context->FillEllipse(D2D1::Ellipse(D2D1::Point2F((float)( toRender.position() + shipPosition ).first, (float)( toRender.position() + shipPosition ).second), 5.0f, (float)toRender.whatSize()/10), brush);
	brush->Release();

}

DCS::Dx11Engine::Dx11Engine()
{
	wManager.addWindow(&fManager);
	wManager.addWindow(&rManager);
	wManager.addWindow(&breaches);
	wManager.addWindow(&objectives);
}

void DCS::Dx11Engine::FireManager::press(Point p)
{
	Point tmp = p - position;
	for ( auto var = fires.begin(); var != fires.end(); var++ )
	{
		Room*  n = var->pointerPress(tmp);
		if ( n != nullptr )
			return;
	}
}

void DCS::Dx11Engine::FireManager::privateRender(ID2D1DeviceContext * context)
{

	if ( (!fires.empty() || remainCount > 0)&&isVisible )
	{
		newFires = false;
		oddFrame++;
		remainCount--;
		ID2D1SolidColorBrush *brush;
		ID2D1SolidColorBrush *brush1;
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &brush1);

		if ( oddFrame % 60 < 30 )
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
		else
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &brush);
		int sizeX = size.first;
		int sizeY = (int)( fires.size() + 2 ) * 20 + 10;
		size.second = sizeY;
		IDWriteFactory* t;
		IDWriteTextFormat*c;
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
		t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
		wchar_t *buffer = new wchar_t[100];
		swprintf(buffer, 100, L"Room - Fire - Functionality - Oxygen");
		context->DrawText(buffer, 37, c, D2D1::RectF((float)position.first + 5, (float)position.second + 20, (float)position.first + sizeX, (float)position.second + 20), brush);

		Windows::Foundation::Point p = Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerPosition;
		auto z = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Bounds;

		Point tmpPoint = Point(p.X, p.Y) - Point(z.Left, z.Top);

		brush->Release();
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);

		for( auto var=fires.begin();var!= fires.end();var++ )
		{
			auto pointer = (*var).render(position, context, tmpPoint);
			if ( pointer != nullptr )
				context->DrawLine(D2D1::Point2F((float)(pointer->position+shipPosition).first, (float)(pointer->position+ shipPosition).second), D2D1::Point2F((float)( ( *var ).location()+position).first, (float)( ( *var ).location() + position ).second+5), brush);

		}

		delete[]buffer;
		brush->Release();
		brush1->Release();
		t->Release();
		c->Release();
	}
	else
	{
		isVisible = false;
	}
}

DCS::Dx11Engine::FireManager::FireManager(Point position):	DraggableWindow(D2D1::ColorF::White, Point(350, 0), position,std::wstring(L"Fires"))
{
}

void DCS::Dx11Engine::FireManager::add(Room * r)
{
	typedef std::function<std::wstring()> f;
	typedef SmartWString::StorageType t;
	for ( auto i = fires.begin(); i != fires.end(); i++ )
		if ( i->whatResult() == r )
			return;
	std::vector<std::pair<SmartWString, Point>> tmp;
	std::vector<SmartWString::StorageType>*tmpvector=new std::vector<SmartWString::StorageType>();
	std::vector<Button<Room*>*> buttonVector;
	if(!isVisible)
		newFires = true;

	float c1[4] = { 1.0,1.0,1.0,1.0 };
	float c2[4] = { 0.0,0.0,0.0,1.0 };
	float c3[4] = { 1.0,0.0,0.0,1.0 };
	float c4[4] = { 0.0,1.0,0.0,1.0 };
	float c5[4] = { 0.0,0.0,0.0,1.0 };

	buttonVector.push_back(new ActiveButton<Room*>(r, nullptr, std::wstring(L"Depresurise"), Point(100, 20), Point(220, 0), c1, c4, c3, 
		std::unique_ptr<std::function<bool(bool)>>(new std::function<bool(bool)>(std::bind([](bool, Room*r) 
	{
		return r->oxygenWanted() == 100; 
	},
			std::placeholders::_1, r))),
		std::unique_ptr<std::function<bool(bool)>>(new std::function<bool(bool)>(std::bind([](bool b, Room*r) 
	{
		r->setDesiredOxygen(b ? 0 : 1000);
		return r->oxygenWanted() == 100; },
			std::placeholders::_1, r)))));

	tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return enumToString(r->whatType()); }, r))));
	tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) 
	{ 
		wchar_t tmp[4];
		swprintf_s(tmp, L"%03d",(int)r->firePrecentage());
		std::wstring tmpp(tmp);
		return tmpp;
	}
	, r))));
	tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return enumToString(r->currentState().second); }, r))));
	tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return std::to_wstring((int)r->currentOxygenLevel()); }, r))));


	tmp.push_back(std::pair<SmartWString, Point>(SmartWString(std::wstring(L"%!  %!%% %! %!%%"), tmpvector), Point(0, 0)));
	fires.emplace_back(Point(300, 20), Point(5, ( fires.size() + 2 ) * 20), tmp, buttonVector, c1, c2, c4, nullptr, r, nullptr);
}

void DCS::Dx11Engine::FireManager::remove(const Room * const r)
{
	int k = (int)fires.size();
	for ( auto i = fires.begin(); i != fires.end(); i++ )
		if ( i->whatResult() == r )
		{
			fires.erase(i);
			if ( k == 1 )
				remainCount = 120;
			return;
		}
}

bool DCS::Dx11Engine::FireManager::newContent()
{
	return newFires;
}

std::wstring  DCS::enumToString(Room::RoomType t)
{
	switch ( t )
	{
	case DCS::Room::Bridge:
		return std::wstring(L"Bridge");
	case DCS::Room::Engineering:
		return std::wstring( L"Engineering");
	case DCS::Room::Corridor:
		return std::wstring(L"Corridor");
	case DCS::Room::LifeSupport:
		return  std::wstring(L"Life Support");
	default:
		throw std::runtime_error("Error in enum to string");
	}
}

std::wstring DCS::enumToString(DamageState t)
{
	switch ( t )
	{
	case Operational:
		return std::wstring(L"Operational");
	case Damaged:
		return std::wstring(L"Damaged");
	case OutOfAction:
		return std::wstring(L"Out of action");
	case Destroyed:
		return std::wstring(L"Destroyed");
	default:
		throw std::runtime_error("Error in enum to string");

	}
}

DCS::Dx11Engine::EscMenu::EscMenu(Point p)
{
	float textColor[4] = { .0f, .0f, .0f, 1 };
	float color[4] = { 1.f, 1.f, 1.0f ,1 };
	position = p;
	sizeX = 300;
	sizeY = 700;
	buttons.push_back(Button<EscMenuButton>(Resume,Null,std::wstring(L"Resume"),Point(100,20),Point(100, 50),textColor,color));
	buttons.push_back(Button<EscMenuButton>(Save, Null, std::wstring(L"Save"), Point(100, 20), Point(100, 100), textColor, color));
	buttons.push_back(Button<EscMenuButton>(Settings, Null, std::wstring(L"Settings"), Point(100, 20), Point(100, 150), textColor, color));
	buttons.push_back(Button<EscMenuButton>(Exit, Null, std::wstring(L"Exit"), Point(100, 20), Point(100, 200), textColor, color));

}

DCS::Dx11Engine::EscMenu::PressResult DCS::Dx11Engine::EscMenu::OnPointerPressed(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	DCS::Point tmp = DCS::Point(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
	for each ( auto var in buttons )
	{
		switch ( var.OnPointerPressed(tmp - position) )
		{
		case Null:
			break;
		case Resume:
			return ExitMenu;
		case Exit:
			return ReturnToMain;
		case Settings:
			return None;
		}
	}
	return None;
}

void DCS::Dx11Engine::EscMenu::render(ID2D1DeviceContext * context) const
{
	ID2D1SolidColorBrush *brush;
	context->CreateSolidColorBrush(D2D1::ColorF(color[0], color[1], color[2], 1.0f), &brush);
	context->FillRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first + sizeX, (float)position.second + sizeY), brush);
	for each ( auto var in buttons )
	{
		var.render(context, position);
	}


	brush->Release();
}

DCS::Dx11Engine::MainMenu::MainButton::MainButton(Point p, MainMenuButton b)
{
	type = b;
	position = p;
}

void DCS::Dx11Engine::MainMenu::MainButton::render(ID2D1DeviceContext * context, Point offset) const
{
	Point tmp = position + offset;
	ID2D1SolidColorBrush *background;
	ID2D1SolidColorBrush *text;
	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], 1.0f), &text);
	context->CreateSolidColorBrush(D2D1::ColorF(color[0], color[1], color[2], 1.0f), &background);
	context->FillRectangle(D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + sizeX, (float)tmp.second + sizeY), background);


	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
	wchar_t *buffer = new wchar_t[100];
	swprintf(buffer, 100, enumToChar(type));
	std::wstring k(buffer);
	context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + sizeX, (float)tmp.second + sizeY), text);
	delete[] buffer;
	background->Release();
	text->Release();
	t->Release();
	c->Release();
}

DCS::Dx11Engine::MainMenu::MainMenuButton DCS::Dx11Engine::MainMenu::MainButton::OnPointerPressed(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args, Point offset)
{
	DCS::Point tmp = DCS::Point(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y) - offset;
	if ( tmp.first > position.first&&tmp.second > position.second&&tmp.first < position.first + sizeX&&tmp.second < position.second + sizeY )
		return type;
	else
		return Null;
}

wchar_t * DCS::Dx11Engine::MainMenu::MainButton::enumToChar(MainMenuButton type)
{
	switch ( type )
	{
	case NewGame:
		return L"New Game";
	case Continue:
		return L"Continue";
	case Settings:
		return L"Settings";
	case Exit:
		return L"Exit";
	default:
		throw std::runtime_error("enum to char error-was NULL");
	}
}


template<typename T>
void DCS::Dx11Engine::Button<T>::basicRender(const float c[4] , ID2D1DeviceContext * context, Point offset) const
{
	Point tmp = position + offset;
	ID2D1SolidColorBrush *background;
	ID2D1SolidColorBrush *text;
	const int &sizeX = size.first;
	const int &sizeY = size.second;
	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], 1.0f), &text);
	context->CreateSolidColorBrush(D2D1::ColorF(c[0], c[1], c[2], 1.0f), &background);
	context->FillRectangle(D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + sizeX, (float)tmp.second + sizeY), background);


	IDWriteFactory* t;
	IDWriteTextFormat*k;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &k);
	context->DrawText(this->text.c_str(), (UINT32)this->text.size(), k, D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + sizeX, (float)tmp.second + sizeY), text);

	background->Release();
	text->Release();
	t->Release();
	k->Release();
}

template<typename T>
DCS::Dx11Engine::Button<T>::Button(T type, T nullValue, std::wstring name, Point size, Point position, float textColor[4], float color[4])
{
	this->type = type;
	this->text = name;
	this->nullValue = nullValue;
	this->size = size;
	this->position = position;
	for(int i=0;i<4;i++)
		this->textColor[i] = textColor[i];
	for ( int i = 0; i < 4; i++ )
		this->color[i] = color[i];
}

template<typename T>
void DCS::Dx11Engine::Button<T>::render(ID2D1DeviceContext * context, Point offset)
{
	basicRender(color, context, offset);
}


template<typename T>
T DCS::Dx11Engine::Button<T>::OnPointerPressed(Point p)
{
	if ( position.first < p.first&&position.second < p.second&&p.first < ( position + size ).first&&p.second < ( position + size ).second )
		return type;
	else
		return nullValue;
}

template<typename T>
T DCS::Dx11Engine::Button<T>::whatResult() const
{
	return type;
}


void DCS::Dx11Engine::ObjectiveScreen::privateRender(ID2D1DeviceContext * context)
{
	int sizeY = (int)( 60 + objective->roomsRequired().size() * 20 + objective->roomsRequired().size() * 20 );
	Point const & tmp = position;
	ID2D1SolidColorBrush *text;
	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], 1.0f), &text);

	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
	std::wstring k;
	wchar_t *buffer = new wchar_t[100];
	int f = 20;
	if ( objective->roomsRequired().size() > 0 )
	{
		swprintf(buffer, 100, L"Protect:");
		k = std::wstring(buffer);
		context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second + f, (float)tmp.first + size.first, (float)tmp.second + sizeY), text);
		f += 20;
		for each ( auto var in objective->roomsRequired() )
		{
			swprintf(buffer, 100, L"* %s must be %s or better", enumToString(var.first->whatType()).c_str(), enumToString(var.second).c_str());
			k = std::wstring(buffer);
			context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second + f, (float)tmp.first + size.first, (float)tmp.second + sizeY), text);
			f += 20;
		}
	}
	auto n = objective->ticsRemaining();
	swprintf(buffer, 100, L"Survive for:");
	k = std::wstring(buffer);
	context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second + f, (float)tmp.first + size.first, (float)tmp.second + sizeY), text);
	f += 20;
	for each ( auto var in n )
	{
		swprintf(buffer, 100, L"* %02d:%02d", var / 60 / 60, ( var / 60 ) % 60);
		k = std::wstring(buffer);
		context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second + f, (float)tmp.first + size.first, (float)tmp.second + sizeY), text);
		f += 20;
	}

	delete[] buffer;
	text->Release();
	t->Release();
	c->Release();

}

void DCS::Dx11Engine::ObjectiveScreen::press(Point)
{
}

DCS::Dx11Engine::ObjectiveScreen::ObjectiveScreen(DCS::Scenario* obj, DCS::Point p):DraggableWindow(D2D1::ColorF::White,Point(300,200),p,std::wstring(L"Objectives"))
{
	objective = obj;
}

void DCS::Dx11Engine::ObjectiveScreen::changeScenario(Scenario * obj)
{
	objective = obj;
}

void DCS::Dx11Engine::VictoryScreen::render(ID2D1DeviceContext * context, Point offset) const
{
	Point tmp = offset;
	ID2D1SolidColorBrush *background;
	ID2D1SolidColorBrush *text;
	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], 1.0f), &text);
	context->CreateSolidColorBrush(D2D1::ColorF(color[0], color[1], color[2], 1.0f), &background);
	context->FillRectangle(D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + size.first, (float)tmp.second + size.second), background);


	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30, L"whatever", &c);
	wchar_t *buffer = new wchar_t[100];
	swprintf(buffer, 100, hasWon ? L"You have won!:" : L"You have lost!");
	std::wstring k(buffer);
	context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + size.first, (float)tmp.second + size.second), text);

	background->Release();
	text->Release();
	t->Release();
	c->Release();
}

void DCS::Dx11Engine::VictoryScreen::changeState(bool newState)
{
	hasWon = newState;
}

DCS::Dx11Engine::MainMenu::PressResult DCS::Dx11Engine::MainMenu::OnPointerPressed(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	for each ( auto var in buttons )
	{
		switch ( var.OnPointerPressed(sender, args, position) )
		{
		case Null:
			break;
		case NewGame:
			return ToGame;
		case Exit:
			return ExitApp;
		}
	}
	return PressResult::Nothing;
}

void DCS::Dx11Engine::MainMenu::render(ID2D1DeviceContext * context) const
{
	context->Clear(D2D1::ColorF(D2D1::ColorF::DarkGray));
	ID2D1SolidColorBrush *background;
	context->CreateSolidColorBrush(D2D1::ColorF(color[0], color[1], color[2], 1.0f), &background);
	context->FillRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first + sizeX, (float)position.second + sizeY), background);
	for each ( auto var in buttons )
	{
		var.render(context, position);
	}
	background->Release();
}

DCS::Dx11Engine::MainMenu::MainMenu()
{
	for ( MainMenuButton i = NewGame; i < Null; i = (MainMenuButton)( i + 1 ) )
		buttons.push_back(MainButton(Point(50, 50 + 50 * i), i));
	position = Point(600, 400);

}

DCS::Dx11Engine::DraggableWindow::DraggableWindow(D2D1::ColorF::Enum c, Point size, Point position,std::wstring name)
{
	this->name = name;
	color = c;
	this->size = size;
	this->position = position;
	dragArea = Point(position.first, 20);
	isDragged = false;
	isVisible = true;
	grabPoint = { 0,0 };
	float a[4] = { 0,0,0,1 };
	float b[4] = { 0,0,1,1 }; 
	closeButton = Button<bool>(true, false, std::wstring(L"X"), Point(20, 20), Point(size.first - 20, 0),a,b );
}

void DCS::Dx11Engine::DraggableWindow::render(DCS::Point p, ID2D1DeviceContext * context)
{
	if ( isVisible )
	{
		if ( isDragged )
			position = p - grabPoint;;
		ID2D1SolidColorBrush *brush;
		ID2D1SolidColorBrush *brush1;
		context->CreateSolidColorBrush(D2D1::ColorF(color, 1.0f), &brush);
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f), &brush1);
		int sizeX = size.first;
		int sizeY = size.second;

		IDWriteFactory* t;
		IDWriteTextFormat*c;
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
		t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);


		context->FillRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first + sizeX, (float)position.second + sizeY), brush);
		context->FillRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first + sizeX, (float)position.second + 20), brush1);
		context->DrawText(name.c_str(), name.length(), c, D2D1::RectF((float)position.first+5, (float)position.second, (float)position.first + sizeX, (float)position.second + sizeY), brush);
		brush->Release();
		brush1->Release();
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
		context->DrawRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first + sizeX, (float)position.second + sizeY), brush);
		brush->Release();
		closeButton.render(context, position);
		privateRender(context);
	}
}

void DCS::Dx11Engine::DraggableWindow::pointerPress(Point p)
{
	if ( closeButton.OnPointerPressed(p - position) )
		isVisible = false;
	if ( p.first > position.first&&p.first<position.first + size.first&&p.second>position.second&&p.second < position.second + size.second )
		if ( p.first > position.first&&p.first<position.first + dragArea.first&&p.second>position.second&&p.second < position.second + dragArea.second )
		{
			isDragged = true;
			grabPoint = p - position;
		}
		else
			press(p);
}

void DCS::Dx11Engine::DraggableWindow::pointerRelease()
{
	isDragged = false;
}

std::wstring DCS::Dx11Engine::DraggableWindow::title()
{
	return name;
}

bool DCS::Dx11Engine::DraggableWindow::visible()
{
	return isVisible;
}

bool DCS::Dx11Engine::DraggableWindow::newContent()
{
	return false;
}

void DCS::Dx11Engine::DraggableWindow::open()
{
	isVisible = true;
}

void DCS::Dx11Engine::BreachScreen::privateRender(ID2D1DeviceContext * context)
{
	newBreaches = false;
	oddFrame++;
	remainCount--;
	ID2D1SolidColorBrush *brush;
	ID2D1SolidColorBrush *brush1;
	context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &brush1);

	if ( abs(oddFrame % 60) < 30 )
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
	else
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &brush);
	int sizeX = size.first;
	int sizeY = (int)( rooms.size() + 2 ) * 20 + 10;
	size.second = sizeY;
	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
	wchar_t *buffer = new wchar_t[100];
	swprintf(buffer, 100, L"Room - Oxygen");
	context->DrawText(buffer, 14, c, D2D1::RectF((float)position.first + 5, (float)position.second + 20, (float)position.first + sizeX, (float)position.second + 20), brush);
	for ( int i = 0; i < rooms.size(); i++ )
	{
		swprintf(buffer, 100, L"%s  %03d%%", DCS::enumToString(rooms[i]->whatType()).c_str(), ( rooms[i]->currentOxygenLevel() ));
		std::wstring t(buffer);
		context->DrawText(buffer, (UINT32)t.length(), c, D2D1::RectF((float)position.first + 5, (float)position.second + ( i + 2 ) * 20, (float)position.first + sizeX, (float)position.second + ( i + 2 ) * 20 + 20), brush);
	}
	delete[]buffer;
	brush->Release();
	brush1->Release();
	t->Release();
	c->Release();
}

void DCS::Dx11Engine::BreachScreen::press(DCS::Point)
{
}

DCS::Dx11Engine::BreachScreen::BreachScreen(Point p) : DraggableWindow(D2D1::ColorF::Enum::White, Point(200, 0), p,std::wstring(L"Hull Breaches"))
{
	int remainCount = 0;
	int oddFrame = 0;
}

void DCS::Dx11Engine::BreachScreen::addBreach(Room *r)
{
	if ( r != nullptr )
		for ( auto i = rooms.begin(); i != rooms.end(); i++ )
			if ( *i == r )
				return;
			else;
	else
		return;
	rooms.push_back(r);
	newBreaches = true;
}

bool DCS::Dx11Engine::BreachScreen::newContent()
{
	return newBreaches;
}

template<typename ButtonResult, typename HoverResult>
HoverResult DCS::Dx11Engine::HoverSection<ButtonResult, HoverResult>::render(DCS::Point offset, ID2D1DeviceContext * context, Point cursor)
{
	ID2D1SolidColorBrush *textBrush;
	ID2D1SolidColorBrush *backgroundBrush;
	HoverResult tmpResult;

	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], textColor[3]), &textBrush);
	Point relativeCursor = cursor - position - offset;
	if ( relativeCursor.first > 0 && relativeCursor.second > 0 && relativeCursor.first < size.first&&relativeCursor.second < size.second )
	{
		context->CreateSolidColorBrush(D2D1::ColorF(activeBackColor[0], activeBackColor[1], activeBackColor[2], activeBackColor[3]), &backgroundBrush);
		tmpResult = result;
	}
	else
	{
		context->CreateSolidColorBrush(D2D1::ColorF(backColor[0], backColor[1], backColor[2], backColor[3]), &backgroundBrush);
		tmpResult = nullHover;
	}
	Point tmp = offset + position;
	context->FillRectangle(D2D1::RectF(tmp.first, tmp.second, ( tmp + size ).first, ( tmp + size ).second), backgroundBrush);
	
	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast<IUnknown**>( &t ));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);

	for each ( auto var in texts )
	{
		tmp = offset + position + var.second;
		auto tmpRect = D2D1::RectF(tmp.first, tmp.second, (float)(tmp.first + (*var.first).size() * 20),(float) tmp.second + 20);
		context->DrawText((*var.first).c_str(), (int)(*var.first).size(), c, tmpRect, textBrush);
	}

	for ( auto var = buttons.begin(); var != buttons.end(); var++ )
	{
		tmp = offset + position;
		(*var)->render(context, tmp);
	}
	t->Release();
	c->Release();
	textBrush->Release();
	backgroundBrush->Release();
	return tmpResult;
}

template<typename ButtonResult, typename HoverResult>
ButtonResult DCS::Dx11Engine::HoverSection<ButtonResult, HoverResult>::pointerPress(Point p)
{
	for ( auto var = buttons.begin(); var != buttons.end(); var++ )
	{
		auto tmp = (*var)->OnPointerPressed(p - position);
		if ( tmp != nullResult )
			return tmp;
	}
	return nullResult;
}

template<typename ButtonResult, typename HoverResult>
DCS::Dx11Engine::HoverSection<ButtonResult, HoverResult>::HoverSection(Point size, Point relativePosition, std::vector<std::pair<SmartWString, Point>> textVector, std::vector < Button<ButtonResult>*> buttons, float backColor[4], float textColor[4], float activeBackColor[4], ButtonResult nullResult, HoverResult result, HoverResult nullHover)
{
	this->size = size;
	this->position = relativePosition;
	texts = textVector;
	for ( int i = 0; i < 4; i++ )
		this->backColor[i] = backColor[i];
	for ( int i = 0; i < 4; i++ )
		this->textColor[i] = textColor[i];
	for ( int i = 0; i < 4; i++ )
		this->activeBackColor[i] = activeBackColor[i];
	this->nullResult = nullResult;
	this->result = result;
	this->nullHover = nullHover;
	this->buttons = buttons;
}

template<typename ButtonResult, typename HoverResult>
ButtonResult DCS::Dx11Engine::HoverSection<ButtonResult, HoverResult>::whatResult(int) const
{
	return buttons[0].whatResult();
}

template<typename ButtonResult, typename HoverResult>
HoverResult DCS::Dx11Engine::HoverSection<ButtonResult, HoverResult>::whatResult() const
{
	return result;
}

template<typename ButtonResult, typename HoverResult>
DCS::Point DCS::Dx11Engine::HoverSection<ButtonResult, HoverResult>::location()
{
	return position;
}


std::wstring DCS::Dx11Engine::SmartWString::operator*()
{
	std::wstring tmp;
	std::wstring tmpString;
	auto k = argVector->begin();
	for( std::wstring::iterator i=format.begin();i!=format.end();i++ )
		switch ( *i )
		{
		case L'%':
		{
			switch ( *++i )
			{
			case L'!':
				tmp.append((* ( *k ))( ));
				k++;
				break;
			default:
				tmp.push_back(*i);
			}
			break;
		}


		default:
			tmp.push_back(*i);
			break;
		}

	return tmp;
}

DCS::Dx11Engine::SmartWString::SmartWString(std::wstring whatFormat, std::vector<StorageType> *v)
{
	format = whatFormat;
	argVector = v;
}

DCS::Dx11Engine::SmartWString::~SmartWString()
{
	//	while ( !argVector.empty() )
	//		if ( format.substr(0, 2) == std::wstring(L"%!") )
	//		{
	//			delete argVector.front().function;
	//			argVector.erase(argVector.begin());
	//		}
	//		else
	//			if ( format.front() == L'%' )
	//			{
	//				format.erase(format.begin());
	//				format.erase(format.begin());
	//				argVector.erase(argVector.begin());
	//			}
	//			else
	//				format.erase(format.begin());
	//
}
template<typename T>
DCS::Dx11Engine::ActiveButton<T>::ActiveButton(T type, T nullValue, std::wstring name, Point size, Point position, float textColor[4], float color[4], float activeColor[4]) : Button(type, nullValue, name, size, position, textColor, color)
{
	isActive = false;
	for ( int i = 0; i < 4; i++ )
		this->activeColor[i] = activeColor[i];
}
template<typename T>
DCS::Dx11Engine::ActiveButton<T>::ActiveButton(T type, T nullValue, std::wstring name, Point size, Point position, float textColor[4], float color[4], float activeColor[4], std::unique_ptr<std::function<bool(bool)>>& condition, std::unique_ptr<std::function<bool(bool)>>& onPress) : Button(type, nullValue, name, size, position, textColor, color)
{
	isActive = false;
	for ( int i = 0; i < 4; i++ )
		this->activeColor[i] = activeColor[i];
	this->condition.swap( condition);
	this->onPress.swap( onPress);
}
template<typename T>
void DCS::Dx11Engine::ActiveButton<T>::render(ID2D1DeviceContext * context, Point offset) 
{
	isActive = ( *condition )( isActive );
	basicRender(isActive ? activeColor : color, context, offset);
}

template<typename T>
T DCS::Dx11Engine::ActiveButton<T>::OnPointerPressed(Point position)
{
	if ( Button::OnPointerPressed(position) == nullValue )
		return nullValue;
	else
	{
		isActive = ( *onPress )( isActive );
		return type;
	}
}

void DCS::Dx11Engine::RoomManager::privateRender(ID2D1DeviceContext * context)
{
	Windows::Foundation::Point p = Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerPosition;
	auto z = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Bounds;

	Point tmpPoint = Point(p.X, p.Y) - Point(z.Left, z.Top);

	ID2D1SolidColorBrush*brush;
	context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);

	for( auto v = rooms.begin();v!=rooms.end();v++ )
	{
		auto n = (*v).render(position, context, tmpPoint);
		if ( n != nullptr )
			context->DrawLine(D2D1::Point2F((float)( n->position + shipPosition ).first, (float)( n->position + shipPosition ).second), D2D1::Point2F((float)( ( *v ).location() + position ).first, (float)( ( *v ).location() + position ).second + 5), brush);

	}
	brush->Release();
}

void DCS::Dx11Engine::RoomManager::press(DCS::Point p)
{
	Point tmp = p - position;
	for ( auto var = rooms.begin(); var != rooms.end(); var++ )
	{
		Room*  n = var->pointerPress(tmp);
		if ( n != nullptr )
			return;
	}
}

DCS::Dx11Engine::RoomManager::RoomManager(std::vector<Room*> &r, Point position):DraggableWindow(D2D1::ColorF::White,Point(400,300),position,std::wstring(L"Rooms"))
{
	typedef std::function<std::wstring()> f;
	float c1[4] = { 1.0,1.0,1.0,1.0 };
	float c2[4] = { 0.0,0.0,0.0,1.0 };
	float c3[4] = { 0.0,1.0,0.0,1.0 };
	int i = 0;
	for each ( auto var in r )
	{
		i += 20;
		std::vector < std::pair<SmartWString, Point>>tmpString;
		std::vector<SmartWString::StorageType> *tmpvector=new std::vector<SmartWString::StorageType>;
		tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return enumToString(r->whatType()); }, var))));
		tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return std::to_wstring(r->currentOxygenLevel()); }, var))));
		tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return std::to_wstring((int)r->firePrecentage()); }, var))));
		tmpvector->emplace_back(std::unique_ptr<f>(new f(std::bind([](Room*r) { return enumToString(r->currentState().second); }, var))));

		tmpString.push_back(std::pair<SmartWString, Point>(SmartWString(std::wstring(L"%! %!% %!% %!"), tmpvector), Point(0, 0)));
		std::vector<Button<Room*>*> tmpButton;
		tmpButton.push_back(new ActiveButton<Room*>(var, nullptr,std::wstring( L"Depresurise"), Point(100, 20),Point(300,0), c2, c1, c3,
			std::unique_ptr<std::function<bool(bool)>>(new std::function<bool(bool)>(std::bind([](bool, Room*r)
		{
			return r->oxygenWanted() == 100;
		},
				std::placeholders::_1, var))),
			std::unique_ptr<std::function<bool(bool)>>(new std::function<bool(bool)>(std::bind([](bool b, Room*r)
		{
			r->setDesiredOxygen(b ? 0 : 1000);
			return r->oxygenWanted() == 100;
		},
				std::placeholders::_1, var)))));
		rooms.emplace_back(DCS::Point(300, 20), DCS::Point(5, i), tmpString, tmpButton, c1, c2, c3, nullptr, var, nullptr);
	}
}

void DCS::Dx11Engine::WindowManager::render(ID2D1DeviceContext * context)
{
	oddFrame++;
	ID2D1SolidColorBrush *brush;
	context->CreateSolidColorBrush(D2D1::ColorF(color[0],color[1],color[2], color[3]), &brush);
	context->FillRectangle(D2D1::RectF(0, 0, 10000, 40), brush);
	brush->Release();
	for ( auto var = windows.begin(); var != windows.end();var++ )
	{
		var->render(context, Point(0, 0));
	}
}

void DCS::Dx11Engine::WindowManager::pointerPress(Point p)
{
	for ( auto button = windows.begin(); button != windows.end(); button++ )
		if ( button->OnPointerPressed(p) )
			return;
}

void DCS::Dx11Engine::WindowManager::addWindow(DraggableWindow *w)
{
	float c1[4] = { 0,0,0,1 };
	float c2[4] = { 1,1,1,1 };
	float c3[4] = { 1,0,0,1 };
	windows.push_back(ActiveButton<bool>(true, false, std::wstring(w->title()), Point(200, 20), Point(250 * ( windows.size() + 1 ), 10), c1, c2, c3,
		std::unique_ptr<std::function<bool(bool)>>
		(new std::function<bool(bool)>(std::bind([](bool, DraggableWindow*r,unsigned int*oddFrame)
	{
		return ( !r->visible() ) && r->newContent() && ( ( *oddFrame ) % 60 < 45 );
	}
			, std::placeholders::_1, w, &oddFrame))),
		std::unique_ptr<std::function<bool(bool)>>
		(new std::function<bool(bool)>(std::bind([](bool b, DraggableWindow*r)
	{
		if ( !r->visible() )
			r->open();
		return b;
	}
			, std::placeholders::_1, w)))
		));
}

DCS::Dx11Engine::WindowManager::WindowManager(float color[4])
{
	for ( int i = 0; i < 4; i++ )
		this->color[i] = color[i];
}

DCS::Dx11Engine::WindowManager::WindowManager()
{
}
