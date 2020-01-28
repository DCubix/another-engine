#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "integer.hpp"
#include "vec_math.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <algorithm>

namespace ae {
	using Type = std::type_index;

	class Entity;
	class EntityWorld;
	class Component {
		friend class Entity;
	public:
		virtual ~Component() = default;

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
		virtual ~Entity() = default;

		const Vector3& position() const { return m_position; }
		void position(const Vector3& position) { m_position = position; }

		const Vector3& scale() const { return m_scale; }
		void scale(const Vector3& scale) { m_scale = scale; }

		const Quaternion& rotation() const { return m_rotation; }
		void rotation(const Quaternion& rotation) { m_rotation = rotation; }

		Matrix4 transform() const;
		Matrix4 viewTransform() const;

		const std::unordered_map<Type, std::unique_ptr<Component>>& components() { return m_components; }

		template <class T, typename... Args>
		inline T* createComponent(Args&&... args) {
			static_assert(std::is_base_of<Component, T>::value, "Invalid Component type.");
			auto&& type = std::type_index(typeid(T));
			T* bh = new T(std::forward<Args>(args)...);
			bh->m_owner = this;
			m_components[type] = std::unique_ptr<Component>(bh);
			return (T*) m_components[type].get();
		}

		template <class T>
		inline T* getComponent() {
			static_assert(std::is_base_of<Component, T>::value, "Invalid Component type.");
			auto&& type = typeid(T);
			auto&& pos = m_components.find(type);
			if (pos == m_components.end()) return nullptr;
			return (T*) pos->second.get();
		}

		template <class T>
		bool has() const {
			static_assert(std::is_base_of<Component, T>::value, "Invalid Component type.");
			auto&& type = std::type_index(typeid(T));
			return m_components.find(type) != m_components.end();
		}

		template<typename T, typename V, typename... Types>
		bool has() const {
			return has<T>() && has<V, Types...>();
		}

		void cleanup();
		void update(EntityWorld* world, float dt);

		float life() const { return m_life; }
		void destroy(float timeout = 0.0f) { m_life = std::abs(timeout); }

	protected:
		Vector3 m_position{}, m_scale{ 1.0f };
		Quaternion m_rotation{};

		std::unordered_map<Type, std::unique_ptr<Component>> m_components;

		float m_life{ -1.0f };
		bool m_init{ false }, m_dead{ false };
	};

	using EntityTemplate = std::function<void(Entity*)>;
	class EntityWorld {
	public:
		virtual ~EntityWorld() = default;

		Entity* create(const std::string& templateName);
		Entity* create();

		void registerTemplate(const std::string& templateName, const EntityTemplate& functor);

		const std::vector<std::unique_ptr<Entity>>& entities() { return m_activePool; }

		void update(float dt);

		template <class T>
		inline Entity* find() {
			for (auto&& ent : m_activePool) {
				if (ent->getComponent<T>()) return ent.get();
			}
			return nullptr;
		}

		template<class... Cs>
		inline void each(void(*f)(Entity&, Cs...)) {
			eachInternalParallel<Cs...>(f);
		}

		template <class F>
		inline void each(F&& func) {
			lambdaEachInternal(&F::operator(), func);
		}

	private:
		std::vector<std::unique_ptr<Entity>> m_activePool, m_inactivePool;
		std::unordered_map<std::string, EntityTemplate> m_templates;

		template <class... Cs, class Fn>
		inline void eachInternalParallel(Fn&& func) {
			std::for_each(m_activePool.begin(), m_activePool.end(), [func](std::unique_ptr<Entity>& ent) {
				if (ent->has<Cs...>()) func(ent.get(), ent->getComponent<Cs>()...);
			});
		}

		template<class G, class... Cs, class Fn>
		inline void lambdaEachInternal(void (G::*)(Entity*, Cs*...) const, Fn&& f) {
			eachInternalParallel<Cs...>(std::forward<Fn>(f));
		}
	};

}

#endif // GAME_LOGIC_H
