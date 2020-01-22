#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <functional>
#include <memory>
#include <unordered_map>

#include "file_system.h"
#include "log.h"

namespace ae {
	class Resource {
	public:
		virtual ~Resource() = default;
		virtual void fromFile(const std::string& fileName) = 0;
	};

	class ResourceManager {
	public:
		template <class T>
		inline T* load(const std::string& key, const std::string& fileName) {
			static_assert(std::is_base_of<Resource, T>::value, "Invalid Resource type.");

			auto pos = m_resources.find(key);
			if (pos != m_resources.end()) {
				auto&& rawPtr = dynamic_cast<T*>(pos->second.get());
				Log.assert(rawPtr, "You cannot cast this resource into the specified type.");

				return rawPtr;
			}

			auto ptr = std::unique_ptr<T>(new T());
			ptr->fromFile(fileName);

			m_resources.insert({ key, std::move(ptr) });
			auto&& rawPtr = dynamic_cast<T*>(m_resources[key].get());
			Log.assert(rawPtr, "You cannot cast this resource into the specified type.");

			return rawPtr;
		}

		template <class T>
		inline T* get(const std::string& key) {
			static_assert(std::is_base_of<Resource, T>::value, "Invalid Resource type.");
			auto pos = m_resources.find(key);
			if (pos == m_resources.end()) {
				Log.error("Resource \"" + key + "\" not found.");
				return nullptr;
			}

			auto&& rawPtr = dynamic_cast<T*>(pos->second.get());
			Log.assert(rawPtr, "You cannot cast this resource into the specified type.");

			return rawPtr;
		}

		static ResourceManager& ston() { return s_instance; }
	private:
		std::unordered_map<std::string, std::unique_ptr<Resource>> m_resources;

		static ResourceManager s_instance;
	};

}

#endif // RESOURCE_MANAGER_H
