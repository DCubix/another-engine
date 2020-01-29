#ifndef RENDERER_H
#define RENDERER_H

#include "vec_math.hpp"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "game_logic.h"

namespace ae {

	enum class LightType : uint8 {
		Directional = 0,
		Point,
		Spot
	};

	class LightComponent : public Component {
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

	private:
		LightType m_type{ LightType::Directional };
		Vector3 m_color{ Vector3(1.0f) };
		float m_radius{ 16.0f }, m_cutoff{ 0.5f }, m_intensity{ 1.0f };
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
		std::unique_ptr<Shader> m_uber;
		CameraComponent* m_camera{ nullptr };

		Vector3 m_ambient{ Vector3(0.15f) };
	};

}

#endif // RENDERER_H
