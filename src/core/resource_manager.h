#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <functional>
#include <memory>
#include <unordered_map>

#include "file_system.h"
#include "log.h"

namespace ae {
	class ResourceBase {
	public:
		virtual ~ResourceBase() = default;
	};

	using ResourcePtr = std::shared_ptr<ResourceBase>;
	using FactoryFunctor = std::function<ResourcePtr()>;

	struct ResourceFactory {
		virtual ResourcePtr load() = 0;

		void fileName(const std::string& fileNme) { m_fileName = fileNme; }
		const std::string& fileName() const { return m_fileName; }

		ResourcePtr operator() () { return load(); }

	private:
		std::string m_fileName;
	};

	class ResourceManager {
	public:
		ResourceManager& subscribe(const std::string& name, FactoryFunctor factory);

		template <class T>
		std::shared_ptr<T> load(const std::string& key) {
			auto& factoryFunctor = m_factories.at(key);
			ResourcePtr ptr = factoryFunctor();

			auto rawPtr = dynamic_cast<T*>(ptr.get());
			Log.assert(rawPtr, "Cannot cast this pointer.");

			return std::shared_ptr<T>(ptr, rawPtr);
		}

		static ResourceManager& ston() { return s_instance; }

	private:
		std::unordered_map<std::string, FactoryFunctor> m_factories;
		static ResourceManager s_instance;
	};

}

#endif // RESOURCE_MANAGER_H
