#define FUSE_USE_VERSION 30

#include <config.h>

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unordered_map>
#include "/u/d/h/dhelia/Downloads/grpc/examples/cpp/route_guide/nfs_client.h"
#include "/u/d/h/dhelia/Downloads/grpc/examples/cpp/route_guide/nfs.grpc.pb.h"
#include "/u/d/h/dhelia/Downloads/grpc/examples/cpp/route_guide/nfs.pb.h"
using namespace nfs;


class Maptable {
        std::unordered_map<std::string,int> htmap;

        public:
        void put(std::string key,int value) {

                std::pair<std::string, int> entry(key,value);
                htmap.insert(entry);
        }

        int get(std:: string key) {

                auto t = htmap.find(key);
                if (t == htmap.end()){
                        return -1;
                }

                else{
                        return((t->second));

                }
        }
        int del(std::string key){
                auto t = htmap.find(key);
                if (t == htmap.end()) return 0;
                else{
                        htmap.erase(key);
                        return 1;
                }
        }

};


void copy_stat(Stat st, struct stat *statbuf)
{
        statbuf->st_dev=st.sta_dev();
         statbuf->st_ino=st.sta_ino();
         statbuf->st_mode=st.sta_mode();
         statbuf->st_nlink=st.sta_nlink();
         statbuf->st_uid=st.sta_uid();
         statbuf->st_gid=st.sta_gid();
         statbuf->st_rdev=st.sta_rdev();
         statbuf->st_size=st.sta_size();
         statbuf->st_blksize=st.sta_blksize();
         statbuf->st_blocks=st.sta_blocks();
         statbuf->st_atime=st.sta_atime();
        statbuf->st_mtime=st.sta_mtime();
        statbuf->st_ctime=st.sta_ctime();

}

