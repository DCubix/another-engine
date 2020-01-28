#include "renderer.h"

namespace ae {
	
	Renderer::Renderer() {
		const std::string VS = R"(
			#version 440 core
			layout (location = 0) in vec3 vPosition;
			layout (location = 1) in vec3 vNormal;
			layout (location = 2) in vec3 vTangent;
			layout (location = 3) in vec2 vTexCoord;

			uniform mat4 uProjection;
			uniform mat4 uView;
			uniform mat4 uModel;

			out Data {
				vec3 position;
				vec3 normal;
				vec3 tangent;
				vec2 texCoord;
				mat3 tbn;
			} VS;

			void main() {
				vec4 pos = uModel * vec4(vPosition, 1.0);
				gl_Position = uProjection * uView * pos;

				VS.position = pos.xyz;
				VS.normal = normalize((uModel * vec4(vNormal, 0.0)).xyz);
				VS.tangent = normalize((uModel * vec4(vTangent, 0.0)).xyz);
				VS.tangent = normalize(VS.tangent - dot(VS.tangent, VS.normal) * VS.normal);
				VS.texCoord = vTexCoord;
				
				vec3 b = cross(VS.tangent, VS.normal);
				VS.tbn = mat3(VS.tangent, b, VS.normal);
			}
		)";

		const std::string FS = R"(
			#version 440 core
			out vec4 fragColor;

			struct Light {
				int type;
				vec3 position;
				vec3 direction;
				vec3 color;
				float intensity;
				float radius;
				float cutoff;
			};

			struct Material {
				vec3 base;
				float shininess;
				float specular;
			};

			in Data {
				vec3 position;
				vec3 normal;
				vec3 tangent;
				vec2 texCoord;
				mat3 tbn;
			} VS;

			uniform Light uLights[32];
			uniform int uLightCount = 0;

			uniform vec3 uEyePos;
			uniform vec3 uAmbient;

			uniform Material uMaterial;

			float rim(vec3 D, vec3 N) {
				float cs = abs(dot(D, N));
				return smoothstep(0.3, 1.0, 1.0 - cs);
			}

			float sqr2(float x) {
				return x * x;
			}

			void main() {
				vec3 V = normalize(uEyePos - VS.position);
				vec3 N = normalize(VS.normal);

				vec3 lighting = uAmbient;
				for (int i = 0; i < uLightCount; i++) {
					Light light = uLights[i];
					if (light.intensity <= 0.0 || light.radius <= 0.0) continue;
					vec3 L = vec3(0.0);
					float att = 1.0f;
					vec3 Lp = light.position;

					if (light.type == 0) {
						L = normalize(-light.direction);
					} else if (light.type == 1) {
						L = Lp - VS.position;
						float dist = length(L);
						L = normalize(L);

						if (dist < light.radius) {
							att = sqr2(clamp(1.0 - dist / light.radius, 0.0, 1.0));
						} else {
							att = 0.0;
						}
					} else if (light.type == 2) {
						L = Lp - VS.position;
						float dist = length(L);
						L = normalize(L);

						att = sqr2(clamp(1.0 - dist / light.radius, 0.0, 1.0));

						float s = dot(L, normalize(-light.direction));
						float c = cos(light.cutoff);
						if (s > c) {
							att *= (1.0 - (1.0 - s) * 1.0 / (1.0 - c));
						} else {
							att = 0.0;
						}
					}

					float NoL = max(dot(N, L), 0.0);
					if (att > 0.0) {
						float fact = NoL * att;
						fact += rim(V, N) * 2.0 * fact * light.intensity;

						vec3 R = reflect(-L, N);

						float spec = max(0.0, dot(R, V));
						spec = att * pow(spec, uMaterial.shininess * 255.0) * uMaterial.specular * light.intensity;

						lighting += (light.color * fact);
						if (spec > 0.0) lighting += (light.color * spec);
					}
				}

				fragColor = vec4(uMaterial.base * lighting, 1.0);
			}
		)";

		m_uber = std::make_unique<Shader>();
		m_uber->addShader(VS, ShaderType::VertexShader);
		m_uber->addShader(FS, ShaderType::FragmentShader);
		m_uber->link();

	}

	void Renderer::render(EntityWorld* world, uint32 width, uint32 height) {
		m_uber->bind();

		if (m_camera == nullptr) {
			Entity* cam = world->find<CameraComponent>();
			if (cam != nullptr) {
				m_camera = cam->getComponent<CameraComponent>();
			} else {
				return;
			}
		}

		const float aspect = float(width) / height;
		m_uber->get("uProjection").set(m_camera->projection(aspect));
		m_uber->get("uView").set(m_camera->viewTransform());
		m_uber->get("uEyePos").set(m_camera->owner()->position());
		m_uber->get("uAmbient").set(m_ambient);

		int i = 0;
		world->each([&](Entity* ent, LightComponent* light) {
			auto&& istr = std::to_string(i);
			m_uber->get("uLights[" + istr + "].type").set(int(light->type()));
			m_uber->get("uLights[" + istr + "].position").set(light->position());
			m_uber->get("uLights[" + istr + "].direction").set(light->direction());
			m_uber->get("uLights[" + istr + "].color").set(light->color());
			m_uber->get("uLights[" + istr + "].intensity").set(light->intensity());
			m_uber->get("uLights[" + istr + "].radius").set(light->radius());
			m_uber->get("uLights[" + istr + "].cutoff").set(light->cutOff());
			i++;
		});
		m_uber->get("uLightCount").set(i);

		world->each([&](Entity* ent, MeshComponent* mesh) {
			m_uber->get("uModel").set(ent->transform());
			m_uber->get("uMaterial.base").set(mesh->material().baseColor);
			m_uber->get("uMaterial.shininess").set(mesh->material().shininess);
			m_uber->get("uMaterial.specular").set(mesh->material().specular);

			Mesh* m = mesh->mesh();
			m->bind();
			m->draw(Mesh::Triangles);
		});
	}

}
