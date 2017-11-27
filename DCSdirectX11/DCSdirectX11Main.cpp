﻿#include "pch.h"
#include "DCSdirectX11Main.h"
#include "Common\DirectXHelper.h"
#include "../DCSengine/DCSengine.h"
#include "Common\DeviceResources.h"

using namespace DCSdirectX11;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;




//void DCS::Game::render(ID2D1DeviceContext * context)
//{
//	ID2D1SolidColorBrush *brush;
//	auto hr = context->CreateSolidColorBrush(
//		D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
//		&brush);
//	context->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(x + 20, y + 20), brush);
//}
//
//void DCS::Game::timerTick()
//{
//	x++;
//	y++;
//}




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
	if (m_timer.GetFrameCount() == 0)
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
	float tmp[4] = { .18f, .208f, .38f ,1.0f};
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
	if ( CurrentScreen == ScreenType::MainMenu )
		switch ( main.OnPointerPressed(sender, args) )
		{
		case MainMenu::PressResult::ToGame:
			isPaused = false;
			vessel = new Ship();
			currentScenario = new Scenario(new Timed(rooms, vessel, 10000), vessel);
			CurrentScreen = ScreenType::InGame;
			objectives.changeScenario(currentScenario);
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

}

void DCS::Dx11Engine::OnButtonPress(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args)
{
	if (args->VirtualKey == Windows::System::VirtualKey::Space)
		currentScenario->ship->rooms[0]->setOnFire();
	else
		if (args->VirtualKey == Windows::System::VirtualKey::Escape)
			switchPause();
}

void DCS::Dx11Engine::gameRender(ID2D1DeviceContext * context)
{
	oddFrame++;
	switch(CurrentScreen )
	{
	case InGame:
		ID2D1SolidColorBrush *brush;
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
		objectives.render(context, Point(600, 500));
		for ( int i = 0; i < currentScenario->ship->silvete.size() - 1; i++ )
		{
			context->DrawLine(D2D1::Point2F((float)( currentScenario->ship->silvete[i] + shipPosition ).first, (float)( currentScenario->ship->silvete[i] + shipPosition ).second), D2D1::Point2F((float)( currentScenario->ship->silvete[i + 1] + shipPosition ).first, (float)( currentScenario->ship->silvete[i + 1] + shipPosition ).second), brush);
		}
		context->DrawLine(D2D1::Point2F((float)( currentScenario->ship->silvete[0] + shipPosition ).first, (float)( currentScenario->ship->silvete[0] + shipPosition ).second), D2D1::Point2F((float)( currentScenario->ship->silvete[currentScenario->ship->silvete.size() - 1] + shipPosition ).first, (float)( currentScenario->ship->silvete[currentScenario->ship->silvete.size() - 1] + shipPosition ).second), brush);
		for ( std::vector<Room*>::iterator i = currentScenario->ship->rooms.begin(); i != currentScenario->ship->rooms.end(); i++ )
			renderRoom(context, **i);
		for ( std::vector<DCS::MobileEntity*>::iterator i = currentScenario->ship->mobileEntities.begin(); i != currentScenario->ship->mobileEntities.end(); i++ )
			renderMobileEntity(context, *i);
		fManager.render(context);
		if ( isPaused )
			escMenu.render(context);


		if ( state != Continue )
			victoryScreen.render(context, Point(500, 400));
		brush->Release();
		break;
	case ScreenType::MainMenu:
		main.render(context);
		break;
	}
}

void DCS::Dx11Engine::renderRoom(ID2D1DeviceContext * context, Room & room)
{
	ID2D1SolidColorBrush *brush;
	context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange, 1.0f), &brush);
	if(!room.isOnFire()||(room.isOnFire()&&this->oddFrame%60<30))
		switch (room.currentState().second)
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
	for (int i = 0; i < room.silvete.size() - 1; i++)
	{
		context->DrawLine(D2D1::Point2F((float)(room.silvete[i]+shipPosition+room.position).first, (float)(room.silvete[i]+shipPosition + room.position).second), D2D1::Point2F((float)(room.silvete[i + 1]+shipPosition + room.position).first, (float)(room.silvete[i + 1]+shipPosition + room.position).second), brush);
	}
	context->DrawLine(D2D1::Point2F((float)(room.silvete[0]+shipPosition + room.position).first, (float)(room.silvete[0]+shipPosition + room.position).second), D2D1::Point2F((float)(room.silvete[room.silvete.size()-1]+shipPosition + room.position).first, (float)(room.silvete[room.silvete.size()-1]+shipPosition + room.position).second), brush);
	if (room.isOnFire())
	{
		fManager.add(&room);
		IDWriteFactory* t;
		IDWriteTextFormat*c;
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&t));
		t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, L"whatever", &c);
		wchar_t *buffer = new wchar_t[3];
		swprintf(buffer, 3 ,L"%02i", (room.fireValue() * 100) / MAX_FIRE_VALUE);
		std::wstring k(buffer);
		context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)(room.position + shipPosition).first, (float)(room.position + shipPosition).second, (float)(room.position + shipPosition).first + 20, (float)(room.position + shipPosition).second + 30), brush);
		t->Release();
		c->Release();
	}
	else
		fManager.remove(&room);
	
	brush->Release();
}

void DCS::Dx11Engine::renderMobileEntity(ID2D1DeviceContext * context, MobileEntity * entity)
{
	ID2D1SolidColorBrush *brush;
	if (!entity->selected || (entity->selected&&(oddFrame%60)<45))
	{
		switch (entity->type)
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
		if(entity->currentStatus()==entity->Nominal)
		context->FillEllipse(D2D1::Ellipse(D2D1::Point2F((float)(entity->position + shipPosition + entity->currentRoom->position).first, (float)(entity->position + shipPosition + entity->currentRoom->position).second), 5, 5), brush);
		else
			context->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((float)(entity->position + shipPosition + entity->currentRoom->position).first, (float)(entity->position + shipPosition + entity->currentRoom->position).second), 5, 5), brush);
		brush->Release();
	}
	if (entity->selected)
	{
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);

		auto t = entity->currentPath();
		Point p1=entity->position+entity->currentRoom->position+shipPosition;
		Point p2 =t.size()!=0? entity->currentRoom->findDoor(t[0]) + entity->currentRoom->position + shipPosition: entity->destination.first + entity->currentRoom->position+ shipPosition;
		context->DrawLine(D2D1::Point2F((float)p1.first, (float)p1.second), D2D1::Point2F((float)p2.first, (float)p2.second), brush);
		p1 = p2;
		for (int i=0;i<t.size();i++)
		{
			if (i + 1 == t.size())
				p2 = entity->destination.first + entity->destination.second->position + shipPosition;
			else
				p2 = t[i]->findDoor(t[i + 1]) + t[i]->position + shipPosition;
			context->DrawLine(D2D1::Point2F((float)p1.first, (float)p1.second), D2D1::Point2F((float)p2.first, (float)p2.second), brush);
			p1 = p2;
		}
		brush->Release();
	}
}

void DCS::Dx11Engine::FireManager::render(ID2D1DeviceContext * context)
{
	
	if (!fires.empty()||remainCount>0)
	{
		oddFrame++;
		remainCount--;
		ID2D1SolidColorBrush *brush;
		ID2D1SolidColorBrush *brush1;
		context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &brush1);

		if(oddFrame%60<30)
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
		else
			context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &brush);
		int sizeX = 250;
		int sizeY = (int)(fires.size() + 2) * 20;
		context->FillRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first + sizeX, (float)position.second + sizeY), brush1);
		IDWriteFactory* t;
		IDWriteTextFormat*c;
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&t));
		t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
		wchar_t *buffer = new wchar_t[100];
		swprintf(buffer,100, L"FIRE!");
		context->DrawText(buffer, 5, c, D2D1::RectF((float)position.first+100, (float)position.second, (float)position.first + sizeX, (float)position.second + 10), brush);
		swprintf(buffer,100, L"Room - Fire - Functionality - Oxygen");
		context->DrawText(buffer, 37, c, D2D1::RectF((float)position.first + 5, (float)position.second + 17, (float)position.first + sizeX, (float)position.second + 20), brush);
		
		for (int i = 0; i < fires.size(); i++)
		{
			swprintf(buffer,100, L"%s  %03d.%d%% %s %03d%%", DCS::enumToString(fires[i]->whatType()), (fires[i]->fireValue()*100)/MAX_FIRE_VALUE, (fires[i]->fireValue()) % (MAX_FIRE_VALUE/100), enumToString(fires[i]->currentState().second),fires[i]->currentOxygenLevel());
			std::wstring t(buffer);
			context->DrawText(buffer, (UINT32)t.length(), c, D2D1::RectF((float)position.first+5, (float)position.second + (i + 2) * 15, (float)position.first + sizeX, (float)position.second + (i + 2) * 15 + 15), brush);
		}
		delete[]buffer;
		brush->Release();
		t->Release();
		c->Release();
	}
}

DCS::Dx11Engine::FireManager::FireManager(Point position)
{
	this->position = position;
}

void DCS::Dx11Engine::FireManager::add(Room * r)
{
	for (auto i = fires.begin(); i != fires.end(); i++)
		if (*i == r)
			return;
	fires.push_back(r);
}

void DCS::Dx11Engine::FireManager::remove(Room * r)
{
	int k = (int)fires.size();
	for (auto i = fires.begin(); i != fires.end(); i++)
		if (*i == r)
		{
			fires.erase(i);
			if (k == 1)
				remainCount = 120;
			return;
		}
}

wchar_t * DCS::enumToString(Room::RoomType t)
{
	switch (t)
	{
	case DCS::Room::Bridge:
		return L"Bridge";
	case DCS::Room::Engineering:
		return L"Engineering";
	case DCS::Room::Corridor:
		return L"Corridor";
	case DCS::Room::LifeSupport:
		return  L"Life Support";
	default:
		throw std::runtime_error("Error in enum to string");
	}
}

wchar_t * DCS::enumToString(DamageState t)
{
	switch (t)
	{
	case Operational:
		return L"Operational";
	case Damaged:
		return L"Damaged";
	case OutOfAction:
		return L"Out of action";
	case Destroyed:
		return L"Destroyed";
	default:
		throw std::runtime_error("Error in enum to string");

	}
}

DCS::Dx11Engine::EscMenu::EscMenu(Point p)
{
	position = p;
	sizeX = 300;
	sizeY = 700;
	buttons.push_back(Button(Point(100, 50), Resume));
	buttons.push_back(Button(Point(100, 100), Settings));
	buttons.push_back(Button(Point(100, 150), Exit));

}

DCS::Dx11Engine::EscMenu::PressResult DCS::Dx11Engine::EscMenu::OnPointerPressed(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args)
{
	for each (auto var in buttons)
	{
		switch (var.OnPointerPressed(sender, args, position))
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
	context->FillRectangle(D2D1::RectF((float)position.first, (float)position.second, (float)position.first+sizeX, (float)position.second+sizeY),brush);
	for each (auto var in buttons)
	{
		var.render(context,position);
	}


	brush->Release();
}

DCS::Dx11Engine::EscMenu::Button::Button(Point p, EscMenuButton t)
{
	position = p;
	sizeX = 100;
	sizeY = 20;
	type = t;
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

	background->Release();
	text->Release();
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
	case Null:
		throw std::runtime_error("enum to char error-was NULL");
	}
}


void DCS::Dx11Engine::EscMenu::Button::render(ID2D1DeviceContext * context, Point offset) const
{
	Point tmp = position + offset;
	ID2D1SolidColorBrush *background;
	ID2D1SolidColorBrush *text;
	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], 1.0f), &text);
	context->CreateSolidColorBrush(D2D1::ColorF(color[0], color[1], color[2], 1.0f), &background);
	context->FillRectangle(D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + sizeX, (float)tmp.second + sizeY), background);
	
	
	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&t));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
	wchar_t *buffer = new wchar_t[100];
	swprintf(buffer,100,enumToChar(type));
	std::wstring k(buffer);
	context->DrawText(buffer,(UINT32) k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + sizeX, (float)tmp.second +sizeY), text);

	background->Release();
	text->Release();

}

DCS::Dx11Engine::EscMenu::EscMenuButton DCS::Dx11Engine::EscMenu::Button::OnPointerPressed(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::PointerEventArgs ^ args, Point offset)
{
	DCS::Point tmp = DCS::Point(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y) - offset;
	if (tmp.first > position.first&&tmp.second > position.second&&tmp.first < position.first + sizeX&&tmp.second < position.second + sizeY)
		return type;
	else
		return Null;
}

wchar_t * DCS::Dx11Engine::EscMenu::Button::enumToChar(EscMenuButton b)
{
	switch (b)
	{
	case Resume:
		return L"Resume";
	case Exit:
		return L"Exit";
	case Settings:
		return L"Settings";
	default:
		throw std::runtime_error("Error in enum to string");

	}
}

void DCS::Dx11Engine::ObjectiveScreen::render(ID2D1DeviceContext * context, Point offset) const
{
	Point tmp = offset;
	ID2D1SolidColorBrush *background;
	ID2D1SolidColorBrush *text;
	context->CreateSolidColorBrush(D2D1::ColorF(textColor[0], textColor[1], textColor[2], 1.0f), &text);
	context->CreateSolidColorBrush(D2D1::ColorF(color[0], color[1], color[2], 1.0f), &background);
	context->FillRectangle(D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + size.first, (float)tmp.second + size.second), background);


	IDWriteFactory* t;
	IDWriteTextFormat*c;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&t));
	t->CreateTextFormat(L"arial", NULL, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"whatever", &c);
	wchar_t *buffer = new wchar_t[100];
	swprintf(buffer, 100, L"Objectives:");
	std::wstring k(buffer);
	context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second, (float)tmp.first + size.first, (float)tmp.second + size.second), text);
	int n = objective->ticsRemaining();
	if (n!=-1)
	{
		swprintf(buffer, 100, L"Survive for: %02d:%02d", n / 60 / 60, (n / 60)%60);
		k = std::wstring(buffer);
	}
	context->DrawText(buffer, (UINT32)k.size(), c, D2D1::RectF((float)tmp.first, (float)tmp.second+20, (float)tmp.first + size.first, (float)tmp.second + size.second), text);
	background->Release();
	text->Release();

}

DCS::Dx11Engine::ObjectiveScreen::ObjectiveScreen(DCS::Scenario* obj)
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

}

DCS::Dx11Engine::MainMenu::MainMenu()
{
	for ( MainMenuButton i = NewGame; i < Null; i = (MainMenuButton)( i + 1 ) )
		buttons.push_back(MainButton(Point(50, 50 + 50 * i), i));
	position = Point(600, 400);

}
