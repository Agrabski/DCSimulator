#include "pch.h"
#include "DCSdirectX11Main.h"
#include "Common\DirectXHelper.h"
#include "../DCSengine/DCSengine.h"

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
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);

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
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	testcontext->BeginDraw();
	game.gameRender(testcontext);
	testcontext->EndDraw();
	//m_sceneRenderer->Render();
	//m_fpsTextRenderer->Render();

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


void DCS::Dx11Engine::gameRender(ID2D1DeviceContext * context)
{
	ID2D1SolidColorBrush *brush;
	auto hr = context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
	context->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(x + 20, y + 20), brush);
	for (int i = 0; i < ship.silvete.size()-1; i++)
	{
		context->DrawLine(D2D1::Point2F((ship.silvete[i]+shipPosition).first, (ship.silvete[i]+shipPosition).second), D2D1::Point2F((ship.silvete[i+1]+shipPosition).first, (ship.silvete[i+1]+shipPosition).second), brush);
	}
	context->DrawLine(D2D1::Point2F((ship.silvete[0]+shipPosition).first, (ship.silvete[0]+shipPosition).second), D2D1::Point2F((ship.silvete[ship.silvete.size()-1]+shipPosition).first, (ship.silvete[ship.silvete.size()-1]+shipPosition).second), brush);
	for (std::vector<Room*>::iterator i = ship.rooms.begin(); i != ship.rooms.end(); i++)
		renderRoom(context, **i);
}

void DCS::Dx11Engine::renderRoom(ID2D1DeviceContext * context, Room & room)
{
	ID2D1SolidColorBrush *brush;
	auto hr = context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f), &brush);
	for (int i = 0; i < room.silvete.size() - 1; i++)
	{
		context->DrawLine(D2D1::Point2F((room.silvete[i]+shipPosition).first, (room.silvete[i]+shipPosition).second), D2D1::Point2F((room.silvete[i + 1]+shipPosition).first, (room.silvete[i + 1]+shipPosition).second), brush);
	}
	context->DrawLine(D2D1::Point2F((room.silvete[0]+shipPosition).first, (room.silvete[0]+shipPosition).second), D2D1::Point2F((room.silvete[room.silvete.size()-1]+shipPosition).first, (room.silvete[room.silvete.size()-1]+shipPosition).second), brush);

}
