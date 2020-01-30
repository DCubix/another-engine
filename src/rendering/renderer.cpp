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

			const mat4 mBias = mat4(
				vec4(0.5, 0.0, 0.0, 0.0),
				vec4(0.0, 0.5, 0.0, 0.0),
				vec4(0.0, 0.0, 0.5, 0.0),
				vec4(0.5, 0.5, 0.5, 1.0)
			);
			const vec2 PoissonDisk[64] = vec2[](
				vec2(-0.613392, 0.617481),
				vec2(0.170019, -0.040254),
				vec2(-0.299417, 0.791925),
				vec2(0.645680, 0.493210),
				vec2(-0.651784, 0.717887),
				vec2(0.421003, 0.027070),
				vec2(-0.817194, -0.271096),
				vec2(-0.705374, -0.668203),
				vec2(0.977050, -0.108615),
				vec2(0.063326, 0.142369),
				vec2(0.203528, 0.214331),
				vec2(-0.667531, 0.326090),
				vec2(-0.098422, -0.295755),
				vec2(-0.885922, 0.215369),
				vec2(0.566637, 0.605213),
				vec2(0.039766, -0.396100),
				vec2(0.751946, 0.453352),
				vec2(0.078707, -0.715323),
				vec2(-0.075838, -0.529344),
				vec2(0.724479, -0.580798),
				vec2(0.222999, -0.215125),
				vec2(-0.467574, -0.405438),
				vec2(-0.248268, -0.814753),
				vec2(0.354411, -0.887570),
				vec2(0.175817, 0.382366),
				vec2(0.487472, -0.063082),
				vec2(-0.084078, 0.898312),
				vec2(0.488876, -0.783441),
				vec2(0.470016, 0.217933),
				vec2(-0.696890, -0.549791),
				vec2(-0.149693, 0.605762),
				vec2(0.034211, 0.979980),
				vec2(0.503098, -0.308878),
				vec2(-0.016205, -0.872921),
				vec2(0.385784, -0.393902),
				vec2(-0.146886, -0.859249),
				vec2(0.643361, 0.164098),
				vec2(0.634388, -0.049471),
				vec2(-0.688894, 0.007843),
				vec2(0.464034, -0.188818),
				vec2(-0.440840, 0.137486),
				vec2(0.364483, 0.511704),
				vec2(0.034028, 0.325968),
				vec2(0.099094, -0.308023),
				vec2(0.693960, -0.366253),
				vec2(0.678884, -0.204688),
				vec2(0.001801, 0.780328),
				vec2(0.145177, -0.898984),
				vec2(0.062655, -0.611866),
				vec2(0.315226, -0.604297),
				vec2(-0.780145, 0.486251),
				vec2(-0.371868, 0.882138),
				vec2(0.200476, 0.494430),
				vec2(-0.494552, -0.711051),
				vec2(0.612476, 0.705252),
				vec2(-0.578845, -0.768792),
				vec2(-0.772454, -0.090976),
				vec2(0.504440, 0.372295),
				vec2(0.155736, 0.065157),
				vec2(0.391522, 0.849605),
				vec2(-0.620106, -0.328104),
				vec2(0.789239, -0.419965),
				vec2(-0.545396, 0.538133),
				vec2(-0.178564, -0.596057)
			);

			struct Light {
				int type;
				vec3 position;
				vec3 direction;
				vec3 color;
				float intensity;
				float radius;
				float cutoff;
				bool hasShadow;
				mat4 viewProj;
			};

			struct Material {
				vec3 base;
				float shininess;
				float specular;
				bool receivesShadow;
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

			uniform sampler2D uShadows[32];

			uniform vec3 uEyePos;
			uniform vec3 uAmbient;

			uniform Material uMaterial;

			uniform sampler2D uDiffuse;
			uniform bool uDiffuseOn = false;

			uniform sampler2D uNormal;
			uniform bool uNormalOn = false;

			uniform sampler2D uSpecular;
			uniform bool uSpecularOn = false;

			uniform sampler2D uReflection;
			uniform bool uReflectionOn = false;

			uniform sampler2D uHeight;
			uniform float uHeightScale = 1.0;
			uniform bool uHeightOn = false;

			float rim(vec3 D, vec3 N) {
				float cs = abs(dot(D, N));
				return smoothstep(0.3, 1.0, 1.0 - cs);
			}

			vec2 matcap(vec3 eye, vec3 normal) {
				vec3 reflected = reflect(eye, normal);
				float m = 2.8284271247461903 * sqrt( reflected.z+1.0 );
				vec2 ret = reflected.xy / m + 0.5;
				ret.y = 1.0 - ret.y;
				return ret;
			}

			vec2 parallax(sampler2D depthMap, vec2 uv, vec3 V, float height) {
				const float minLayers = 8.0;
				const float maxLayers = 32.0;
				float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), V)));
				float layerDepth = 1.0 / numLayers;

				float currentLayerDepth = 0.0;
				
				vec2 P = V.xy * height;
				vec2 deltaTexCoords = P / numLayers;

				vec2  currentTexCoords     = uv;
				float currentDepthMapValue = 1.0 - texture(depthMap, currentTexCoords).r;

				while (currentLayerDepth < currentDepthMapValue) {
					currentTexCoords -= deltaTexCoords;
					currentDepthMapValue = 1.0 - texture(depthMap, currentTexCoords).r;
					currentLayerDepth += layerDepth;
				}

				vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

				float afterDepth  = currentDepthMapValue - currentLayerDepth;
				float beforeDepth = (1.0 - texture(depthMap, prevTexCoords).r) - currentLayerDepth + layerDepth;

				float weight = afterDepth / (afterDepth - beforeDepth);
				vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

				return finalTexCoords;
			}

			float sqr2(float x) {
				return x * x;
			}

			float findBlockerDistance(sampler2D shadowMap, vec3 coord, float lightSize, float bias) {
				float fd = coord.z - bias;

				int blockers = 0;
				float avgBlockerDistance = 0.0;
				float sw = lightSize * 0.01 / coord.z;

				for (int i = 0; i < 64; i++) {
					float z = texture(shadowMap, coord.xy + PoissonDisk[i % 64] * sw).r;
					if (z < fd) {
						blockers++;
						avgBlockerDistance += z;
					}
				}
				if (blockers < 1) return -1.1;
				return avgBlockerDistance / float(blockers);
			}

			bool inRange(float v) {
				return v >= 0.0 && v <= 1.0;
			}

			float PCF(in sampler2D sbuffer, vec3 coord, float bias, float radius) {
				if (!inRange(coord.x) || !inRange(coord.y) || !inRange(coord.z)) {
					return 0.0;
				}

				float fd = coord.z - bias;
				float sum = 0.0;
				for (int i = 0; i < 64; i++) {
					vec2 uvc = coord.xy + PoissonDisk[i % 64] * radius;
					float z = texture(sbuffer, uvc).r;
					sum += z < fd ? 1.0 : 0.0;
				}
				return sum / 64.0;
			}

			float PCSS(in sampler2D shadowMap, vec3 coord, float bias, float lightSize) {
				// blocker search
				float dist = findBlockerDistance(shadowMap, coord, lightSize, bias);
				if (dist <= -1.0) return 1.0;

				// penumbra estimation
				float pr = (coord.z - dist) / dist;

				// percentage-close filtering
				float radius = pr * lightSize * 0.01 / coord.z;
				return 1.0 - PCF(shadowMap, coord, bias, radius);
			}

			void main() {
				vec3 V = normalize(uEyePos - VS.position);
				vec3 N = normalize(VS.normal);
				vec2 uv = VS.texCoord;

				float h = abs(uHeightScale);
				if (uHeightOn && h > 0.0) {
					vec3 tanViewPos = VS.tbn * uEyePos;
					vec3 tanFragPos = VS.tbn * VS.position;
					vec3 viewDir = normalize(tanViewPos - tanFragPos);
					uv = parallax(uHeight, uv, viewDir, h);
				}

				if (uNormalOn) {
					vec3 norm = texture(uNormal, uv).xyz;
					norm.y = 1.0 - norm.y;
					N = normalize(VS.tbn * (norm * 2.0 - 1.0));
				}

				vec3 lighting = uAmbient;
				for (int i = 0; i < uLightCount; i++) {
					Light light = uLights[i];
					if (light.intensity <= 0.0 || light.radius <= 0.0) continue;
					vec3 L = vec3(0.0);
					float att = 1.0f;
					vec3 Lp = light.position;

					if (light.type == 0) {
						L = normalize(light.direction);
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

						float s = dot(L, normalize(light.direction));
						float c = cos(light.cutoff);
						if (s > c) {
							att *= (1.0 - (1.0 - s) * 1.0 / (1.0 - c));
						} else {
							att = 0.0;
						}
					}

					float NoL = max(dot(N, L), 0.0);
					float vis = 1.0;
					if (light.hasShadow && uMaterial.receivesShadow) {
						vec4 sc = mBias * light.viewProj * vec4(VS.position, 1.0);
						vec3 coord = sc.xyz / sc.w;
						if (uHeightOn && h > 0.0) {
							coord += VS.normal * texture(uHeight, uv).r * h;
						}

						float bias = clamp(tan(acos(NoL)) * 0.000001, 0.0, 0.000001);
						if (light.type == 0) vis = PCSS(uShadows[i], coord, bias, light.radius * 0.004);
						else if (light.type == 2) vis = 1.0 - PCF(uShadows[i], coord, bias, light.radius * 0.00035);
					}

					if (att > 0.0) {
						float fact = NoL * att * vis;
						fact += rim(V, N) * fact;

						vec3 R = reflect(-L, N);

						float shin = uMaterial.shininess;
						if (uSpecularOn) {
							shin *= texture(uSpecular, uv).r;
						}

						float spec = max(0.0, dot(R, V));
						spec = att * pow(spec, shin * 255.0) * uMaterial.specular;

						if (uSpecularOn) {
							spec *= texture(uSpecular, uv).g;
						}

						vec3 col = (light.color * fact);
						if (spec > 0.0) col += (light.color * spec);

						lighting += col * light.intensity;
					}
				}

				fragColor = vec4(uMaterial.base * lighting, 1.0);
				if (uDiffuseOn) {
					fragColor *= texture(uDiffuse, uv);
				}
				if (uReflectionOn) {
					fragColor *= texture(uReflection, matcap(V, N));
				}
				fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
			}
		)";

		const std::string shadowVS = R"(
			#version 440 core
			layout (location = 0) in vec3 vPosition;
			layout (location = 1) in vec3 vNormal;
			layout (location = 2) in vec3 vTangent;
			layout (location = 3) in vec2 vTexCoord;

			uniform mat4 uProjection;
			uniform mat4 uView;
			uniform mat4 uModel;

			void main() {
				gl_Position = uProjection * uView * uModel * vec4(vPosition, 1.0);
			}
		)";

		const std::string shadowFS = R"(
			#version 440 core
			void main() {}
		)";

		m_uber = std::make_unique<Shader>();
		m_uber->addShader(VS, ShaderType::VertexShader);
		m_uber->addShader(FS, ShaderType::FragmentShader);
		m_uber->link();

		m_shadows = std::make_unique<Shader>();
		m_shadows->addShader(shadowVS, ShaderType::VertexShader);
		m_shadows->addShader(shadowFS, ShaderType::FragmentShader);
		m_shadows->link();
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
		int shadowIndex = 0;
		world->each([&](Entity* ent, LightComponent* light) {
			auto&& istr = std::to_string(i);
			m_uber->get("uLights[" + istr + "].type").set(int(light->type()));
			m_uber->get("uLights[" + istr + "].position").set(light->position());
			m_uber->get("uLights[" + istr + "].direction").set(light->direction());
			m_uber->get("uLights[" + istr + "].color").set(light->color());
			m_uber->get("uLights[" + istr + "].intensity").set(light->intensity());
			m_uber->get("uLights[" + istr + "].radius").set(light->radius());
			m_uber->get("uLights[" + istr + "].cutoff").set(light->cutOff());
			m_uber->get("uLights[" + istr + "].viewProj").set(light->projection() * light->viewTransform());

			if ((light->type() == LightType::Directional || light->type() == LightType::Spot) && light->shadowsEnabled() && shadowIndex < 8) {
				renderShadows(world, light);
				m_uber->bind();
				light->shadowBuffer()->depthAttachment()->bind(shadowIndex);

				m_uber->get("uLights[" + istr + "].hasShadow").set(1);
				m_uber->get("uShadows[" + istr + "]").set(shadowIndex);
				shadowIndex++;
			} else {
				m_uber->get("uLights[" + istr + "].hasShadow").set(0);
			}

			i++;
		});
		m_uber->get("uLightCount").set(i);

		world->each([&](Entity* ent, MeshComponent* mesh) {
			m_uber->get("uDiffuseOn").set(0);
			m_uber->get("uNormalOn").set(0);
			m_uber->get("uSpecularOn").set(0);
			m_uber->get("uReflectionOn").set(0);
			m_uber->get("uHeightOn").set(0);

			m_uber->get("uModel").set(ent->transform());
			m_uber->get("uMaterial.base").set(mesh->material().base());
			m_uber->get("uMaterial.shininess").set(mesh->material().shininess());
			m_uber->get("uMaterial.specular").set(mesh->material().specular());
			m_uber->get("uMaterial.receivesShadow").set(mesh->material().receivesShadow());

			uint32 slot = shadowIndex;
			for (uint32 k = 0; k < Material::SlotCount; k++) {
				Texture* tex = mesh->material().m_textures[k];
				if (tex == nullptr) continue;

				tex->bind(slot);
				switch (k) {
					case Material::SlotDiffuse:
						m_uber->get("uDiffuse").set(int(slot));
						m_uber->get("uDiffuseOn").set(1);
						break;
					case Material::SlotNormal:
						m_uber->get("uNormal").set(int(slot));
						m_uber->get("uNormalOn").set(1);
						break;
					case Material::SlotSpecular:
						m_uber->get("uSpecular").set(int(slot));
						m_uber->get("uSpecularOn").set(1);
						break;
					case Material::SlotReflection:
						m_uber->get("uReflection").set(int(slot));
						m_uber->get("uReflectionOn").set(1);
						break;
					case Material::SlotHeight:
						m_uber->get("uHeight").set(int(slot));
						m_uber->get("uHeightScale").set(mesh->material().height());
						m_uber->get("uHeightOn").set(1);
						break;
				}
				slot++;
			}

			Mesh* m = mesh->mesh();
			m->bind();
			m->draw(Mesh::Triangles);

			for (uint32 k = 0; k < Material::SlotCount; k++) {
				Texture* tex = mesh->material().m_textures[k];
				if (tex == nullptr) continue;
				tex->unbind();
			}
		});
	}

	void Renderer::renderShadows(EntityWorld* world, LightComponent* comp) {
		if (!comp->shadowBuffer()) {
			comp->createShadowBuffer();
		}

		auto buff = comp->shadowBuffer();

		buff->bind();
		buff->clear(ClearBufferMask::DepthBuffer);

		m_shadows->bind();

		m_shadows->get("uProjection").set(comp->projection());
		m_shadows->get("uView").set(comp->viewTransform());

		world->each([&](Entity* ent, MeshComponent* mesh) {
			if (mesh->material().castsShadow()) {
				m_shadows->get("uModel").set(ent->transform());
				Mesh* m = mesh->mesh();

				glCullFace(GL_FRONT);
				m->bind();
				m->draw(Mesh::Triangles);
				m->unbind();
				glCullFace(GL_BACK);
			}
		});

		m_shadows->unbind();
		buff->unbind();
	}

}
