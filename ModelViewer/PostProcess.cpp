#include "PostProcess.h"

Microsoft::WRL::ComPtr<ID3D11VertexShader> PostProcess::ms_QuadVertexShader;
Microsoft::WRL::ComPtr<ID3D11InputLayout> PostProcess::ms_QuadInputLayout;
Microsoft::WRL::ComPtr<ID3D11Buffer> PostProcess::ms_QuadVertexBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> PostProcess::ms_QuadIndexBuffer;
std::shared_ptr<PostProcessEmpty> PostProcess::ms_EmptyPostProcess;
bool PostProcess::ms_bInitialised = false;