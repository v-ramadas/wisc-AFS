#ifndef UNRELIABLEFS_WRAPPER_HPP
#define UNRELIABLEFS_WRAPPER_HPP

#include <string>
#include <vector>
#include "unreliablefs.h"

class UnreliableFSWrapper {
public:
    UnreliableFSWrapper(const std::string& path, int mode) {
        unreliable_init(&ufs, path.c_str(), mode);
    }

    ~UnreliableFSWrapper() {
        unreliable_cleanup(&ufs);
    }

    int open(const std::string& path, int flags) {
        return wiscAFS_open(&ufs, path.c_str(), flags);
    }

    int close(int fd) {
        return wiscAFS_close(&ufs, fd);
    }

    ssize_t read(int fd, void *buf, size_t count) {
        return wiscAFS_read(&ufs, fd, buf, count);
    }

    ssize_t write(int fd, const void *buf, size_t count) {
        return wiscAFS_write(&ufs, fd, buf, count);
    }

    int unlink(const std::string& path) {
        return wiscAFS_unlink(&ufs, path.c_str());
    }

    std::vector<std::string> readdir(const std::string& path) {
        std::vector<std::string> files;
        unreliablefs_dir_entry_t entry;
        unreliablefs_dir_t dir = unreliable_opendir(&ufs, path.c_str());
        while (unreliable_readdir(&ufs, dir, &entry) == 0) {
            files.push_back(std::string(entry.d_name));
        }
        unreliable_releasedir(&ufs, dir);
        return files;
    }

private:
    unreliable_fs_t ufs;
};

#endif
