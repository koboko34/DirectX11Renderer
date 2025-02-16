#include "PostProcess.h"

Microsoft::WRL::ComPtr<ID3D11VertexShader> PostProcess::m_QuadVertexShader;
Microsoft::WRL::ComPtr<ID3D11InputLayout> PostProcess::m_QuadInputLayout;
Microsoft::WRL::ComPtr<ID3D11Buffer> PostProcess::m_QuadVertexBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> PostProcess::m_QuadIndexBuffer;
bool PostProcess::Initialised = false;