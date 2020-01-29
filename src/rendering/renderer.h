#ifndef RENDERER_H
#define RENDERER_H

#include "vec_math.hpp"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "game_logic.h"
#include "framebuffer.h"

namespace ae {

	constexpr uint32 shadowMapSize = 1024;

	enum class LightType : uint8 {
		Directional = 0,
		Point,
		Spot
	};

	class LightComponent : public Component {
		friend class Renderer;
	public:
		LightType type() const { return m_type; }
		void type(LightType type) { m_type = type; }

		const Vector3& position() const { return owner()->position(); }
		Vector3 direction() const { return owner()->rotation().rotate(Vector3(0.0f, 0.0f, 1.0f)); }

		const Vector3& color() const { return m_color; }
		void color(const Vector3& color) { m_color = color; }

		float intensity() const { return m_intensity; }
		void intensity(float v) { m_intensity = v; }

		float radius() const { return m_radius; }
		void radius(float r) { m_radius = r; }

		float cutOff() const { return m_cutoff; }
		void cutOff(float cutOff) { m_cutoff = cutOff; }

		bool castsShadow() const { return m_castsShadow; }
		void castsShadow(bool v) { m_castsShadow = v; }

		FrameBuffer* shadowBuffer() { return m_shadowBuffer.get(); }

		inline Matrix4 projection() const {
			return m_type == LightType::Directional ?
					Matrix4::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.01f, 1000.0f) :
					Matrix4::perspective(m_cutoff, 1.0f, 0.01f, m_radius * 2.0f);
		}

		inline Matrix4 viewTransform() const {
			return owner()->rotation().conjugated().toMatrix4() *
					Matrix4::translation(owner()->position() * -1.0f);
		}

	private:
		LightType m_type{ LightType::Directional };
		Vector3 m_color{ Vector3(1.0f) };
		float m_radius{ 16.0f }, m_cutoff{ 0.5f }, m_intensity{ 1.0f };
		bool m_castsShadow{ true };

		std::unique_ptr<FrameBuffer> m_shadowBuffer;

		inline void createShadowBuffer() {
			m_shadowBuffer = std::make_unique<FrameBuffer>();
			m_shadowBuffer->create(shadowMapSize, shadowMapSize);
			m_shadowBuffer->depth();
		}
	};

	struct Material {
		enum SlotType {
			SlotDiffuse = 0,
			SlotNormal,
			SlotSpecular,
			SlotCount
		};

		Vector3 baseColor{ Vector3(1.0f) };
		float shininess{ 0.15f };
		float specular{ 1.0f };
		Texture* textures[SlotCount];
		bool castsShadow{ true };
		bool receivesShadow{ true };
	};

	class MeshComponent : public Component {
	public:
		MeshComponent() = default;
		explicit MeshComponent(Mesh* mesh) : m_mesh(mesh) {}

		Mesh* mesh() { return m_mesh; }
		void mesh(Mesh* mesh) { m_mesh = mesh; }

		Material& material() { return m_material; }

	private:
		Mesh* m_mesh{ nullptr };
		Material m_material{};
	};

	class CameraComponent : public Component {
	public:
		inline Matrix4 projection(float aspect) {
			return Matrix4::perspective(m_fov, aspect, m_near, m_far);
		}

		inline Matrix4 viewTransform() {
			return owner()->rotation().conjugated().toMatrix4() *
					Matrix4::translation(owner()->position() * -1.0f);
		}

		float fov() const { return m_fov; }
		void fov(float f) { m_fov = f; }

		float znear() const { return m_near; }
		void znear(float n) { m_near = n; }

		float zfar() const { return m_far; }
		void zfar(float f) { m_far = f; }

	private:
		float m_near{ 0.01f }, m_far{ 500.0f }, m_fov{ mathutils::toRadians(60.0f) };
	};

	class Renderer {
	public:
		Renderer();
		void render(EntityWorld* world, uint32 width, uint32 height);
	
		CameraComponent* camera() { return m_camera; }
		void camera(CameraComponent* camera) { m_camera = camera; }

		const Vector3& ambient() const { return m_ambient; }
		void ambient(const Vector3& ambient) { m_ambient = ambient; }

	private:
		std::unique_ptr<Shader> m_uber, m_shadows;

		CameraComponent* m_camera{ nullptr };

		Vector3 m_ambient{ Vector3(0.15f) };

		void renderShadows(EntityWorld* world, LightComponent* comp);
	};

}

#endif // RENDERER_H
