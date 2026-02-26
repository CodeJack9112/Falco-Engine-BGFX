#include "API_SubMesh.h"

#include "../Core/APIManager.h"
#include "../Renderer/Color.h"

#include "../Assets/Mesh.h"

namespace GX
{
	void API_SubMesh::ctor(MonoObject* this_ptr)
	{
		SubMesh* subMesh = new SubMesh();
		subMesh->load();

		subMesh->setManagedObject(this_ptr);
	}

	MonoObject* API_SubMesh::getParent(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			Mesh* parent = subMesh->getParent();

			if (parent != nullptr)
				return parent->getManagedObject();
		}

		return nullptr;
	}

	MonoArray* API_SubMesh::getIndices(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& indexBuffer = subMesh->getIndexBuffer();
			MonoClass* type = APIManager::getSingleton()->int_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, indexBuffer.size());
			for (int i = 0; i < indexBuffer.size(); ++i)
				mono_array_set(arr, int, i, indexBuffer[i]);

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setIndices(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& indexBuffer = subMesh->getIndexBuffer();
			indexBuffer.clear();

			int size = (int)mono_array_length(value);
			indexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				int val = mono_array_get(value, int, i);
				indexBuffer[i] = val;
			}
		}
	}

	MonoArray* API_SubMesh::getVertices(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector3_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 vec;
				vec.x = vbuf.position.x;
				vec.y = vbuf.position.y;
				vec.z = vbuf.position.z;

				mono_array_set(arr, API::Vector3, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setVertices(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 val = mono_array_get(value, API::Vector3, i);
				vbuf.position.x = val.x;
				vbuf.position.y = val.y;
				vbuf.position.z = val.z;
			}
		}
	}

	MonoArray* API_SubMesh::getNormals(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector3_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 vec;
				vec.x = vbuf.normal.x;
				vec.y = vbuf.normal.y;
				vec.z = vbuf.normal.z;

				mono_array_set(arr, API::Vector3, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setNormals(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 val = mono_array_get(value, API::Vector3, i);
				vbuf.normal.x = val.x;
				vbuf.normal.y = val.y;
				vbuf.normal.z = val.z;
			}
		}
	}

	MonoArray* API_SubMesh::getTangents(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector3_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 vec;
				vec.x = vbuf.tangent.x;
				vec.y = vbuf.tangent.y;
				vec.z = vbuf.tangent.z;

				mono_array_set(arr, API::Vector3, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setTangents(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 val = mono_array_get(value, API::Vector3, i);
				vbuf.tangent.x = val.x;
				vbuf.tangent.y = val.y;
				vbuf.tangent.z = val.z;
			}
		}
	}

	MonoArray* API_SubMesh::getBitangents(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector3_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 vec;
				vec.x = vbuf.bitangent.x;
				vec.y = vbuf.bitangent.y;
				vec.z = vbuf.bitangent.z;

				mono_array_set(arr, API::Vector3, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setBitangents(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector3 val = mono_array_get(value, API::Vector3, i);
				vbuf.bitangent.x = val.x;
				vbuf.bitangent.y = val.y;
				vbuf.bitangent.z = val.z;
			}
		}
	}

	MonoArray* API_SubMesh::getTexcoords0(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector2_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector2 vec;
				vec.x = vbuf.texcoord0.x;
				vec.y = vbuf.texcoord0.y;

				mono_array_set(arr, API::Vector2, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setTexcoords0(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector2 val = mono_array_get(value, API::Vector2, i);
				vbuf.texcoord0.x = val.x;
				vbuf.texcoord0.y = val.y;
			}
		}
	}

	MonoArray* API_SubMesh::getTexcoords1(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector2_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector2 vec;
				vec.x = vbuf.texcoord1.x;
				vec.y = vbuf.texcoord1.y;

				mono_array_set(arr, API::Vector2, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setTexcoords1(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector2 val = mono_array_get(value, API::Vector2, i);
				vbuf.texcoord1.x = val.x;
				vbuf.texcoord1.y = val.y;
			}
		}
	}

	MonoArray* API_SubMesh::getBlendWeights(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector4_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector4 vec;
				vec.x = vbuf.blendWeights.x;
				vec.y = vbuf.blendWeights.y;
				vec.z = vbuf.blendWeights.z;
				vec.w = vbuf.blendWeights.w;

				mono_array_set(arr, API::Vector4, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setBlendWeights(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector4 val = mono_array_get(value, API::Vector4, i);
				vbuf.blendWeights.x = val.x;
				vbuf.blendWeights.y = val.y;
				vbuf.blendWeights.z = val.z;
				vbuf.blendWeights.w = val.w;
			}
		}
	}

	MonoArray* API_SubMesh::getBlendIndices(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->vector4_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector4 vec;
				vec.x = vbuf.blendIndices.x;
				vec.y = vbuf.blendIndices.y;
				vec.z = vbuf.blendIndices.z;
				vec.w = vbuf.blendIndices.w;

				mono_array_set(arr, API::Vector4, i, vec);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setBlendIndices(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Vector4 val = mono_array_get(value, API::Vector4, i);
				vbuf.blendIndices.x = val.x;
				vbuf.blendIndices.y = val.y;
				vbuf.blendIndices.z = val.z;
				vbuf.blendIndices.w = val.w;
			}
		}
	}

	MonoArray* API_SubMesh::getColors(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();
			MonoClass* type = APIManager::getSingleton()->color_class;

			MonoArray* arr = mono_array_new(APIManager::getSingleton()->getDomain(), type, vertexBuffer.size());
			for (int i = 0; i < vertexBuffer.size(); ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				Color col = Color::unpackABGR(vbuf.color);

				API::Color color;
				color.r = col.r();
				color.g = col.g();
				color.b = col.b();
				color.a = col.a();

				mono_array_set(arr, API::Color, i, color);
			}

			return arr;
		}

		return nullptr;
	}

	void API_SubMesh::setColors(MonoObject* this_ptr, MonoArray* value)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			auto& vertexBuffer = subMesh->getVertexBuffer();

			int size = (int)mono_array_length(value);

			if (vertexBuffer.size() != size)
				vertexBuffer.resize(size);

			for (int i = 0; i < size; ++i)
			{
				VertexBuffer& vbuf = vertexBuffer[i];

				API::Color val = mono_array_get(value, API::Color, i);
				vbuf.color = Color::packABGR(Color(val.r, val.g, val.b, val.a));
			}
		}
	}

	void API_SubMesh::commit(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			subMesh->commit();
		}
	}

	void API_SubMesh::destroy(MonoObject* this_ptr)
	{
		SubMesh* subMesh = nullptr;
		mono_field_get_value(this_ptr, APIManager::getSingleton()->object_ptr_field, &subMesh);

		if (subMesh != nullptr)
		{
			delete subMesh;
		}
	}
}