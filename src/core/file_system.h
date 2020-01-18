#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "physfs.h"
#include <string>
#include <vector>

namespace ae {
	class FileSystem {
	public:
		enum FileMode {
			ModeRead = 0,
			ModeWrite,
			ModeAppend
		};

		FileSystem() = default;
		~FileSystem();

		void init(const char* argv0 = NULL);

		void mount(const std::string& path, const std::string& mountPoint = "");
		void umount(const std::string& path);

		void mkdir(const std::string& path);
		void rm(const std::string& path);
		std::vector<std::string> ls(const std::string& path);
		std::string where(const std::string& file);

		bool exists(const std::string& path);
		bool is_dir(const std::string& path);

		std::string app_dir() const;
		void set_write_dir(const std::string& path);

		struct File {
			enum {
				TypeFile = 0,
				TypeDirectory,
				TypeUnknown
			} type;

			std::string file_name;
			bool read_only;

			PHYSFS_File* fp;

			File(const char* path, FileMode mode);

			uint64_t size() const;
			uint64_t modified() const;
			void close();
			bool eof() const;

			void write(const void* data, uint64_t size);
			uint64_t read(void* buffer, uint64_t size);

		};

		File open(const std::string& path, FileMode mode = FileMode::ModeRead);

		static FileSystem& ston() { return s_instance; }

	private:
		static FileSystem s_instance;
	};
}

#endif // FILE_SYSTEM_H