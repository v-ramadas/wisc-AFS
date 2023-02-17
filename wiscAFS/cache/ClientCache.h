#pragma once
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <map>
#include <vector>

class FileAttrs{
    public:
    FileAttrs() : filesize(0), atime(0), mtime(0) {};
    FileAttrs(const int file_size, const int a_time, const int mtime) : filesize(file_size), atime(a_time), mtime(mtime){};
    unsigned int filesize;
    unsigned long int atime;
    unsigned long int mtime;
    
};

class ClientCacheValue {
    public:
    ClientCacheValue(){};
    ClientCacheValue(const FileAttrs &file_Attrs, int inode, bool isDirty, int fileDiscriptor) : fileAttrs(file_Attrs), inode(-1), isDirty(false), fileDiscriptor(fileDiscriptor)  {};
    FileAttrs fileAttrs;
    int inode;
    bool isDirty;
    int fileDiscriptor;

};


class DiskCache {
    
    const std::string CACHE_FILE =  "cache_metadata.txt";
    std::map<std::string, ClientCacheValue> cache;

    public:
    void loadCache() {
        std::ifstream cache_file(CACHE_FILE);
        if (cache_file) {
            std::string line;
            std::vector<std::string> substrings;
            while (std::getline(cache_file, line)) {
                std::string sep = ":";
                size_t pos = line.find(":");
                int start = 0;
                while(pos != std::string::npos) {
                    std::string key = line.substr(start, pos-start);
                    substrings.push_back(key);
                    start = pos + 1;
                    pos = line.find(":", pos + sep.size());
                    
                }

               std::string key = substrings[0];
               int inode = std::stoi(substrings[1]);
               bool isDirty = std::stoi(substrings[2]);
               int fd = std::stoi(substrings[3]);
               int atime = std::stoi(substrings[4]);
               int mtime = std::stoi(substrings[5]);
               int size = std::stoi(substrings[6]);
               FileAttrs fileattrs(size, atime, mtime);
               ClientCacheValue clientCacheValue(fileattrs, inode, isDirty, fd);
               cache.insert({key, clientCacheValue});
            }
            cache_file.close();
        }
    }

    void saveCache() {
        std::ofstream cache_file(CACHE_FILE);
        if (cache_file) {
            //for (const auto& [key, value] : cache) {
            for (auto it = cache.begin(); it != cache.end(); it++) {
                std::string key = it->first;
                ClientCacheValue value = it->second;
                cache_file << key << ":" << value.inode << ":" << (value.isDirty ? 1 : 0) << ":" << value.fileDiscriptor <<":"<< value.fileAttrs.atime << ":" << value.fileAttrs.mtime << ":" <<value.fileAttrs.filesize << ":" <<std::endl;
            }
            cache_file.close();
        }
        
    }
    //check for nullPTR
    ClientCacheValue* getCacheValue(std::string key) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    void addCacheValue(std::string key, ClientCacheValue value) {
        cache.insert({key,value});
        saveCache();
    }

    void updateCacheValue(const std::string &key, ClientCacheValue &value) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            it->second = value;
            saveCache();
        }
    }

     void deleteCacheValue(const std::string &key) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            cache.erase(key);
            saveCache();
        }
    }

};
