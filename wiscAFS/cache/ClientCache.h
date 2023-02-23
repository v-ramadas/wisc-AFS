#pragma once
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <map>
#include <vector>
#include <semaphore.h>
#include "wiscAFS.grpc.pb.h"

using wiscAFS::FileInfo;

class CacheFileInfo{
    public:
    CacheFileInfo() : st_dev(0),
                 st_ino(0),
                 st_mode(0),
                 st_nlink(0),
                 st_uid(0),
                 st_gid(0),
                 st_rdev(0),
                 st_size(0),
                 st_blksize(0),
                 st_blocks(0),
                 st_atim(0),
                 st_mtim(0),
                 st_ctim(0) {
    }

    CacheFileInfo(unsigned int dev, unsigned int ino, unsigned int mode, unsigned int nlink, unsigned int uid, unsigned int gid,unsigned int rdev, unsigned long int size, unsigned long int blksize, unsigned long int blocks, unsigned long int atim, unsigned long mtim, unsigned long int ctim ) : st_dev(dev),
                 st_ino(ino),
                 st_mode(mode),
                 st_nlink(nlink),
                 st_uid(uid),
                 st_gid(gid),
                 st_rdev(rdev),
                 st_size(size),
                 st_blksize(blksize),
                 st_blocks(blocks),
                 st_atim(atim),
                 st_mtim(mtim),
                 st_ctim(ctim) {
    }


    unsigned int st_dev;
    unsigned int st_ino;
    unsigned int st_mode;
    unsigned int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    long unsigned int st_size;
    long unsigned int st_blksize;
    long unsigned int st_blocks;
    long unsigned int st_atim;
    long unsigned int st_mtim;
    long unsigned int st_ctim;

    void getFileInfo(FileInfo* info) {
        info->set_st_dev(this->st_dev);
        info->set_st_ino(this->st_ino);
        info->set_st_mode(this->st_mode);
        info->set_st_nlink(this->st_nlink);
        info->set_st_uid(this->st_uid);
        info->set_st_gid(this->st_gid);
        info->set_st_rdev(this->st_rdev);
        info->set_st_size(this->st_size);
        info->set_st_blksize(this->st_blksize);
        info->set_st_blocks(this->st_blocks);
        info->set_st_atim(this->st_atim);
        info->set_st_mtim(this->st_mtim);
        info->set_st_ctim(this->st_ctim);
    }

    void setFileInfo(const FileInfo* info) {
        this->st_dev = info->st_dev();
        this->st_ino = info->st_ino();
        this->st_mode = info->st_mode();
        this->st_nlink = info->st_nlink();
        this->st_uid = info->st_uid();
        this->st_gid = info->st_gid();
        this->st_rdev = info->st_rdev();
        this->st_size = info->st_size();
        this->st_blksize = info->st_blksize();
        this->st_blocks = info->st_blocks();
        this->st_atim = info->st_atim();
        this->st_mtim = info->st_mtim();
        this->st_ctim = info->st_ctim();
    }
};

class ClientCacheValue {
    public:
    ClientCacheValue(){};
    ClientCacheValue(const CacheFileInfo &file_Attrs,  bool isDirty, int fileDescriptor) : fileInfo(file_Attrs), isDirty(false), fileDescriptor(fileDescriptor)  {};
    CacheFileInfo fileInfo;
    bool isDirty;
    int fileDescriptor;

};


class DiskCache {
    
    const std::string CACHE_FILE =  "/tmp/afs/cache_metadata.txt";
    std::map<std::string, ClientCacheValue> cache;

    public:

    sem_t semaphore;
    int cacheAccess = 1;

    DiskCache() {
        sem_init(&semaphore, 1, 1);
    }

    ~DiskCache() {
        sem_destroy(&semaphore);
    }

    void loadCache() {
        sem_wait(&semaphore);
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
               bool isDirty = std::stoi(substrings[1]);
               int fd = std::stoi(substrings[2]);
               int st_dev = std::stoi(substrings[3]);
               int std_ino = std::stoi(substrings[4]);
               int st_mode = std::stoi(substrings[5]);
               int st_nline = std::stoi(substrings[6]);
               int st_uid = std::stoi(substrings[7]);
               int st_gid = std::stoi(substrings[8]);
               int st_rdev = std::stoi(substrings[9]);
               int st_size = std::stoi(substrings[10]);
               int st_blksize = std::stoi(substrings[11]);
               int st_blocks = std::stoi(substrings[12]);
               int st_atim = std::stoi(substrings[13]);
               int st_mtim = std::stoi(substrings[14]);
               int st_ctim = std::stoi(substrings[15]);
               CacheFileInfo fileInfo(st_dev, std_ino, st_mode, st_nline, st_uid, st_gid, st_rdev, st_size, st_blksize, st_blocks, st_atim, st_mtim, st_ctim);
               ClientCacheValue clientCacheValue(fileInfo, isDirty, fd);
               cache.insert({key, clientCacheValue});
            }
            cache_file.close();
        }
        sem_post(&semaphore);
    }

    void saveCache() {
        std::ofstream cache_file;
        if (cache.empty()){
            std::cout<< "ClientCache: Cache empty, just truncating the file\n";
            cache_file.open(CACHE_FILE, std::ofstream::out | std::ofstream::trunc);
        }
        else{
            cache_file.open(CACHE_FILE);
            if (cache_file.is_open()) {
                for (auto it = cache.begin(); it != cache.end(); it++) {
                    std::string key = it->first;
                    ClientCacheValue value= it->second;
                    cache_file << key << ":" << (value.isDirty ? 1 : 0) << ":" << value.fileDescriptor <<":"<< value.fileInfo.st_dev << ":" << value.fileInfo.st_ino<<":"<<value.fileInfo.st_mode<<":"<<value.fileInfo.st_nlink<<":"<<value.fileInfo.st_uid<<":"<<value.fileInfo.st_gid<<":"<<value.fileInfo.st_rdev<<":"<<value.fileInfo.st_size<<":"<<value.fileInfo.st_blksize<<":" << value.fileInfo.st_blocks<<":"<<value.fileInfo.st_atim << ":" << value.fileInfo.st_mtim << ":" <<value.fileInfo.st_ctim << ":" <<std::endl;
                }
                cache_file.close();
            }
            else{
                std::cout << "ERROR:  Couldn't open cache file\n";
            }
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
        sem_wait(&semaphore);
        cache.insert({key,value});
        saveCache();
        sem_post(&semaphore);
    }

    void updateCacheValue(const std::string &key, ClientCacheValue &value) {
        sem_wait(&semaphore);
        auto it = cache.find(key);
        if (it != cache.end()) {
            it->second = value;
            saveCache();
        }
        sem_post(&semaphore);
    }

     void deleteCacheValue(const std::string &key) {
        sem_wait(&semaphore);
        auto it = cache.find(key);
        if (it != cache.end()) {
            cache.erase(key);
            saveCache();
        }
        sem_post(&semaphore);
    }

};
