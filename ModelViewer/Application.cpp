#include "Application.h"

#include <iostream>

#include "Windows.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include "MyMacros.h"
#include "Camera.h"
#include "Shader.h"
#include "InstancedShader.h"
#include "Model.h"
#include "Light.h"
#include "PostProcess.h"
#include "ResourceManager.h"
#include "SystemClass.h"
#include "InputClass.h"
#include "Skybox.h"

Application* Application::m_Instance = nullptr;

Application::Application()
{
	m_LastUpdate = std::chrono::steady_clock::now();
	m_AppTime = 0.0;
}

bool Application::Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd)
{
	m_hWnd = hWnd;
	
	m_Graphics = Graphics::GetSingletonPtr();
	assert(m_Graphics->Initialise(ScreenWidth, ScreenHeight, VSYNC_ENABLED, hWnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR));

	m_Shader = std::make_unique<Shader>();
	assert(m_Shader->Initialise(m_Graphics->GetDevice(), hWnd));

	m_InstancedShader = std::make_unique<InstancedShader>();
	assert(m_InstancedShader->Initialise(m_Graphics->GetDevice(), hWnd));

	m_Skybox = std::make_unique<Skybox>();
	assert(m_Skybox->Init());

	m_Camera = std::make_unique<Camera>();
	m_Camera->SetPosition(0.f, 2.5f, -7.f);
	m_Camera->SetRotation(0.f, 0.f);

	std::shared_ptr<Model> CarModel = LoadModel("Models/american_fullsize_73/scene.gltf", "Models/american_fullsize_73/");
	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(-1.f, -1.f, 0.f);
	m_GameObjects.back()->AddComponent(CarModel);

	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(1.f, 1.f, 0.f);
	m_GameObjects.back()->AddComponent(CarModel);

	std::shared_ptr<PointLight> pPointLight = std::make_shared<PointLight>();
	pPointLight->SetSpecularPower(256.f);
	pPointLight->SetRadius(10.f);
	pPointLight->SetDiffuseColor(1.f, 1.f, 1.f);

	std::shared_ptr<Model> SceneLightModel = LoadModel("Models/sphere.obj");
	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(1.7f, 2.5f, -1.7f);
	m_GameObjects.back()->SetScale(0.1f, 0.1f, 0.1f);
	m_GameObjects.back()->AddComponent(SceneLightModel);
	m_GameObjects.back()->AddComponent(pPointLight);

	pPointLight.reset();
	pPointLight = std::make_shared<PointLight>();
	pPointLight->SetSpecularPower(256.f);
	pPointLight->SetRadius(10.f);
	pPointLight->SetDiffuseColor(1.f, 1.f, 1.f);
	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(-2.f, 3.f, 0.f);
	m_GameObjects.back()->SetScale(0.1f, 0.1f, 0.1f);
	m_GameObjects.back()->AddComponent(SceneLightModel);
	m_GameObjects.back()->AddComponent(pPointLight);

	m_TextureResourceView = reinterpret_cast<ID3D11ShaderResourceView*>(ResourceManager::GetSingletonPtr()->LoadTexture(m_QuadTexturePath));

	//m_PostProcesses.emplace_back(std::make_unique<PostProcessFog>());
	//m_PostProcesses.emplace_back(std::make_unique<PostProcessBoxBlur>(25));
	//m_PostProcesses.emplace_back(std::make_unique<PostProcessPixelation>(8.f));
	//m_PostProcesses.emplace_back(std::make_unique<PostProcessGaussianBlur>(30, 4.f));
	m_PostProcesses.emplace_back(std::make_unique<PostProcessBloom>(0.5f));
	m_PostProcesses.emplace_back(std::make_unique<PostProcessToneMapper>(1.5f, 1.f, 1.f, PostProcessToneMapper::ToneMapperFormula::HillACES));
	m_PostProcesses.emplace_back(std::make_unique<PostProcessGammaCorrection>(2.2f));

	m_EmptyPostProcess = std::make_unique<PostProcessEmpty>();

	return true;
}

void Application::Shutdown()
{
	ResourceManager::GetSingletonPtr()->UnloadTexture(m_QuadTexturePath);
	m_TextureResourceView = nullptr;
	
	for (auto& Object : m_GameObjects)
	{
		Object->Shutdown();
	}
	
	for (auto& Model : m_Models)
	{
		Model->Shutdown();
	}
	m_Models.clear();

	if (m_Skybox.get())
	{
		m_Skybox->Shutdown();
	}
	m_Skybox.reset();

	PostProcess::ShutdownStatics();
	m_PostProcesses.clear();

	if (m_InstancedShader)
	{
		m_InstancedShader->Shutdown();
		m_InstancedShader.reset();
	}

	if (m_Shader)
	{
		m_Shader->Shutdown();
		m_Shader.reset();
	}

	if (m_Camera)
	{
		m_Camera.reset();
	}

	ResourceManager::GetSingletonPtr()->Shutdown();

	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		//delete m_Graphics; // should I be calling this on a singleton?
		m_Graphics = 0;
	}
}

bool Application::Frame()
{	
	double DeltaTime;
	auto Now = std::chrono::steady_clock::now();
	DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(Now - m_LastUpdate).count() / 1000000.0;
	m_LastUpdate = Now;
	m_AppTime += DeltaTime;

	float RotationAngle = (float)fmod(m_AppTime, 360.f);
	m_GameObjects[0]->SetRotation(0.f, RotationAngle * 30.f, 0.f);
	m_GameObjects[1]->SetRotation(0.f, -RotationAngle * 20.f, 0.f);

	if (GetForegroundWindow() == m_hWnd)
	{
		ProcessInput();
	}
	
	bool Result = Render(DeltaTime);
	if (!Result)
	{
		return false;
	}
	
	return true;
}

std::shared_ptr<Model> Application::LoadModel(const char* ModelFilename, const char* TexturesPath)
{
	m_Models.emplace_back(std::make_shared<Model>());
	if (!m_Models.back()->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), ModelFilename, TexturesPath))
	{
		m_Models.back()->Shutdown();
		m_Models.pop_back();
		return std::shared_ptr<Model>();
	}
		
	return m_Models.back();
}

bool Application::Render(double DeltaTime)
{			
	bool Result;
	
	// set first post process render target here
	bool DrawingForward = false; // true if drawing to RTV2 or from SRV1, false if drawing to RTV1 or from SRV2
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV = m_Graphics->m_PostProcessRTVFirst;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV = m_Graphics->m_PostProcessRTVSecond;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV = m_Graphics->m_PostProcessSRVFirst;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV = m_Graphics->m_PostProcessSRVSecond;
	ID3D11ShaderResourceView* NullSRVs[] = { nullptr };

	m_Graphics->BeginScene(0.f, 0.f, 0.f, 1.f);

	m_Graphics->GetDeviceContext()->PSSetShaderResources(0u, 1u, NullSRVs);
	m_Graphics->GetDeviceContext()->OMSetRenderTargets(1u, CurrentRTV.GetAddressOf(), m_Graphics->GetDepthStencilView());
	m_Graphics->EnableDepthWrite(); // simpler for now but might need to refactor when wanting to use depth data, enables depth test and writing to depth buffer

	FALSE_IF_FAILED(RenderScene());
	//FALSE_IF_FAILED(RenderTexture(m_TextureResourceView));
	
	// apply post processes (if any) and keep track of which shader resource view is the latest
	DrawingForward = !DrawingForward;
	unsigned int Stride, Offset;
	Stride = sizeof(Vertex);
	Offset = 0u;
	m_Graphics->GetDeviceContext()->IASetVertexBuffers(0u, 1u, PostProcess::GetQuadVertexBuffer(m_Graphics->GetDevice()).GetAddressOf(), &Stride, &Offset);
	m_Graphics->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Graphics->GetDeviceContext()->IASetInputLayout(PostProcess::GetQuadInputLayout(m_Graphics->GetDevice()).Get());
	m_Graphics->GetDeviceContext()->IASetIndexBuffer(PostProcess::GetQuadIndexBuffer(m_Graphics->GetDevice()).Get(), DXGI_FORMAT_R32_UINT, 0u);
	m_Graphics->GetDeviceContext()->VSSetShader(PostProcess::GetQuadVertexShader(m_Graphics->GetDevice()).Get(), nullptr, 0u);
	m_Graphics->DisableDepthWriteAlwaysPass(); // simpler for now but might need to refactor when wanting to use depth data in post processes
	ApplyPostProcesses(CurrentRTV, SecondaryRTV, CurrentSRV, SecondarySRV, DrawingForward);

	// set back buffer as render target
	m_Graphics->SetBackBufferRenderTarget();

	// use last used post process texture to draw a full screen quad
	m_Graphics->GetDeviceContext()->PSSetShader(m_EmptyPostProcess->GetPixelShader().Get(), NULL, 0u);
	m_Graphics->GetDeviceContext()->PSSetShaderResources(0u, 1u, DrawingForward ? CurrentSRV.GetAddressOf() : SecondarySRV.GetAddressOf());
	m_Graphics->GetDeviceContext()->DrawIndexed(6u, 0u, 0);

	RenderImGui();

	m_Graphics->EndScene();

	return true;
}

bool Application::RenderScene()
{
	m_Camera->CalcViewMatrix();
	
	if (m_Skybox.get())
	{
		m_Skybox->Render();
	}

	RenderModels();
	
	return true;
}

void Application::RenderModels()
{
	DirectX::XMMATRIX ViewMatrix, ProjectionMatrix;
	m_Camera->GetViewMatrix(ViewMatrix);
	m_Graphics->GetProjectionMatrix(ProjectionMatrix);

	for (auto& Model : m_Models)
	{
		Model->GetTransforms().clear();
	}

	std::vector<Light*> Lights;
	for (auto& Object : m_GameObjects)
	{
		Object->SendTransformToModels();
		for (auto& Comp : Object->GetComponents())
		{
			Light* pLight = dynamic_cast<Light*>(Comp.get());
			if (pLight)
			{
				Lights.push_back(pLight);
			}
		}
	}
	
	for (const auto& pModel : m_Models)
	{		
		m_InstancedShader->ActivateShader(m_Graphics->GetDeviceContext());
		m_InstancedShader->SetShaderParameters(
			m_Graphics->GetDeviceContext(),
			pModel.get(),
			ViewMatrix,
			ProjectionMatrix,
			m_Camera->GetPosition(),
			Lights
		);

		pModel->Render();
	}
}

bool Application::RenderTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView)
{
	unsigned int Stride, Offset;
	Stride = sizeof(Vertex);
	Offset = 0u;

	m_Graphics->GetDeviceContext()->IASetVertexBuffers(0u, 1u, PostProcess::GetQuadVertexBuffer(m_Graphics->GetDevice()).GetAddressOf(), &Stride, &Offset);
	m_Graphics->GetDeviceContext()->IASetIndexBuffer(PostProcess::GetQuadIndexBuffer(m_Graphics->GetDevice()).Get(), DXGI_FORMAT_R32_UINT, 0u);
	m_Graphics->GetDeviceContext()->VSSetShader(PostProcess::GetQuadVertexShader(m_Graphics->GetDevice()).Get(), nullptr, 0u);
	
	m_Graphics->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Graphics->GetDeviceContext()->IASetInputLayout(m_Shader->GetInputLayout().Get());
	m_Graphics->GetDeviceContext()->PSSetShader(m_EmptyPostProcess->GetPixelShader().Get(), nullptr, 0u);
	m_Graphics->GetDeviceContext()->PSSetShaderResources(0, 1, TextureView.GetAddressOf());

	m_Graphics->GetDeviceContext()->DrawIndexed(6u, 0u, 0);

	return true;
}

void Application::RenderImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	/*if (m_LightObject.get())
	{
		if (ImGui::Begin("Light"))
		{
			ImGui::SliderFloat("X", reinterpret_cast<float*>(m_LightObject->GetPositionPtr()) + 0, -10.f, 10.f);
			ImGui::SliderFloat("Y", reinterpret_cast<float*>(m_LightObject->GetPositionPtr()) + 1, -10.f, 10.f);
			ImGui::SliderFloat("Z", reinterpret_cast<float*>(m_LightObject->GetPositionPtr()) + 2, -10.f, 10.f);

			ImGui::Dummy(ImVec2(0.f, 10.f));

			ImGui::Checkbox("Render scene light?", &m_bShouldRenderLight);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}*/

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Application::ApplyPostProcesses(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV,
										Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV, bool& DrawingForward)
{	
	for (int i = 0; i < m_PostProcesses.size(); i++)
	{
		m_PostProcesses[i]->ApplyPostProcess(m_Graphics->GetDeviceContext(), DrawingForward ? SecondaryRTV : CurrentRTV,
												DrawingForward ? CurrentSRV : SecondarySRV, m_Graphics->GetDepthStencilView());

		DrawingForward = !DrawingForward;
	}
}

void Application::ProcessInput()
{
	DirectX::XMFLOAT3 LookDir = m_Camera->GetRotatedLookDir();
	DirectX::XMFLOAT3 LookRight = m_Camera->GetRotatedLookRight();
	
	if (GetAsyncKeyState('W') & 0x8000)
	{
		m_Camera->SetPosition(m_Camera->GetPosition().x + LookDir.x * m_CameraSpeed, m_Camera->GetPosition().y + LookDir.y * m_CameraSpeed, m_Camera->GetPosition().z + LookDir.z * m_CameraSpeed);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		m_Camera->SetPosition(m_Camera->GetPosition().x - LookDir.x * m_CameraSpeed, m_Camera->GetPosition().y - LookDir.y * m_CameraSpeed, m_Camera->GetPosition().z - LookDir.z * m_CameraSpeed);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		m_Camera->SetPosition(m_Camera->GetPosition().x + LookRight.x * m_CameraSpeed, m_Camera->GetPosition().y, m_Camera->GetPosition().z + LookRight.z * m_CameraSpeed);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		m_Camera->SetPosition(m_Camera->GetPosition().x - LookRight.x * m_CameraSpeed, m_Camera->GetPosition().y, m_Camera->GetPosition().z - LookRight.z * m_CameraSpeed);
	}
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_Camera->SetPosition(m_Camera->GetPosition().x, m_Camera->GetPosition().y - 1.f * m_CameraSpeed, m_Camera->GetPosition().z);
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		m_Camera->SetPosition(m_Camera->GetPosition().x, m_Camera->GetPosition().y + 1.f * m_CameraSpeed, m_Camera->GetPosition().z);
	}

	m_Camera->SetRotation(m_Camera->GetRotation().x + SystemClass::m_MouseDelta.y * 0.1f, m_Camera->GetRotation().y + SystemClass::m_MouseDelta.x * 0.1f);

	if (InputClass::GetSingletonPtr()->IsKeyDown('M'))
	{
		if (m_bCursorToggleReleased)
		{
			m_bCursorToggleReleased = false;
			ToggleShowCursor();
		}
	}
	else
	{
		m_bCursorToggleReleased = true;
	}

	short Delta = InputClass::GetSingletonPtr()->GetMouseWheelDelta();
	if (Delta > 0)
	{
		m_CameraSpeed = std::fmin(m_CameraSpeed * (float)(Delta / 60), m_CameraSpeedMax);
	}
	else if (Delta < 0)
	{
		m_CameraSpeed = std::fmax(m_CameraSpeed * (1.f / (float)(Delta / 60)), m_CameraSpeedMin);
	}
}

void Application::ToggleShowCursor()
{	
	if (m_bShowCursor)
	{
		int Count;
		do {
			Count = ShowCursor(FALSE);
		} while (Count >= 0);
		SystemClass::ms_bShouldProcessMouse = true;
		SetCursorPos(SystemClass::ms_Center.x, SystemClass::ms_Center.y);
		SystemClass::m_MouseDelta = { 0.f, 0.f };
	}
	else
	{
		int Count;
		do {
			Count = ShowCursor(TRUE);
		} while (Count < 0);
		SystemClass::ms_bShouldProcessMouse = false;
		SystemClass::m_MouseDelta = { 0.f, 0.f };
	}
	m_bShowCursor = !m_bShowCursor;
}
