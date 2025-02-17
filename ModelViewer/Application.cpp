#include "Application.h"

#include <iostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include "MyMacros.h"

Application::Application()
{
	m_Graphics = 0;
	m_Shader = 0;
	m_Camera = 0;
	m_Light = 0;
	m_Model = 0;

	m_ModelPos   = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	m_ModelRot   = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	m_ModelScale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);

	m_LastUpdate = std::chrono::steady_clock::now();
	m_AppTime = 0.0;
}

Application::Application(const Application& Other)
{
}

Application::~Application()
{
}

bool Application::Initialise(int ScreenWidth, int ScreenHeight, HWND hWnd)
{
	m_hWnd = hWnd;
	
	m_Graphics = new Graphics();
	bool Result = m_Graphics->Initialise(ScreenWidth, ScreenHeight, VSYNC_ENABLED, hWnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise Graphics object!", L"Error", MB_OK);
		return false;
	}

	m_Camera = new Camera();
	m_Camera->SetPosition(0.f, 1.f, -2.f);
	m_Camera->SetRotation(0.f, 0.f, 0.f);

	m_Shader = new Shader();
	Result = m_Shader->Initialise(m_Graphics->GetDevice(), hWnd);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise Shader object!", L"Error", MB_OK);
		return false;
	}

	char ModelFilename[128];
	strcpy_s(ModelFilename, "Models/sphere.obj");

	m_SceneLight = new Model();
	Result = m_SceneLight->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), ModelFilename);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise scene light object!", L"Error", MB_OK);
		return false;
	}

	strcpy_s(ModelFilename, "Models/plane.obj");

	m_Plane = new Model();
	Result = m_Plane->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), ModelFilename);
	if (!Result)
	{
		ShowCursor(true);
		MessageBox(hWnd, L"Failed to initialise plane object!", L"Error", MB_OK);
		return false;
	}

	m_Light = new Light();
	m_Light->SetPosition(1.f, 1.f, -0.5f);
	m_Light->SetSpecularPower(32.f);
	m_Light->SetRadius(5.f);
	m_Light->SetDiffuseColor(1.f, 1.f, 1.f);

	//m_PostProcesses.emplace_back(std::make_unique<PostProcessFog>());
	//m_PostProcesses.emplace_back(std::make_unique<PostProcessBlur>(5));

	m_PostProcesses.emplace_back(std::make_unique<PostProcessEmpty>(m_Graphics->GetDevice())); // testing if the post process chain correctly swaps render targets and shader resources
	m_PostProcesses.emplace_back(std::make_unique<PostProcessEmpty>(m_Graphics->GetDevice())); // testing if the post process chain correctly swaps render targets and shader resources
	m_PostProcesses.emplace_back(std::make_unique<PostProcessEmpty>(m_Graphics->GetDevice())); // testing if the post process chain correctly swaps render targets and shader resources
	m_PostProcesses.emplace_back(std::make_unique<PostProcessEmpty>(m_Graphics->GetDevice())); // make sure this is always last!
	
	return true;
}

void Application::Shutdown()
{
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}

	if (m_Shader)
	{
		m_Shader->Shutdown();
		delete m_Shader;
		m_Shader = 0;
	}

	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
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
	
	bool Result = Render(DeltaTime);
	if (!Result)
	{
		return false;
	}
	
	return true;
}

bool Application::LoadModel(const char* ModelFilename)
{
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
	}
	
	bool Result;
	char Filename[128];
	
	strcpy_s(Filename, ModelFilename);

	m_Model = new Model();
	FALSE_IF_FAILED(m_Model->Initialise(m_Graphics->GetDevice(), m_Graphics->GetDeviceContext(), Filename));
		
	return true;
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

	m_Graphics->GetDeviceContext()->PSSetShaderResources(0u, 1u, NullSRVs);
	m_Graphics->GetDeviceContext()->OMSetRenderTargets(1u, CurrentRTV.GetAddressOf(), m_Graphics->GetDepthStencilView());
	m_Graphics->EnableDepthWrite(); // simpler for now but might need to refactor when wanting to use depth data, enables depth test and writing to depth buffer

	FALSE_IF_FAILED(RenderScene());
	
	// apply post processes (if any) and keep track of which shader resource view is the latest
	DrawingForward = !DrawingForward;
	unsigned int Stride, Offset;
	Stride = sizeof(Vertex);
	Offset = 0u;
	m_Graphics->GetDeviceContext()->IASetVertexBuffers(0u, 1u, PostProcess::GetQuadVertexBuffer(m_Graphics->GetDevice()).GetAddressOf(), &Stride, &Offset);
	m_Graphics->GetDeviceContext()->IASetIndexBuffer(PostProcess::GetQuadIndexBuffer(m_Graphics->GetDevice()).Get(), DXGI_FORMAT_R32_UINT, 0u);
	m_Graphics->GetDeviceContext()->VSSetShader(PostProcess::GetQuadVertexShader(m_Graphics->GetDevice()).Get(), nullptr, 0u);
	ApplyPostProcesses(CurrentRTV, SecondaryRTV, CurrentSRV, SecondarySRV, DrawingForward);

	// set back buffer as render target
	m_Graphics->SetBackBufferRenderTarget();

	// use last used post process texture to draw a full screen quad
	m_Graphics->GetDeviceContext()->PSSetShader(m_PostProcesses[m_PostProcesses.size() - 1].get()->m_PixelShader.Get(), NULL, 0u); // last post process effect will be empty post process
	m_Graphics->GetDeviceContext()->PSSetShaderResources(0u, 1u, DrawingForward ? CurrentSRV.GetAddressOf() : SecondarySRV.GetAddressOf());
	m_Graphics->GetDeviceContext()->DrawIndexed(6u, 0u, 0);

	RenderImGui();

	m_Graphics->EndScene();

	return true;
}

bool Application::RenderScene()
{
	m_Graphics->BeginScene(0.5f, 0.8f, 1.f, 1.f);
	
	float RotationAngle = fmod(m_AppTime, 360.f);

	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;
	
	m_Camera->Render();

	if (m_ShouldRenderPlane && m_Plane)
	{
		RenderPlane();
	}
	if (m_ShouldRenderLight && m_Light)
	{
		RenderPhysicalLight();
	}

	if (m_Model)
	{
		m_Graphics->GetWorldMatrix(WorldMatrix);
		WorldMatrix *= DirectX::XMMatrixRotationY(RotationAngle);
		WorldMatrix *= DirectX::XMMatrixScaling(m_ModelScale.x, m_ModelScale.y, m_ModelScale.z);
		WorldMatrix *= DirectX::XMMatrixTranslation(m_ModelPos.x, m_ModelPos.y, m_ModelPos.z);
		m_Camera->GetViewMatrix(ViewMatrix);
		m_Graphics->GetProjectionMatrix(ProjectionMatrix);

		m_Model->Render(m_Graphics->GetDeviceContext());

		FALSE_IF_FAILED(
			m_Shader->Render(
				m_Graphics->GetDeviceContext(),
				m_Model->GetIndexCount(),
				WorldMatrix,
				ViewMatrix,
				ProjectionMatrix,
				m_Camera->GetPosition(),
				m_Light->GetRadius(),
				m_Light->GetPosition(),
				m_Light->GetDiffuseColor(),
				m_Light->GetSpecularPower()
			)
		);
	}
	
	return true;
}

bool Application::RenderPhysicalLight()
{
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->GetWorldMatrix(WorldMatrix);
	auto LightPos = m_Light->GetPosition();
	WorldMatrix *= DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f);
	WorldMatrix *= DirectX::XMMatrixTranslation(LightPos.x, LightPos.y, LightPos.z);
	m_Camera->GetViewMatrix(ViewMatrix);
	m_Graphics->GetProjectionMatrix(ProjectionMatrix);

	m_SceneLight->Render(m_Graphics->GetDeviceContext());

	FALSE_IF_FAILED(
		m_Shader->Render(
			m_Graphics->GetDeviceContext(),
			m_SceneLight->GetIndexCount(),
			WorldMatrix,
			ViewMatrix,
			ProjectionMatrix,
			m_Camera->GetPosition(),
			0.f,
			m_Light->GetPosition(),
			m_Light->GetDiffuseColor(),
			m_Light->GetSpecularPower()
		)
	);
	
	return true;
}

bool Application::RenderPlane()
{
	DirectX::XMMATRIX WorldMatrix, ViewMatrix, ProjectionMatrix;
	bool Result;

	m_Graphics->GetWorldMatrix(WorldMatrix);
	WorldMatrix *= DirectX::XMMatrixScaling(20.f, 1.f, 20.f);
	m_Camera->GetViewMatrix(ViewMatrix);
	m_Graphics->GetProjectionMatrix(ProjectionMatrix);

	m_Plane->Render(m_Graphics->GetDeviceContext());

	FALSE_IF_FAILED(
		m_Shader->Render(
			m_Graphics->GetDeviceContext(),
			m_Plane->GetIndexCount(),
			WorldMatrix,
			ViewMatrix,
			ProjectionMatrix,
			m_Camera->GetPosition(),
			m_Light->GetRadius(),
			m_Light->GetPosition(),
			m_Light->GetDiffuseColor(),
			m_Light->GetSpecularPower()
		)
	);
	
	return true;
}

void Application::RenderImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ShowCursor(true);
	static bool ShowDemoWindow = false;
	if (ShowDemoWindow)
	{
		ImGui::ShowDemoWindow(&ShowDemoWindow);
	}

	assert(m_Light || "Must have a light in the scene before spawning light window!");
	if (ImGui::Begin("Light"))
	{
		ImGui::SliderFloat("X", reinterpret_cast<float*>(m_Light->GetPositionPtr()) + 0, -10.f, 10.f);
		ImGui::SliderFloat("Y", reinterpret_cast<float*>(m_Light->GetPositionPtr()) + 1, -10.f, 10.f);
		ImGui::SliderFloat("Z", reinterpret_cast<float*>(m_Light->GetPositionPtr()) + 2, -10.f, 10.f);

		ImGui::Dummy(ImVec2(0.f, 10.f));

		ImGui::Checkbox("Render scene light?", &m_ShouldRenderLight);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
	ImGui::End();

	if (ImGui::Begin("Model"))
	{
		ImGui::PushID(0);
		ImGui::Text("Position");
		ImGui::SliderFloat("X", reinterpret_cast<float*>(&m_ModelPos) + 0, -10.f, 10.f);
		ImGui::SliderFloat("Y", reinterpret_cast<float*>(&m_ModelPos) + 1, -10.f, 10.f);
		ImGui::SliderFloat("Z", reinterpret_cast<float*>(&m_ModelPos) + 2, -10.f, 10.f);
		ImGui::PopID();

		ImGui::Dummy(ImVec2(0.f, 2.f));

		ImGui::PushID(1);
		ImGui::Text("Scale");
		ImGui::SliderFloat("XYZ", reinterpret_cast<float*>(&m_ModelScale), 0.f, 5.f);
		m_ModelScale.y = m_ModelScale.x;
		m_ModelScale.z = m_ModelScale.x;
		ImGui::PopID();

		ImGui::Dummy(ImVec2(0.f, 10.f));

		ImGui::Checkbox("Render plane?", &m_ShouldRenderPlane);

		ImGui::Dummy(ImVec2(0.f, 20.f));

		if (ImGui::Button("Restore Defaults"))
		{
			m_ModelPos = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
			m_ModelScale = DirectX::XMFLOAT3(1.f, 1.f, 1.f);
		}
	}
	ImGui::End();

	static char ModelLocationBuffer[1024];
	if (ImGui::Begin("Load Models"))
	{
		if (ImGui::Button("Load Stanford Bunny"))
		{
			strcpy_s(ModelLocationBuffer, "Models/stanford-bunny.obj");
			if (LoadModel(ModelLocationBuffer))
			{
				m_ModelLoadSuccessMessage = "Loaded model successfully!";
			}
			else
			{
				m_ModelLoadSuccessMessage = "Failed to load model!";
			}
		}

		if (ImGui::Button("Load Suzanne"))
		{
			strcpy_s(ModelLocationBuffer, "Models/suzanne.obj");
			if (LoadModel(ModelLocationBuffer))
			{
				m_ModelLoadSuccessMessage = "Loaded model successfully!";
			}
			else
			{
				m_ModelLoadSuccessMessage = "Failed to load model!";
			}
		}

		ImGui::Dummy(ImVec2(0.f, 20.f));

		ImGui::InputText("Model file location", ModelLocationBuffer, sizeof(ModelLocationBuffer));
		if (ImGui::Button("Load model from file"))
		{
			if (LoadModel(ModelLocationBuffer))
			{
				m_ModelLoadSuccessMessage = "Loaded model successfully!";
			}
			else
			{
				m_ModelLoadSuccessMessage = "Failed to load model!";
			}
		}
		ImGui::Text(m_ModelLoadSuccessMessage);
	}
	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Application::ApplyPostProcesses(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> CurrentRTV, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> SecondaryRTV,
										Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CurrentSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondarySRV, bool& DrawingForward)
{
	m_Graphics->DisableDepthWrite(); // simpler for now but might need to refactor when wanting to use depth data in post processes
	
	// skipping last post process as that is the empty post process used to draw to back buffer render target
	for (int i = 0; i < m_PostProcesses.size() - 1; i++)
	{
		ID3D11RenderTargetView* NullRTVs[] = { nullptr };
		m_Graphics->GetDeviceContext()->OMSetRenderTargets(1u, NullRTVs, nullptr); // can't bind the texture as a shader resource while it is still the render target
		m_Graphics->GetDeviceContext()->OMSetDepthStencilState(m_PostProcesses[i]->GetDepthStencilState(m_Graphics->GetDevice()).Get(), 0); // binds a depth stencil state which always passes depth test

		m_PostProcesses[i]->ApplyPostProcess(m_Graphics->GetDeviceContext(), DrawingForward ? SecondaryRTV : CurrentRTV,
												DrawingForward ? CurrentSRV : SecondarySRV, m_Graphics->GetDepthStencilView());

		DrawingForward = !DrawingForward;
	}
}
