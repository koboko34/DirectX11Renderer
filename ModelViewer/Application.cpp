#include "Application.h"

#include <iostream>

#include "Windows.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include "MyMacros.h"
#include "Common.h"
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
#include "Resource.h"
#include "ModelData.h"
#include "Landscape.h"
#include "BoxRenderer.h"
#include "FrustumCuller.h"
#include "TessellatedPlane.h"

Application* Application::m_Instance = nullptr;

Application::Application()
{
	m_LastUpdate = std::chrono::steady_clock::now();
	m_AppTime = 0.0;
	m_hWnd = {};
	m_TextureResourceView = {};
	m_ActiveCameraID = 0;
	m_CameraSpeed = 20.f;
	m_CameraSpeedMin = 10.f;
	m_CameraSpeedMax = 200.f;
}

bool Application::Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd)
{
	m_hWnd = hWnd;
	bool bResult;

	m_Graphics = Graphics::GetSingletonPtr();
	bResult = m_Graphics->Initialise(ScreenWidth, ScreenHeight, VSYNC_ENABLED, hWnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	assert(bResult);

	m_Shader = std::make_unique<Shader>();
	bResult = m_Shader->Initialise(m_Graphics->GetDevice());
	assert(bResult);

	m_InstancedShader = std::make_unique<InstancedShader>();
	bResult = m_InstancedShader->Initialise(m_Graphics->GetDevice());
	assert(bResult);

	m_FrustumCuller = std::make_shared<FrustumCuller>();
	bResult = m_FrustumCuller->Init();
	assert(bResult);

	m_BoxRenderer = std::make_unique<BoxRenderer>();
	bResult = m_BoxRenderer->Init();
	assert(bResult);

	m_Skybox = std::make_unique<Skybox>();
	bResult = m_Skybox->Init();
	assert(bResult);

	m_Landscape = std::make_shared<Landscape>(32u, 25.f, 20.f);
	m_GameObjects.push_back(m_Landscape);
	bResult = m_Landscape->Init("Textures/perlin_noise.png", 256.f);
	assert(bResult);

	m_Cameras.emplace_back(std::make_shared<Camera>(m_Graphics->GetProjectionMatrix()));
	m_Cameras.back()->SetPosition(0.f, 2.5f, -7.f);
	m_Cameras.back()->SetName("Main Camera");
	m_ActiveCamera = m_Cameras.back();
	m_MainCamera = m_Cameras.back();
	m_GameObjects.push_back(m_ActiveCamera);

	m_Cameras.emplace_back(std::make_shared<Camera>(m_Graphics->GetProjectionMatrix()));
	m_GameObjects.push_back(m_Cameras.back());
	m_Cameras.emplace_back(std::make_shared<Camera>(m_Graphics->GetProjectionMatrix()));
	m_GameObjects.push_back(m_Cameras.back());

	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(0.f, 0.f, 0.f);
	m_GameObjects.back()->SetName("Car_1");
	m_GameObjects.back()->AddComponent(std::make_shared<Model>("Models/american_fullsize_73/scene.gltf", "Models/american_fullsize_73/"));

	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(1.f, 1.f, 0.f);
	m_GameObjects.back()->SetName("Car_2");
	m_GameObjects.back()->AddComponent(std::make_shared<Model>("Models/american_fullsize_73/scene.gltf", "Models/american_fullsize_73/"));

	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(0.f, 0.f, 0.f);
	m_GameObjects.back()->AddComponent(std::make_shared<Model>("Models/fantasy_sword_stylized/scene.gltf", "Models/fantasy_sword_stylized/"));

	/*m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetPosition(1.7f, 2.5f, -1.7f);
	m_GameObjects.back()->SetScale(0.1f, 0.1f, 0.1f);
	m_GameObjects.back()->AddComponent(std::make_shared<Model>("Models/sphere.obj"));
	m_GameObjects.back()->AddComponent(std::make_shared<PointLight>());*/

	/*m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetName("Point Light");
	m_GameObjects.back()->SetPosition(-2.f, 3.f, 0.f);
	m_GameObjects.back()->SetScale(0.1f);
	m_GameObjects.back()->AddComponent(std::make_shared<Model>("Models/sphere.obj"));
	m_GameObjects.back()->AddComponent(std::make_shared<PointLight>());*/

	m_GameObjects.emplace_back(std::make_shared<GameObject>());
	m_GameObjects.back()->SetName("Directional Light");
	m_GameObjects.back()->AddComponent(std::make_shared<DirectionalLight>());

	m_TextureResourceView = static_cast<ID3D11ShaderResourceView*>(ResourceManager::GetSingletonPtr()->LoadTexture(m_QuadTexturePath));

	m_PostProcesses.emplace_back(std::make_unique<PostProcessFog>(0.3f, 0.3f, 0.3f, 0.002f, PostProcessFog::FogFormula::ExponentialSquared));
	m_PostProcesses.back()->Deactivate();
	m_PostProcesses.emplace_back(std::make_unique<PostProcessPixelation>(8.f));
	m_PostProcesses.back()->Deactivate();
	m_PostProcesses.emplace_back(std::make_unique<PostProcessBoxBlur>(30));
	m_PostProcesses.back()->Deactivate();
	m_PostProcesses.emplace_back(std::make_unique<PostProcessGaussianBlur>(30, 4.f));
	m_PostProcesses.back()->Deactivate();
	m_PostProcesses.emplace_back(std::make_unique<PostProcessBloom>(0.9f, 16, 8.f));
	m_PostProcesses.back()->Deactivate();
	m_PostProcesses.emplace_back(std::make_unique<PostProcessToneMapper>(1.5f, 1.f, 1.f, PostProcessToneMapper::ToneMapperFormula::HillACES));
	m_PostProcesses.emplace_back(std::make_unique<PostProcessColorCorrection>(1.f, 0.f, 1.15f));
	m_PostProcesses.emplace_back(std::make_unique<PostProcessGammaCorrection>(2.2f));

	return true;
}

void Application::Shutdown()
{
	ResourceManager::GetSingletonPtr()->UnloadTexture(m_QuadTexturePath);
	m_TextureResourceView = nullptr;
	
	PostProcess::ShutdownStatics();
	m_PostProcesses.clear();
	m_GameObjects.clear();

	m_Skybox.reset();
	m_InstancedShader.reset();
	m_Shader.reset();
	m_ActiveCamera.reset();
	m_Landscape.reset();
	m_FrustumCuller.reset();
	m_BoxRenderer.reset();

	ResourceManager::GetSingletonPtr()->Shutdown();

	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		m_Graphics = nullptr;
	}
}

bool Application::Frame()
{	
	auto Now = std::chrono::steady_clock::now();
	m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(Now - m_LastUpdate).count() / 1000000.0; // in seconds
	m_LastUpdate = Now;
	m_AppTime += m_DeltaTime;

	ClearRenderStats();
	m_RenderStats.FrameTime = m_DeltaTime * 1000.0;
	m_RenderStats.FPS = 1.0 / m_DeltaTime;

	float RotationAngle = (float)fmod(m_AppTime, 360.f);
	//m_GameObjects[1]->SetRotation(0.f, RotationAngle * 30.f, 0.f);
	//m_GameObjects[2]->SetRotation(0.f, -RotationAngle * 20.f, 0.f);

	if (GetForegroundWindow() == m_hWnd)
	{
		ProcessInput();
	}
	
	bool Result = Render();
	if (!Result)
	{
		return false;
	}
	
	return true;
}

void Application::SetActiveCamera(int ID)
{
	m_ActiveCameraID = ID;
	m_ActiveCamera = m_Cameras[ID];
}

bool Application::Render()
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

	FALSE_IF_FAILED(RenderScene());
	//FALSE_IF_FAILED(RenderTexture(m_TextureResourceView));
	
	// apply post processes (if any) and keep track of which shader resource view is the latest
	DrawingForward = !DrawingForward;
	unsigned int Stride, Offset;
	Stride = sizeof(Vertex);
	Offset = 0u;
	m_Graphics->GetDeviceContext()->IASetVertexBuffers(0u, 1u, PostProcess::GetQuadVertexBuffer().GetAddressOf(), &Stride, &Offset);
	m_Graphics->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Graphics->GetDeviceContext()->IASetInputLayout(PostProcess::GetQuadInputLayout().Get());
	m_Graphics->GetDeviceContext()->IASetIndexBuffer(PostProcess::GetQuadIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0u);
	m_Graphics->GetDeviceContext()->VSSetShader(PostProcess::GetQuadVertexShader(), nullptr, 0u);
	m_Graphics->DisableDepthWriteAlwaysPass(); // simpler for now but might need to refactor when wanting to use depth data in post processes
	ApplyPostProcesses(CurrentRTV, SecondaryRTV, CurrentSRV, SecondarySRV, DrawingForward);

	// set back buffer as render target
	m_Graphics->SetBackBufferRenderTarget();

	// use last used post process texture to draw a full screen quad
	m_Graphics->SetRasterStateBackFaceCull(true);
	m_Graphics->GetDeviceContext()->PSSetShader(PostProcess::GetEmptyPostProcess()->GetPixelShader().Get(), NULL, 0u);
	m_Graphics->GetDeviceContext()->PSSetShaderResources(0u, 1u, DrawingForward ? CurrentSRV.GetAddressOf() : SecondarySRV.GetAddressOf());
	m_Graphics->GetDeviceContext()->DrawIndexed(6u, 0u, 0);
	m_RenderStats.DrawCalls++;

	for (const std::shared_ptr<Camera>& c : m_Cameras)
	{
		if (c->ShouldVisualiseFrustum() && c.get() != m_ActiveCamera.get())
		{
			m_BoxRenderer->RenderFrustum(c);
		}
	}

	if (m_bShowBoundingBoxes)
	{
		// TODO: refactor this to also use the culled transforms
		std::unordered_map<std::string, std::unique_ptr<Resource>>& Models = ResourceManager::GetSingletonPtr()->GetModelsMap();
		for (const auto& ModelPair : Models)
		{
			ModelData* pModelData = static_cast<ModelData*>(ModelPair.second->GetDataPtr());
			if (!pModelData)
				continue;

			for (const auto& t : pModelData->GetTransforms())
			{
				m_BoxRenderer->RenderBox(pModelData->GetBoundingBox(), DirectX::XMMatrixTranspose(t)); // back to column major
			}
		}

		for (const auto& t : m_Landscape->GetChunkTransforms())
		{
			m_BoxRenderer->RenderBox(m_Landscape->GetPlane()->GetBoundingBox(), DirectX::XMMatrixTranspose(t)); // back to column major
		}
	}

	if (m_bShowCursor)
		RenderImGui();

	m_Graphics->EndScene();

	return true;
}

bool Application::RenderScene()
{
	for (const std::shared_ptr<Camera>& c : m_Cameras)
	{
		c->CalcViewMatrix();
	}
	
	if (m_Skybox.get())
	{
		m_Skybox->Render(); // this should probably be rendered last to reduce overdraw
	}

	m_Graphics->EnableDepthWrite();
	m_Graphics->GetDeviceContext()->OMSetRenderTargets(1u, m_Graphics->m_PostProcessRTVFirst.GetAddressOf(), m_Graphics->GetDepthStencilView());

	RenderModels();

	if (m_Landscape.get() && m_Landscape->ShouldRender())
	{
		m_Landscape->Render();
	}
	
	return true;
}

void Application::RenderModels()
{	
	DirectX::XMMATRIX View, Proj, ViewProj;
	m_ActiveCamera->GetViewMatrix(View);
	m_ActiveCamera->GetProjMatrix(Proj);
	m_ActiveCamera->GetViewProjMatrix(ViewProj);

	std::unordered_map<std::string, std::unique_ptr<Resource>>& Models = ResourceManager::GetSingletonPtr()->GetModelsMap();

	for (const auto& ModelPair : Models)
	{
		ModelData* pModelData = static_cast<ModelData*>(ModelPair.second->GetDataPtr());
		if (!pModelData)
			continue;

		pModelData->GetTransforms().clear();
	}

	std::vector<PointLight*> PointLights;
	std::vector<DirectionalLight*> DirLights;
	for (auto& Object : m_GameObjects)
	{
		Object->SendTransformToModels();
		for (auto& Comp : Object->GetComponents())
		{
			Light* pLight = dynamic_cast<Light*>(Comp.get());
			if (pLight && pLight->IsActive())
			{
				PointLight* pPointLight = dynamic_cast<PointLight*>(pLight);
				if (pPointLight)
				{
					PointLights.push_back(pPointLight);
					continue;
				}

				DirectionalLight* pDirLight = dynamic_cast<DirectionalLight*>(pLight);
				if (pDirLight)
				{
					DirLights.push_back(pDirLight);
					continue;
				}
			}
		}
	}
	
	for (const auto& ModelPair : Models)
	{		
		ModelData* pModelData = static_cast<ModelData*>(ModelPair.second->GetDataPtr());
		if (!pModelData)
			continue;
		
		// AABB frustum culling on transforms
		m_FrustumCuller->DispatchShader(pModelData->GetTransforms(), pModelData->GetBoundingBox().Corners, m_MainCamera->GetViewProjMatrix());
		
		UINT InstanceCount = m_FrustumCuller->GetInstanceCount();
		if (InstanceCount == 0)
			continue;

		m_RenderStats.InstancesRendered.push_back(std::make_pair(pModelData->GetModelPath(), InstanceCount));
		
		m_InstancedShader->ActivateShader(m_Graphics->GetDeviceContext());
		m_InstancedShader->SetShaderParameters(
			m_Graphics->GetDeviceContext(),
			View,
			Proj,
			m_ActiveCamera->GetPosition(),
			PointLights,
			DirLights,
			m_Skybox->GetAverageSkyColor()
		);

		pModelData->Render();
	}
}

bool Application::RenderTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureView)
{
	unsigned int Stride, Offset;
	Stride = sizeof(Vertex);
	Offset = 0u;

	m_Graphics->GetDeviceContext()->IASetVertexBuffers(0u, 1u, PostProcess::GetQuadVertexBuffer().GetAddressOf(), &Stride, &Offset);
	m_Graphics->GetDeviceContext()->IASetIndexBuffer(PostProcess::GetQuadIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0u);
	m_Graphics->GetDeviceContext()->VSSetShader(PostProcess::GetQuadVertexShader(), nullptr, 0u);
	
	m_Graphics->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_Graphics->GetDeviceContext()->IASetInputLayout(m_Shader->GetInputLayout().Get());
	m_Graphics->GetDeviceContext()->PSSetShader(PostProcess::GetEmptyPostProcess()->GetPixelShader().Get(), nullptr, 0u);
	m_Graphics->GetDeviceContext()->PSSetShaderResources(0, 1, TextureView.GetAddressOf());

	m_Graphics->GetDeviceContext()->DrawIndexed(6u, 0u, 0);
	m_RenderStats.DrawCalls++;

	return true;
}

void Application::RenderImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiManager::RenderPostProcessWindow();
	ImGuiManager::RenderWorldHierarchyWindow();
	ImGuiManager::RenderCamerasWindow();
	ImGuiManager::RenderStatsWindow(m_RenderStats);

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Application::ApplyPostProcesses(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV,
										Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV, bool& DrawingForward)
{	
	for (int i = 0; i < m_PostProcesses.size(); i++)
	{
		if (!m_PostProcesses[i]->GetIsActive())
		{
			continue;
		}

		m_PostProcesses[i]->ApplyPostProcess(m_Graphics->GetDeviceContext(), DrawingForward ? SecondaryRTV : CurrentRTV,
												DrawingForward ? CurrentSRV : SecondarySRV);

		DrawingForward = !DrawingForward;
	}
}

void Application::ProcessInput()
{
	DirectX::XMFLOAT3 LookDir = m_ActiveCamera->GetRotatedLookDir();
	DirectX::XMFLOAT3 LookRight = m_ActiveCamera->GetRotatedLookRight();
	float EffectiveCameraSpeed = m_CameraSpeed * (float)m_DeltaTime;
	
	if (GetAsyncKeyState('W') & 0x8000)
	{
		m_ActiveCamera->SetPosition(m_ActiveCamera->GetPosition().x + LookDir.x * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().y + LookDir.y * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().z + LookDir.z * EffectiveCameraSpeed);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		m_ActiveCamera->SetPosition(m_ActiveCamera->GetPosition().x - LookDir.x * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().y - LookDir.y * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().z - LookDir.z * EffectiveCameraSpeed);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		m_ActiveCamera->SetPosition(m_ActiveCamera->GetPosition().x + LookRight.x * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().y, m_ActiveCamera->GetPosition().z + LookRight.z * EffectiveCameraSpeed);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		m_ActiveCamera->SetPosition(m_ActiveCamera->GetPosition().x - LookRight.x * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().y, m_ActiveCamera->GetPosition().z - LookRight.z * EffectiveCameraSpeed);
	}
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_ActiveCamera->SetPosition(m_ActiveCamera->GetPosition().x, m_ActiveCamera->GetPosition().y - 1.f * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().z);
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		m_ActiveCamera->SetPosition(m_ActiveCamera->GetPosition().x, m_ActiveCamera->GetPosition().y + 1.f * EffectiveCameraSpeed, m_ActiveCamera->GetPosition().z);
	}

	m_ActiveCamera->SetRotation(m_ActiveCamera->GetRotation().x + SystemClass::m_MouseDelta.y * 0.1f, m_ActiveCamera->GetRotation().y + SystemClass::m_MouseDelta.x * 0.1f, 0.f);

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

void Application::ClearRenderStats()
{
	m_RenderStats.TrianglesRendered.clear();
	m_RenderStats.InstancesRendered.clear();
	m_RenderStats = {};
}
