#include "Node.h"
#include "Model.h"
#include "Application.h"
#include "MyMacros.h"

Node::Node(Model* pModel, Node* pOwner) : m_pModel(pModel), m_pOwner(pOwner)
{
}

void Node::ProcessNode(aiNode* ModelNode, const aiScene* Scene, const DirectX::XMMATRIX& AccumulatedTransform)
{
	m_NodeName = ModelNode->mName.C_Str();
	
	m_LocalTransform = ConvertToXMMATRIX(ModelNode->mTransformation);
	m_AccumulatedTransform = AccumulatedTransform * m_LocalTransform;
	CreateConstantBuffer();
	
	for (size_t i = 0; i < ModelNode->mNumMeshes; i++)
	{
		m_Meshes.emplace_back(std::make_unique<Mesh>(m_pModel, this));
		aiMesh* SceneMesh = Scene->mMeshes[ModelNode->mMeshes[i]];
		m_Meshes.back()->ProcessMesh(SceneMesh);
	}

	for (size_t i = 0; i < ModelNode->mNumChildren; i++)
	{
		m_Children.emplace_back(std::make_unique<Node>(m_pModel, this));
		m_Children.back().get()->ProcessNode(ModelNode->mChildren[i], Scene, m_AccumulatedTransform);
	}
}

void Node::RenderMeshes()
{
	ID3D11DeviceContext* DeviceContext = Application::GetSingletonPtr()->GetGraphics()->GetDeviceContext();
	UINT Stride = sizeof(Vertex);
	UINT Offset = 0u;

	DeviceContext->IASetVertexBuffers(0u, 1u, m_pModel->GetVertexBuffer().GetAddressOf(), &Stride, &Offset);
	DeviceContext->IASetIndexBuffer(m_pModel->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0u);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->VSSetConstantBuffers(1u, 1u, m_ConstantBuffer.GetAddressOf());

	for (const std::unique_ptr<Mesh>& m : m_Meshes)
	{
		std::shared_ptr<Material> Mat = m.get()->m_Material;

		DeviceContext->PSSetConstantBuffers(1u, 1u, Mat->m_ConstantBuffer.GetAddressOf());

		if (Mat->m_DiffuseSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(0u, 1u, m_pModel->GetTextures()[Mat->m_DiffuseSRV].GetAddressOf());
		}

		if (Mat->m_SpecularSRV >= 0)
		{
			DeviceContext->PSSetShaderResources(1u, 1u, m_pModel->GetTextures()[Mat->m_SpecularSRV].GetAddressOf());
		}

		//Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(!Mat->m_bTwoSided); // this doesn't actually work in some cases, investigate
		Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(true);

		DeviceContext->DrawIndexed(m->m_IndexCount, m->m_IndicesOffset, 0);
	}

	for (const std::unique_ptr<Node>& ChildNode : m_Children)
	{
		ChildNode->RenderMeshes();
	}
}

DirectX::XMMATRIX Node::ConvertToXMMATRIX(const aiMatrix4x4& aiMatrix) const
{
	return DirectX::XMMATRIX(
		aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
		aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
		aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
		aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4
	);
}

void Node::CreateConstantBuffer()
{
	HRESULT hResult;
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX);
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA Data;
	Data.pSysMem = &m_AccumulatedTransform;

	ASSERT_NOT_FAILED(Application::GetSingletonPtr()->GetGraphics()->GetDevice()->CreateBuffer(&BufferDesc, &Data, &m_ConstantBuffer));
}
