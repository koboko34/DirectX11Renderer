#include "Node.h"
#include "Model.h"
#include "Application.h"

Node::Node(Model* pModel, Node* pOwner) : m_pModel(pModel), m_pOwner(pOwner)
{
}

void Node::ProcessNode(aiNode* ModelNode, const aiScene* Scene)
{
	for (size_t i = 0; i < ModelNode->mNumMeshes; i++)
	{
		m_Meshes.emplace_back(std::make_unique<Mesh>(m_pModel, this));
		UINT MeshIndex = (UINT)m_Meshes.size() - 1;

		aiMesh* SceneMesh = Scene->mMeshes[ModelNode->mMeshes[i]];
		m_Meshes[MeshIndex]->ProcessMesh(SceneMesh);
	}

	for (size_t i = 0; i < ModelNode->mNumChildren; i++)
	{
		m_Children.emplace_back(std::make_unique<Node>(m_pModel, this));
		UINT Index = (UINT)(m_Children.size() - 1);
		m_Children[Index].get()->ProcessNode(ModelNode->mChildren[i], Scene);
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

		Application::GetSingletonPtr()->GetGraphics()->SetRasterStateBackFaceCull(!Mat->m_bTwoSided);

		DeviceContext->DrawIndexed(m->m_IndexCount, m->m_IndicesOffset, 0);
	}

	for (const std::unique_ptr<Node>& ChildNode : m_Children)
	{
		ChildNode->RenderMeshes();
	}
}