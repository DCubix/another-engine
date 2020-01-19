#include "file_system.h"

#include "log.h"

namespace ae {
	FileSystem FileSystem::s_instance{};

	void FileSystem::init(const char* argv0) {
		if (!PHYSFS_isInit()) {
			PHYSFS_init(argv0);
			mount(appDir());
		}
	}

	FileSystem::~FileSystem() {
		if (PHYSFS_isInit()) {
			PHYSFS_deinit();
		}
	}

	void FileSystem::mount(const std::string& path, const std::string& mountPoint) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		if (!PHYSFS_mount(path.c_str(), mountPoint.c_str(), 1)) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	void FileSystem::umount(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		if (!PHYSFS_unmount(path.c_str())) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	void FileSystem::mkdir(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		if (!PHYSFS_mkdir(path.c_str())) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	void FileSystem::rm(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		if (!PHYSFS_delete(path.c_str())) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	std::vector<std::string> FileSystem::ls(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");

		std::vector<std::string> files;
		for (char** i = PHYSFS_enumerateFiles(path.c_str()); *i != nullptr; i++) {
			files.push_back(std::string(*i));
		}
		return files;
	}

	std::string FileSystem::where(const std::string& file) {
		init(nullptr);
		Log.assert(!file.empty(), "Please specify a path.");
		return std::string(PHYSFS_getRealDir(file.c_str()));
	}

	std::string FileSystem::appDir() {
		init(nullptr);
		return std::string(PHYSFS_getBaseDir());
	}

	void FileSystem::setWriteDir(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		if (!PHYSFS_setWriteDir(path.c_str())) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	bool FileSystem::exists(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		return PHYSFS_exists(path.c_str()) != 0;
	}

	bool FileSystem::isDir(const std::string& path) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		PHYSFS_Stat stat;
		PHYSFS_stat(path.c_str(), &stat);
		return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
	}

	FileSystem::File FileSystem::open(const std::string& path, FileMode mode) {
		init(nullptr);
		Log.assert(!path.empty(), "Please specify a path.");
		return File(path.c_str(), mode);
	}

	FileSystem::File::File(const char* path, FileMode mode) {
		PHYSFS_Stat stats;
		if (!PHYSFS_stat(path, &stats)) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
		
		switch (stats.filetype) {
			case PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR: type = File::TypeFile; break;
			case PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY: type = File::TypeDirectory; break;
			default: type = File::TypeUnknown; break;
		}
		read_only = stats.readonly != 0;
		file_name = std::string(path);

		if ((mode == FileMode::ModeWrite || mode == FileMode::ModeAppend) && read_only) {
			Log.error("File is read-only.");
			fp = nullptr;
			return;
		}

		switch (mode) {
			case FileMode::ModeWrite: fp = PHYSFS_openWrite(path); break;
			case FileMode::ModeRead: fp = PHYSFS_openRead(path); break;
			case FileMode::ModeAppend: fp = PHYSFS_openAppend(path); break;
		}

		if (fp == nullptr) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	uint64_t FileSystem::File::size() const {
		Log.assert(fp != nullptr, "Invalid file.");
		return PHYSFS_fileLength(fp);
	}

	uint64_t FileSystem::File::modified() const {
		Log.assert(fp != nullptr, "Invalid file.");
		PHYSFS_Stat stat;
		PHYSFS_stat(file_name.c_str(), &stat);
		return stat.filesize;
	}

	void FileSystem::File::close() {
		Log.assert(fp != nullptr, "Invalid file.");
		PHYSFS_close(fp);
	}

	bool FileSystem::File::eof() const {
		Log.assert(fp != nullptr, "Invalid file.");
		return PHYSFS_eof(fp) != 0;
	}

	void FileSystem::File::write(const void* data, uint64_t size) {
		Log.assert(fp != nullptr, "Invalid file.");
		if (PHYSFS_writeBytes(fp, data, size) < size) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
	}

	uint64_t FileSystem::File::read(void* buffer, uint64_t size) {
		Log.assert(fp != nullptr, "Invalid file.");
		int64_t sz = PHYSFS_readBytes(fp, buffer, size);
		if (sz < size) {
			Log.error("File System Error: " + std::string(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
		}
		return sz;
	}

}