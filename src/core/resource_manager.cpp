#include "resource_manager.h"

namespace ae {
	ResourceManager& ResourceManager::subscribe(const std::string& name, FactoryFunctor factory) {
		m_factories[name] = factory;
		return *this;
	}
}