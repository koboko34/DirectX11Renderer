#include "PostProcess.h"

Microsoft::WRL::ComPtr<ID3D11VertexShader> PostProcess::ms_QuadVertexShader;
Microsoft::WRL::ComPtr<ID3D11InputLayout> PostProcess::ms_QuadInputLayout;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> PostProcess::ms_DepthStencilState;
Microsoft::WRL::ComPtr<ID3D11Buffer> PostProcess::ms_QuadVertexBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> PostProcess::ms_QuadIndexBuffer;
bool PostProcess::ms_bInitialised = false;