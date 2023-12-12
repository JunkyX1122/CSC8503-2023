#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader);
			~RenderObject();

			void SetDefaultTexture(Texture* t) {
				texture = t;
			}

			Texture* GetDefaultTexture() const {
				return texture;
			}

			Mesh*	GetMesh() const {
				return mesh;
			}

			Transform*		GetTransform() const {
				return transform;
			}

			Shader*		GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return colour;
			}

			void SetTextureScale(Vector3 v) { textureScale = v; }
			Vector3 GetTextureScale() { return textureScale; }
		protected:
			Mesh*		mesh;
			Texture*	texture;
			Shader*		shader;
			Transform*	transform;
			Vector4		colour;
			Vector3		textureScale = Vector3(1,1,1);
		};
	}
}
