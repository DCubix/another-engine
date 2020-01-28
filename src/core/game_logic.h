#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "integer.hpp"
#include "vec_math.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace ae {
	class Entity;
	class EntityWorld;
	class Behavior {
		friend class Entity;
	public:
		Behavior() = default;
		virtual ~Behavior() = default;

		virtual void onCreate(EntityWorld& world) {}
		virtual void onUpdate(EntityWorld& world, float dt) {}
		virtual void onDestroy(EntityWorld& world) {}

		Entity* owner() const { return m_owner; }

		bool enabled() const { return m_enabled; }
		void enabled(bool enabled) { m_enabled = enabled; }

	protected:
		Entity *m_owner;
		bool m_enabled{ true };
	};

	class Entity {
		friend class EntityWorld;
	public:
		Entity() = default;
		virtual ~Entity() = default;

		const Vector3& position() const { return m_position; }
		void position(const Vector3& position) { m_position = position; }

		const Vector3& scale() const { return m_scale; }
		void scale(const Vector3& scale) { m_scale = scale; }

		const Quaternion& rotation() const { return m_rotation; }
		void rotation(const Quaternion& rotation) { m_rotation = rotation; }

		Matrix4 transform() const;
		Matrix4 viewTransform() const;

		const std::vector<std::unique_ptr<Behavior>>& behaviors() { return m_behaviors; }

		template <class T, typename... Args>
		T* createBehavior(Args&&... args) {
			static_assert(std::is_base_of<Behavior, T>::value, "Invalid Behavior type.");
			T* bh = new T(std::forward<Args>(args)...);
			bh->m_owner = this;
			m_behaviors.push_back(std::unique_ptr<Behavior>(bh));
			return m_behaviors.back().get();
		}

		void cleanup();

		float life() const { return m_life; }
		void destroy(float timeout = 0.0f) { m_life = std::abs(timeout); }

	protected:
		Vector3 m_position{}, m_scale{ 1.0f };
		Quaternion m_rotation{};

		std::vector<std::unique_ptr<Behavior>> m_behaviors;

		float m_life{ -1.0f };
		bool m_init{ false }, m_dead{ false };
	};

	using EntityTemplate = std::function<void(Entity*)>;
	class EntityWorld {
	public:
		EntityWorld() = default;
		virtual ~EntityWorld() = default;

		Entity* create(const std::string& templateName);
		Entity* create();

		void registerTemplate(const std::string& templateName, const EntityTemplate& functor);

		const std::vector<std::unique_ptr<Entity>>& entities() { return m_activePool; }

		void update(float dt);

	private:
		std::vector<std::unique_ptr<Entity>> m_activePool, m_inactivePool;
		std::unordered_map<std::string, EntityTemplate> m_templates;
	};

}

#endif // GAME_LOGIC_H
