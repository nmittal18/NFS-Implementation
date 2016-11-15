#include<unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unordered_map>
#include <unistd.h>
#include <grpc++/grpc++.h>
#include "nfs.grpc.pb.h"
#define MAX_PATH 100
#define BUFFER_SIZE 4096
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using nfs::Request;
using nfs::Response;
using nfs::readDataResponse;
using nfs::writeDataRequest;
using nfs::communication;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;

using namespace nfs;

/////////Copy stat function ////////////
void copyStat(struct Stat* fs, struct stat* buf)
{
        fs->set_sta_ino(buf->st_ino);
        fs->set_sta_mode(buf->st_mode);
        fs->set_sta_nlink(buf->st_nlink);
        fs->set_sta_uid(buf->st_uid);
        fs->set_sta_gid(buf->st_gid);
        fs->set_sta_rdev(buf->st_rdev);
        fs->set_sta_size(buf->st_size);
        fs->set_sta_blksize(buf->st_blksize);
        fs->set_sta_blocks(buf->st_blocks);
        fs->set_sta_atime(buf->st_atime);
        fs->set_sta_mtime(buf->st_mtime);
        fs->set_sta_ctime(buf->st_ctime);

}



// class Hashtable {
//         std::unordered_map<void *, void *> htmap;
// 
//         public:
//         void put(void *key, void *value) {
//	 
//                 std::pair<void *, void*> entry((void *)key,(void *)value);
//                 htmap.insert(entry);
//         }
// 
//         void* get(void *key) {
///*                 auto t = htmap.find(key);
//                 if (t == htmap.end()) return NULL;
//                 else{
//                         return(t->second);
// 
//                 }
//
//*/
//		          for(auto kv : htmap) {
//                          if(strcmp((char *)kv.first,(char *)key)==0)
//                        {
//				printf("value %s\n",(char *)kv.second); 
//                                return (kv.second);
//				
//                        }
//
//
//}
//return NULL;
//
//	
//         }
//         int del(void *key){
//                 auto t = htmap.find(key);
//                 if (t == htmap.end()) return 0;
//                 else{
//                         htmap.erase(key);
//                         return 1;
//                 }
//         }
// 
// };


class Ht_fh_path {
         std::unordered_map<std::string,std::string> htmap;
 
         public:
         void put(std::string key, std::string value) {
///		printf("HASHTABLE: put key = %s value %s\n", key.c_str(), value.c_str()); 
                 std::pair<std::string, std::string> entry(key,value);
                 htmap.insert(entry);
//		          for(auto kv : htmap) {
//                          printf("HASHTABLE: PUT keys = %s, value = %s \n", kv.first.c_str(), kv.second.c_str() );
//			}
         }
 


         std::string get(std::string key) {
		          for(auto kv : htmap) {
   //                       printf("HASHTABLE: get keys = %s, value = %s \n", kv.first.c_str(), kv.second.c_str() );
		//	    if(kv.first.compare(key)==0)
		if(strcmp((kv.first).c_str(), key.c_str())==0)
                        {
                          
                             return (kv.second);
                        }


		}
		return "Fail" ;
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

class Ht_fh_fd {
         std::unordered_map<std::string,int> htmap;
 
         public:
         void put(std::string key, int value) {
 
                 std::pair<std::string, int> entry(key,value);
                 htmap.insert(entry);
         }  

         int get(std::string key) {
                /* auto t = htmap.find(key);
                 if (t == htmap.end()) return 0;
                 
		else{
                    return(t->second);

                 }*/

		          for(auto kv : htmap) {
                          if(strcmp((kv.first).c_str(),key.c_str())==0)
                        {
				//printf("fd matched for %s %d \n",(char *)key.c_str(),kv.second);
                                return ( kv.second);
                        }


}
return -1;

         }
         int del(std:: string key){
                 auto t = htmap.find(key);
                 if (t == htmap.end()) return 0;
                 else{
                         htmap.erase(key);
                         return 1;
                 }
         }

 };




