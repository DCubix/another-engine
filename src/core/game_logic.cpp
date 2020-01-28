#include "game_logic.h"

namespace ae {

	void Entity::cleanup() {
		m_behaviors.clear();
		m_position = Vector3(0.0f);
		m_rotation = Quaternion();
		m_scale = Vector3(1.0f);
		m_life = -1.0f;
		m_init = false;
		m_dead = false;
	}

	Matrix4 Entity::transform() const {
		return Matrix4::translation(m_position) *
				m_rotation.toMatrix4() *
				Matrix4::scale(m_scale);
	}

	Matrix4 Entity::viewTransform() const {
		return m_rotation.conjugated().toMatrix4() *
				Matrix4::translation(m_position * -1.0f);
	}

	Entity* EntityWorld::create(const std::string& templateName) {
		auto&& temp = m_templates.find(templateName);
		if (temp == m_templates.end()) return nullptr;
		auto&& ent = create();
		temp->second(ent);
		return ent;
	}

	Entity* EntityWorld::create() {
		if (m_inactivePool.empty()) { // Allocate new Entity
			auto&& ent = std::make_unique<Entity>();
			m_activePool.push_back(std::move(ent));
		} else { // Reuse inactive
			auto&& ent = std::move(m_inactivePool[0]);
			m_inactivePool.erase(m_inactivePool.begin());
			ent->cleanup();
			m_activePool.push_back(std::move(ent));
		}
		return m_activePool.back().get();
	}

	void EntityWorld::registerTemplate(const std::string& templateName, const EntityTemplate& functor) {
		m_templates.insert({ templateName, functor });
	}
	
	void EntityWorld::update(float dt) {
		for (auto&& entity : m_activePool) {
			if (!entity->m_dead) {
				if (entity->m_life > 0.0f) {
					entity->m_life -= dt;
				} else {
					entity->m_dead = true;
					entity->m_life = 0.0f;
				}
				for (auto&& behavior : entity->behaviors()) {
					if (!entity->m_init) behavior->onCreate(*this);
					if (!behavior->enabled()) continue;
					behavior->onUpdate(*this, dt);
				}
				entity->m_init = true;
			} else continue;
		}

		auto&& it = m_activePool.begin();
		while (it != m_activePool.end()) {
			if ((*it)->m_dead) {
				auto&& ent = std::move(*it);
				for (auto&& behavior : ent->behaviors()) {
					if (!behavior->enabled()) continue;
					behavior->onDestroy(*this);
				}
				it = m_activePool.erase(it);
				m_inactivePool.push_back(std::move(ent));
			} else ++it;
		}
	}

}