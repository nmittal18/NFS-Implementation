/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributio:wqns of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
//TODO
/*While deleting files/directories increment generation number in iGTable, and delete corresponding filehandles from the table.*/
#include </home/vish.dhelia/grpc/examples/cpp/route_guide/nfs_server.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <mutex>          // std::mutex
std:: mutex mtx;
Ht_fh_fd fd_ht;
Ht_fh_path ht;
FILE* backupfile;
int backupfd;
FILE* replaycacheFile;
int replaycachefd;
//Hashtable igtable
std::string get_filesystempath(int volumeID);
/**********************************RECURSIVE LOOKUP********************************/
std::string recursive_lookup(std::string fs_path,int inodenum)
        {
                std::string returned_path="not_found";
                std::string placeholder;
                DIR *dir;
                int ret;
                struct dirent *ent;
                printf("dir  = %s inodenum = %d\n", fs_path.c_str(), inodenum);
                  struct stat *buf = (struct stat*)malloc(sizeof(struct stat));

                if ((dir = opendir ((char *)fs_path.c_str())) != NULL) {

                while ((ent = readdir (dir)) != NULL) {
                        if((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)){
                                 placeholder=fs_path+"/"+std::string(ent->d_name);
                                ret = stat(placeholder.c_str(), buf);
                                if (buf->st_ino==inodenum)
                                {
                                         returned_path=placeholder;
                                        return returned_path;
                                }
                                else
                                {
                                         std::string new_path=fs_path+"/"+std::string(ent->d_name);
                                         returned_path=recursive_lookup(new_path,inodenum);
                                         if(returned_path.compare("not found")!=0)
                                        {
                                                return(returned_path);
                                        }
                                }
                        }
                }
         }
        returned_path="not found";
        return(returned_path);
        }
/******************************************************RENAME******************************************************/
void NfsRename(const Request * request, Response * response)
{
        printf("Entered remove") ;
        std::string fH_key;
        char* dirPath= new char[MAX_PATH];
        DIR *dir;
        struct dirent *ent;
        fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum())     + "#";
        std::string path = ht.get(fH_key);
        std::string fH_key2;
        fH_key2 = std::to_string((request->fh2()).volumeid()) + "#" + std::to_string((request->fh2()).inodeno()) + "#" + std::to_string((request->fh2()).generationnum())     + "#";
        std::string path_2 = ht.get(fH_key2);
  /*           if(path.compare("Fail")==0)
              {
                      std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
                      std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
                      if(returned_path.compare("not found")==0)
                      {
                              path = "Fail";
                      }
                      else
                      {
                              path=returned_path;
			      mtx.lock();
                              ht.put(fH_key,returned_path);
                              fprintf(backupfile,"%s\t%s\n",(char *)fH_key.c_str(),(char *)returned_path.c_str());
                              fsync(backupfd);
			      mtx.unlock();
                      }
              }
          if(path_2.compare("Fail")==0)
              {
                      std::string filesystem_path = get_filesystempath((request->fh2()).volumeid());
                      std::string returned_path = recursive_lookup(filesystem_path,(request->fh2()).inodeno());
                      if(returned_path.compare("not found")==0)
                      {
                              path_2 = "Fail";
                      }
                      else
                      {
                              path_2=returned_path;
                              ht.put(fH_key,returned_path);
                              fprintf(backupfile,"%s\t%s\n",(char *)fH_key2.c_str(),(char *)returned_path.c_str());
                              fsync(backupfd);
                       }
              }*/
        dirPath=(char *)path.c_str();
        printf("The directory path is %s\n",dirPath) ;
                                           std::string pathcopy=path+"/"+request->name();            
                                      struct stat *buf=(struct stat*)malloc(sizeof(struct stat));
					               stat(pathcopy.c_str(),buf);
                                             int fileno;                                  

 if (S_ISREG(buf->st_mode)) {
                                        fileno = open(pathcopy.c_str(), O_RDONLY);
                                        }
                                        else
                                        {
                                        DIR * dr=opendir(pathcopy.c_str());
                                        fileno=dirfd(dr);
                                        }
                                                long int generation = 0;
                                                if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
                                                       
                                                        printf("errno: %d\n", errno);
                                                }
                                        close(fileno);
					std :: string keytoremove=std::to_string((request->fh()).volumeid())+"#"+std::to_string(buf->st_ino)+"#"+std::to_string(generation) + "#";
					mtx.lock();
					ht.del(keytoremove);
					mtx.unlock();
        if(strcmp(dirPath,"Fail")!=0 && path_2.compare("Fail")!=0)
        {
                if ((dir = opendir(dirPath)) != NULL) {
                        while ((ent = readdir (dir)) != NULL) {
                                if (strcmp(ent->d_name,(request->name()).c_str())==0);
                                {
                                        std::string filepath = std::string(path) + "/" + request->name();
                                         std :: string new_path=path_2 + "/" + request->name2();
                                       	//struct stat * buf = (struct stat *)malloc(sizeof(struct stat));
					int ret=rename(filepath.c_str(),new_path.c_str());
					ret=stat(new_path.c_str(),buf);
					
					if (S_ISREG(buf->st_mode)) {
					fileno = open(filepath.c_str(), O_RDONLY);
					}
					else
					{
					DIR * dr=opendir(filepath.c_str());
					fileno=dirfd(dr);
					}
                                                long int generation = 0;
                                                if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
				
                                                        printf("errno: %d\n", errno);
                                                }
					close(fileno);
					std :: string key=std::to_string((request->fh2()).volumeid())+"#"+std::to_string(buf->st_ino)+"#"+std::to_string(generation) + "#";
					printf("File Handle of renamed file %s\n",(char *)key.c_str());
					mtx.lock();
					ht.put(key,new_path);
					//fprintf(backupfile,"%s\t%s\n",(char *)key.c_str(),(char *)new_path.c_str());
                              		//fsync(backupfd);
					mtx.unlock();
                                        response->set_status(ret);
                                        response->set_err(errno);
                                        printf("rename completed %d\n", ret);
                                        break;
                                        //TODO add logic to remove entry from hashtable : to remove entries from hash table we need path->fh 
                                }
                        }
                }
                closedir(dir);
        }
	else
	{
                response->set_status(-1);
                response->set_err(9);
                response->set_errormsg("File Handle incorrect");

	}
}
/**********************GET_FILESYSTEM_PATH***************************************/
std::string get_filesystempath(int volumeID)
{
	int vID,iN,iGN;
	FILE * fp=fopen("Filesystem.txt","r");
	char * path=new char[MAX_PATH];
	std :: string filesystem_path;
	while(!feof(fp)){
		fscanf(fp,"%d\t%d\t%d\t%s\n",&vID,&iN,&iGN,path);
		if(vID==volumeID)
		{
			path[strlen(path)-1] = 0;
			filesystem_path=std::string(path);
			return(filesystem_path);
		}
	}
	filesystem_path="not found";
	return(filesystem_path);
}

/**********************************RECURSIVE LOOKUP********************************/


/*std::string recursive_lookup(std::string fs_path,std::string name)
{
	std::string returned_path="nothing";
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir ((char *)fs_path.c_str())) != NULL) {

		while ((ent = readdir (dir)) != NULL) {
			if (strcmp(ent->d_name,name.c_str())==0)
			{
				returned_path=fs_path+"/"+name;
				return returned_path;
			}
			else if((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0))
			{
				std::string new_path=fs_path+"/"+std::string(ent->d_name);
				returned_path=recursive_lookup(new_path,name);
				if(returned_path.compare("not found")!=0)
				{
					return(returned_path);
				}
			}
		}
	}
	returned_path="not found";
	return(returned_path);
}
*/
/***********************************************Generation Number****************************/
/*int get_gennumber()
{
	
}*/
/*********************************READDIR********************************************************************************/
void NfsReaddir(const Request* request, readDataResponse* response, ServerWriter<readDataResponse>* writer)
{
	printf("Readdir called");
	struct stat *buf;
	buf = (struct stat *)malloc(sizeof(struct stat));
	DIR *dir;
	struct dirent *ent;
	std::string str = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum()) + "#";
	//std::string new_path = "/u/d/h/dhelia/Documents";
	std::string new_path = ht.get(str);
	printf("reading dir = %s\n", new_path.c_str());

/*	if(new_path.compare("Fail")==0)
	{
		std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
		std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
		if(returned_path.compare("not found")==0)
		{
			new_path = "Fail";
		}
		else
		{
			new_path=returned_path;
			mtx.lock();
			ht.put(str,returned_path);
			//                      int backupfd=open("HashMap.txt",O_APPEND);
			fprintf(backupfile,"%s\t%s\n",(char *)str.c_str(),(char *)returned_path.c_str());
			fsync(backupfd);
			//                      close(backupfd);
			mtx.unlock();
		}
	} 
*/
	if(new_path.compare("Fail")!=0)
	{
		std::string contents;
		if ((dir = opendir(new_path.c_str())) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				//printf("READDIR : context = %s\n", ent->d_name);
				contents=contents+std::string(ent->d_name)+"#";
			}
			response->set_status(0);
			//printf("READDIR sending data = %s", contents.c_str());
			response->set_data((char *)contents.c_str());
			writer->Write(*response);
		}
		else
		{
			response->set_status(-1);
			response->set_err(errno);
		}
		closedir (dir);
	}
	else
	{
		response->set_status(-1);
		response->set_err(9);
		response->set_errormsg("File Handle incorrect");

	}



}


//----------------------------------------------LOOKUP --------------------------------------------------------------------------------------------------------


//Response Lookup(const Request* request,Response* response)
void Lookup(const Request* request,Response* response)
{
	//Form the key to the Hashtable which has filehandle->path mapping : filehandle is the key which is inode+volumeid+generation no.
	//if(check_generation_number())//TODO

	printf("lookup function called\n");
	struct stat *buf;
	buf = (struct stat *)malloc(sizeof(struct stat));
	DIR *dir;
	struct dirent *ent;
	std::string str = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum()) + "#";
	printf("LOOKUP parent filehandle key to look for %s\n",str.c_str());
	std::string path = ht.get(str);
	int flag=0;

	if(path.compare("Fail")!=0)
	{ 
		char* count_path = (char*)path.c_str();
		if(count_path[strlen(count_path)-1]=='/')
		{
			count_path[strlen(count_path)-1]=0;
		}	// From the path we look the file in that path and form a new file handle
		//std::string path = "/u/d/h/dhelia/Downloads";
		printf("LOOKUP parent path = %s  name to look for = %s\n",count_path,request->name().c_str());
		if ((dir = opendir(count_path)) != NULL) {  
			//printf("inside\n");

			while ((ent = readdir (dir)) != NULL) {  

				if (strcmp(ent->d_name,(request->name()).c_str())==0);
				{  

					std::string fullpath = std::string(count_path) + "/" + request->name();
					int ret=stat(fullpath.c_str(),buf);
					response->set_status(ret);
					response->set_err(errno);
					//printf("lookup found path %s\n",ent->d_name);
					if(ret >= 0)
					{
						int inode_no=buf->st_ino;
						int volumeID=(request->fh()).volumeid();
						std::string partial_fh = std::to_string(inode_no) + "#" + std::to_string(volumeID) + "#";
						int fileno = open(fullpath.c_str(), O_RDONLY);
						long int generation = 0;
						if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
							printf("errno: %d\n", errno);
						}
						//printf("generation: %d\n", generation);
						//	if(gen_num=iGTable.get(partial_fh)==NULL)
						//	{
						//		*gen_num=0;
						//		iGTable.put();	
						//	}	

						fileHandle* fh = new fileHandle();
						fh->set_generationnum(generation);
						fh->set_inodeno(inode_no);
						fh->set_volumeid(volumeID);
						response->set_allocated_fh(fh);
						Stat* fs = new Stat();
						copyStat(fs, buf);
						char * triy= new char();
						triy = (char *)request->name().c_str();
						int l=strlen(triy);
						if(triy[0]!='.'){
							std::string str1 = std::to_string(fh->volumeid()) + "#" + std::to_string(fh->inodeno()) + "#" + std::to_string(fh->generationnum()) + "#";
							mtx.lock();
							ht.put(str1,fullpath);
						//	fprintf(backupfile,"%s\t%s\n",(char *)str1.c_str(),(char *)fullpath.c_str());
						//	fsync(backupfd);
							mtx.unlock();
						}
						printf("Done lookup\n");
						response->set_allocated_attr(fs);
						flag=1;
					}
				}
			}

			closedir (dir);
		}
		if(flag==0)
		{
			fileHandle* fh = new fileHandle();
			fh->set_generationnum(0);
			fh->set_inodeno(0);
			fh->set_volumeid(0);
			response->set_allocated_fh(fh);

		}

	}
}



/****************************************CREATE******************************/
void NfsCreate(const Request * request, Response * response)
{

	printf("/***************************************************************************************************************************************************Create called\n");
	struct stat *buf;
	buf = (struct stat *)malloc(sizeof(struct stat));
	std::string str;
	str = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum()) + "#";

	printf("NFS create : parent filehandle key to look for %s\n",str.c_str());
	// std::string new_path = "/u/d/h/dhelia/Documents";
	std::string new_path = ht.get(str);

	fileHandle* fh = new fileHandle();
	int inode_no=0;
	int volumeID=0;
	long int gen_num=0;
	printf("inside create %s     name %s\n",new_path.c_str(),request->name().c_str());
/*		if(new_path.compare("Fail")==0)
		{
			std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
			std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
			if(returned_path.compare("not found")==0)
			{
				new_path = "Fail";
			}
			else
			{
				new_path=returned_path;
				mtx.lock();
				ht.put(str,returned_path);
				//                      int backupfd=open("HashMap.txt",O_APPEND);
				fprintf(backupfile,"%s\t%s\n",(char *)str.c_str(),(char *)returned_path.c_str());
				fsync(backupfd);
				mtx.unlock();
				//                      close(backupfd);
			}
		} 
*/

	if(new_path.compare("Fail")!=0)
	{
		std::string fpath = new_path + "/" + request->name();
		printf("new file to be created =  %s\n", fpath.c_str());
		//		int ret=creat(path,(request->attr()).sta_mode());
		int ret=open(fpath.c_str(),O_CREAT|O_WRONLY|O_EXCL,0777);
		printf("CREATE : ret from open =  %d, err = %d\n", ret, errno);
		response->set_status(ret);
		if(ret <0)
		{
			response->set_err(errno);
		}
		else
		{
		//	mtx.lock();
                     //   fd_ht.put(fpath,ret);
                  //      mtx.unlock();*/
			stat(fpath.c_str(),buf);
			inode_no=buf->st_ino;
			volumeID=(request->fh()).volumeid();
			int fileno = open(fpath.c_str(), O_RDONLY);
                        long int generation = 0;
						if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
                                                        printf("errno: %d\n", errno);
                                                }
                                                printf("generation: %ld\n", generation);
			close(fileno);
			gen_num = generation;
			Stat* fs = new Stat();
			copyStat(fs, buf);
			response->set_allocated_attr(fs);
			std::string str1 = std::to_string(volumeID) + "#" + std::to_string(inode_no) + "#" + std::to_string(gen_num) + "#";
			printf("File handle indoe no = %d for new file = %s\n", inode_no, fpath.c_str());
			mtx.lock();
			ht.put(str1,fpath);
                        fd_ht.put(str1,ret);
			//fprintf(backupfile,"%s\t%s\n",(char *)str1.c_str(),(char *)fpath.c_str());
		//	fsync(backupfd);
			mtx.unlock();

		}
		fh->set_generationnum(gen_num);
		fh->set_inodeno(inode_no);
		fh->set_volumeid(volumeID);
		response->set_allocated_fh(fh);
	}
	else
	{
		response->set_status(-1);
		response->set_err(9);
		response->set_errormsg("File Handle incorrect");
	}
}



//------------------------ READ -------------------------------------------------------------------------------------------------


void NfsReadRequest(const Request* request, readDataResponse* response, ServerWriter<readDataResponse>* writer) {

	std::string fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum()) + "#";
	int fd=fd_ht.get(fH_key);
	std::string path = ht.get(fH_key);
	
/*		        if(path.compare("Fail")==0)   
			  {
			  std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
			  std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
			  if(returned_path.compare("not found")==0)
			  {
			  path = "Fail";
			  }
			  else
			  {
			  path=returned_path;
				mtx.lock();
			  ht.put(fH_key,returned_path);
			fprintf(backupfile,"%s\t%s\n",(char *)fH_key.c_str(),(char *)returned_path.c_str());
			fsync(backupfd);
			mtx.unlock();
			}
		}
*/
	if(path.compare("Fail")!=0) {
			if(fd==-1){
			fd=open(path.c_str(),O_RDWR);//TODO put fd
			mtx.lock();
			fd_ht.put(path,fd);
			mtx.unlock();
			
}
			printf("open call succeded with fd = %d\n", fd);	
		char * buf = new char[request->count()];
		response->set_status(pread(fd,buf,request->count(),request->offset()));
		printf("pread call succeded with data read = %s\n", (char*)buf);
		response->set_err(errno);
		response->set_data(buf);
		writer->Write(*response);
		printf("writer done \n");
		struct stat *statbuf=(struct stat*)malloc(sizeof(struct stat));
		stat(path.c_str(),statbuf);
		Stat* fs = new Stat();
		copyStat(fs, statbuf);
		response->set_allocated_attr(fs);
		free(statbuf);
		}
	else
	{
		response->set_status(-1);
                response->set_err(9);
                response->set_errormsg("File Handle incorrect");		
	}
	
}

//--------------------------------------------WRITE-----------------------------------------------------------------------------

void NfsWriteRequest(writeDataRequest* request,Response* response, ServerReader<writeDataRequest>* reader) {
	static int buffer_size=0;
	std::string path;
	//streaming code
	while (reader->Read(request)) {}

	std::string fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum())     + "#";
	int fd=fd_ht.get(fH_key);
	printf("HEllo %d \n",fd);	
		path = ht.get(fH_key);
/*		 if(path.compare("Fail")==0)
                          {
                          std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
                          std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
                          if(returned_path.compare("not found")==0)
                          {
                          path = "Fail";
                          }
                          else
                          {
                          path=returned_path;
                                mtx.lock();
                          ht.put(fH_key,returned_path);
                        fprintf(backupfile,"%s\t%s\n",(char *)fH_key.c_str(),(char *)returned_path.c_str());
                        fsync(backupfd);
                        mtx.unlock();
                        }
                }
*/
		if(path.compare("Fail")!=0) 
		{
			printf("WRITE : file path to be written %s has fd = %d\n", path.c_str(), fd);
			if(fd==-1)
			{
			 fd = open(path.c_str(),O_RDWR);
			printf("NEW FD %d\n",fd);
			 mtx.lock();
                        fd_ht.put(path,fd);
                        mtx.unlock();
			}
			printf("WRITE open call succeded with fd = %d\n", fd);
			 
	
	int ret =pwrite(fd,(void*)(request->data().c_str()),request->count(),request->offset());
        //buffer_size+=ret;
        response->set_status(ret);
        if(ret == -1)
                response->set_err(errno);
        printf(" pwrite finished with buffer size = %d\n", buffer_size);
        response->set_username(request->username());
        struct stat *statbuf=(struct stat*)malloc(sizeof(struct stat));
        stat(path.c_str(),statbuf);
        Stat* fs = new Stat();
        copyStat(fs, statbuf);
        response->set_allocated_attr(fs);
        free(statbuf);
		}
		else 
		{
			response->set_status(-1);
			response->set_errormsg("Invalid File Hanlde");
		}


	/*int ret =pwrite(fd,(void*)(request->data().c_str()),request->count(),request->offset());
	//buffer_size+=ret;
	response->set_status(ret);
	if(ret == -1)
		response->set_err(errno);
	printf(" pwrite finished with buffer size = %d\n", buffer_size);
	response->set_username(request->username());
	struct stat *statbuf=(struct stat*)malloc(sizeof(struct stat));
	stat(path.c_str(),statbuf);
	Stat* fs = new Stat();
	copyStat(fs, statbuf);
	response->set_allocated_attr(fs);
	free(statbuf);*/
}


//--------------------------------------------WRITE-----------------------------------------------------------------------------

void NfsCommit(const Request* request ,Response* response) {
	static int buffer_size=0;
	std::string path;
	std::string fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum())     + "#";
	int fd=fd_ht.get((char*)fH_key.c_str());
	if(fd==-1)
	{
		path = ht.get(fH_key);
		if(path.compare("Fail")!=0)
		{
			fd = open(path.c_str(),O_RDWR);
			printf("open call succeded with fd = %d\n", fd);
		}
		else
		{
			response->set_status(-1);
			response->set_errormsg("Invalid File Handle");
		}
	}

	int ret =fsync(fd);
	//buffer_size+=ret;
	response->set_status(ret);
	if(ret == -1)
		response->set_err(errno);
	printf(" commit finished with buffer size = %d\n", buffer_size);

	response->set_username(request->username());
}



//--------------------- MKDIR -------------------------------------------------
//int mkdir(const char *pathname, mode_t mode);
//mkdir(dirfh, name,attr) returns (fh, newattr)

void NfsMkdir(const Request* request,Response* response) {
	printf("Entered mkdir \n") ;
	std::string fH_key;
	fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum())     + "#";
	char* dirPath = new char[MAX_PATH];
	printf("MKDIR The request key is %s \n", fH_key.c_str()) ;
	std::string new_path = ht.get(fH_key);
/*	if(new_path.compare("Fail")==0)
                {
                        std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
                        std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
                        if(returned_path.compare("not found")==0)
                        {
                                new_path = "Fail";
                        }
                        else
                        {
                                new_path=returned_path;
                                mtx.lock();
                                ht.put(fH_key,returned_path);
                                //                      int backupfd=open("HashMap.txt",O_APPEND);
                                fprintf(backupfile,"%s\t%s\n",(char *)fH_key.c_str(),(char *)returned_path.c_str());
                                fsync(backupfd);
                                mtx.unlock();
                                //                      close(backupfd);
                        }
                }*/
	dirPath=(char *)new_path.c_str();
	//TODO make sure you send parent directory's path ///
	//path = "/u/d/h/dhelia/Documents";
	printf("The path is %s\n",dirPath) ;
	//TODO use recursive inode lookup code from read function
	if(strcmp(dirPath,"Fail")!=0)
	{
		struct stat *buf;
		buf = (struct stat *)malloc(sizeof(struct stat));
		std::string newDirPath = std::string(dirPath) + "/" + request->name();
		printf("MKDIR The newDirPath is %s \n", newDirPath.c_str() ) ;
		stat(dirPath,buf);
		mode_t mode=buf->st_mode;
		//mkdir system call
		int ret = mkdir(newDirPath.c_str(), mode);
		response->set_status(ret);
		printf("mkdir completed %d\n", ret) ;

		stat(newDirPath.c_str(),buf);
		int inode_no=buf->st_ino;
		int volumeID=(request->fh()).volumeid();
		std::string partial_fh = std::to_string(inode_no) + "#" + std::to_string(volumeID) + "#";
		DIR * dirnew = opendir(newDirPath.c_str());
		int fileno=dirfd(dirnew);
                                                long int generation = 0;
						if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
                                                        printf("errno: %d\n", errno);
                                                }
                                                printf("generation: %ld\n", generation);
		close(fileno);
		closedir(dirnew);
		long int gen_num = generation;
		//      if(gen_num=iGTable.get(partial_fh)==NULL)
		//      {
		//              *gen_num=0;
		//              iGTable.put();  
		//      }       


		//updating the hashtable for new directory file handle
		fileHandle* fh = new fileHandle();
		fh->set_generationnum(gen_num);
		fh->set_inodeno(inode_no);
		fh->set_volumeid(volumeID);
		std::string str1 = std::to_string(fh->volumeid()) + "#" + std::to_string(fh->inodeno()) + "#" + std::to_string(fh->generationnum()) + "#";
		ht.put((char*)str1.c_str(),(char*)newDirPath.c_str());
		//fprintf(backupfile,"%s\t%s\n",(char *)str1.c_str(),(char *)newDirPath.c_str());
		//fsync(backupfd);

		response->set_allocated_fh(fh);
		Stat* fs = new Stat();
		copyStat(fs, buf);
		response->set_allocated_attr(fs);
	}
	else 
	{
		response->set_status(-1);
                response->set_err(9);
                response->set_errormsg("File Handle incorrect");

	}
}

//--------------------- REMOVE -------------------------------------------------
//int remove(const char *pathname);
//remove(dirfh, name) returns (status)
void NfsRemove(const Request* request,Response* response) {
	printf("Entered remove") ;
	std::string fH_key;
	char* dirPath= new char[MAX_PATH];
	DIR *dir;
	struct dirent *ent;

	fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum())     + "#";
	std::string path = ht.get(fH_key);
	//std::string path = "/u/d/h/dhelia/Documents";
/*		if(path.compare("Fail")==0)
		{
			std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
			std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
			if(returned_path.compare("not found")==0)
			{
				path = "Fail";
			}
			else
			{
				path=returned_path;
				ht.put(fH_key,returned_path);
	
				//                      int backupfd=open("HashMap.txt",O_APPEND);
				fprintf(backupfile,"%s\t%s\n",(char *)fH_key.c_str(),(char *)returned_path.c_str());
				fsync(backupfd);
	//			//                      close(backupfd);
			}
		} 
*/
	dirPath=(char *)path.c_str();

	printf("The directory path is %s\n",dirPath) ;
	if(strcmp(dirPath,"Fail")!=0)
	{
		if ((dir = opendir(dirPath)) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				if (strcmp(ent->d_name,(request->name()).c_str())==0);
				{
					std::string filepath = std::string(path) + "/" + request->name();
					struct stat *buf = (struct stat*)malloc(sizeof(struct stat));
                                        int ret=stat(filepath.c_str(),buf);
                                        int fileno =open(filepath.c_str(),O_RDONLY) ;
                                               long int generation = 0;
                                                if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
                                                        printf("errno: %d\n", errno);
                                                }
                                                printf("generation: %ld\n", generation);
                                        close(fileno);
					ret = remove(filepath.c_str());
					//TODO : removing from hashtable: Vishakha : dont have path to FH mapping
					response->set_status(ret);
					response->set_err(errno);
					  if(ret==0)
                                        {

                                                std::string keyToBeDeleted=std::to_string((request->fh()).volumeid()) + "#" + std::to_string(buf->st_ino) + "#" + std::to_string(generation)     + "#";
                                                mtx.lock();
                                                ht.del(keyToBeDeleted);
                                                //fprintf(backupfile,"%s\t%s\n",(char *)keyToBeDeleted.c_str(),"Delete");
                                                //fsync(backupfd);
                                                mtx.unlock();
                                        }

					printf("remove completed %d\n", ret);
					break;
					//TODO add logic to remove entry from hashtable : to remove entries from hash table we need path->fh 
				}
			}
		}
		closedir(dir);
	}
}



//--------------------- RMDIR -------------------------------------------------
//int rmdir(const char *pathname);
//rmdir(dirfh, name) returns (status)
void NfsRmdir(const Request* request,Response* response) {
	printf("Entered rmdir") ;
	std::string fH_key;
	char* dirPath= new char[MAX_PATH];
	DIR *dir;
	struct dirent *ent;

	fH_key = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum())     + "#";
	std::string path = ht.get(fH_key);
	//std::string path = "/u/d/h/dhelia/Documents";
/*		if(path.compare("Fail")==0)
		{
			std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
			std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
			if(returned_path.compare("not found")==0)
			{
				path = "Fail";
			}
			else
			{
				path=returned_path;
				mtx.lock();
				ht.put(fH_key,returned_path);
				//                      int backupfd=open("HashMap.txt",O_APPEND);
				fprintf(backupfile,"%s\t%s\n",(char *)fH_key.c_str(),(char *)returned_path.c_str());
				fsync(backupfd);
				mtx.unlock();
				//                      close(backupfd);
		}
		} 
*/
	dirPath=(char *)path.c_str();
	printf("The directory path is %s name = %s\n",dirPath,request->name().c_str()) ;
	if(path.compare("Fail") != 0)
	{
		if ((dir = opendir(dirPath)) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				if (strcmp(ent->d_name,(request->name()).c_str())==0);
				{
					std::string dirToBeDeleted = std::string(path) + "/" + request->name();
					//TODO call stat and delete entry from hash table
					struct stat *buf = (struct stat*)malloc(sizeof(struct stat));
					int ret=stat(dirToBeDeleted.c_str(),buf);	
					DIR * dirnew=opendir(dirToBeDeleted.c_str());
					int fileno =dirfd(dirnew) ;
                                                long int generation = 0;
                                                if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
                                                        printf("errno: %d\n", errno);
                                                }
                                                printf("generation: %ld\n", generation);
			                close(fileno);	
					closedir(dirnew);
					ret = rmdir(dirToBeDeleted.c_str());
					response->set_status(ret);
					response->set_err(errno);
					if(ret==0)
					{
	
						std::string keyToBeDeleted=std::to_string((request->fh()).volumeid()) + "#" + std::to_string(buf->st_ino) + "#" + std::to_string(generation)     + "#";
						mtx.lock();
						ht.del(keyToBeDeleted);
						//fprintf(backupfile,"%s\t%s\n",(char *)keyToBeDeleted.c_str(),"Delete");
                                		//fsync(backupfd);
						mtx.unlock();
					}
					free(buf);
					printf("rmdir completed %d\n", ret) ;
					break;
					//TODO add logic to remove entry from hashtable
				}
			}
		}
		closedir(dir);
	}
}

//--------------------- STATFS -------------------------------------------------
//int statvfs(const char *path, struct statvfs *buf);
//statfs(fh) returns (fsstats)
void NfsStatFs(const Request* request,Response* response)
{
	std::string str;
	str = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum()) + "#";
	char* path = new char[MAX_PATH];
	std::string new_path = ht.get(str);
	path=(char *)new_path.c_str();
	struct statvfs *buf = (struct statvfs*)malloc(sizeof(struct statvfs));

	//	if(new_path.compare("Fail")==0)
	//	{
	//		std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
	//		std::string returned_path = recursive_lookup(filesystem_path,request->name());
	//		if(returned_path.compare("not found")==0)
	//		{
	//			new_path = "Fail";
	//		}
	//		else
	//		{
	//			new_path=returned_path;
				
	//			ht.put(str,returned_path);
	//			//                      int backupfd=open("HashMap.txt",O_APPEND);
	//			fprintf(backupfile,"%s\t%s\n",(char *)str.c_str(),(char *)returned_path.c_str());
	//			fsync(backupfd);
	//			//                      close(backupfd);
	//		}
	//	} 

	if(new_path.compare("Fail") != 0) 
	{
		int ret = statvfs(path, buf);
		Statfs* fs = new Statfs();
		fs->set_f_bsize(buf->f_bsize);	 
		fs->set_f_frsize(buf->f_frsize);
		fs->set_f_blocks(buf->f_blocks);
		fs->set_f_bfree(buf->f_bfree);
		fs->set_f_bavail(buf->f_bavail);
		fs->set_f_files(buf->f_files);
		fs->set_f_ffree(buf->f_ffree);
		fs->set_f_favail(buf->f_favail);
		fs->set_f_fsid(buf->f_fsid);
		fs->set_f_flag(buf->f_flag);
		fs->set_f_namemax(buf->f_namemax);
		response->set_allocated_filestatus(fs);
	}
}

void NfsGetAttribute(const Request* request,Response* response)
{
	std::string str = std::to_string((request->fh()).volumeid()) + "#" + std::to_string((request->fh()).inodeno()) + "#" + std::to_string((request->fh()).generationnum()) + "#";
	std::string new_path = ht.get(str);
	char * path =new char[MAX_PATH];
	path=(char *)new_path.c_str();   //TODO fullpath in case of mkdir

	printf("GET ATTRIBUTE  path = %s corresponding to filehandle key = %s\n", path, str.c_str());
	struct stat *buf = (struct stat*)malloc(sizeof(struct stat));
	int ret;

	/*if(new_path.compare("Fail")==0)
	{
		std::string filesystem_path = get_filesystempath((request->fh()).volumeid());
		//std::string returned_path = recursive_lookup(filesystem_path,request->name());
		std::string returned_path = recursive_lookup(filesystem_path,(request->fh()).inodeno());
		if(returned_path.compare("not found")==0)
		{
			new_path = "Fail";
		}
		else
		{
			new_path=returned_path;
			mtx.lock();
			ht.put(str,returned_path);
	      		fprintf(backupfile,"%s\t%s\n",(char *)str.c_str(),(char *)returned_path.c_str());
			fsync(backupfd);
			mtx.unlock();
		}
	} */

	if(new_path.compare("Fail") != 0)
	{
		ret = stat(path, buf);
		response->set_err(errno);
		response->set_status(ret);
		if(ret != 0)
			perror("stat failed\n");
		Stat* fs = new Stat();
		printf("the inode returned by getattr is %ld \n", buf->st_ino ) ;
		fs->set_sta_dev(buf->st_dev);
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
		printf("the mode from Stat fs %u \n", buf->st_mode ) ;
		response->set_allocated_attr(fs);
		printf("the mode from  response %d \n", response->attr().sta_mode() ) ;
	} 
	else 
	{
		printf("Entered else\n");
		ret = stat(path, buf);
		response->set_err(errno);
		response->set_status(ret);
		if(ret != 0)
			perror("stat failed\n");
		printf("errorno from  response %d \n", response->err() ) ;
	}
}


void NfsMount(const Request* request,Response* response)
{
	std::string fs_name=request->name();
	printf("fs-name %s\n", fs_name.c_str());
	int vID,iN,iGN;
	FILE * fp=fopen("Filesystem.txt","r");
	char * path=new char[MAX_PATH];
	while(!feof(fp)){
		fscanf(fp,"%d\t%d\t%d\t%s\n",&vID,&iN,&iGN,path);
		if(strcmp(path,fs_name.c_str())==0)
		{
			printf("volumeID %d\t inode number = %d\t gen num = %d\t  path = %s\n", vID, iN, iGN, path );
			fileHandle* fh = new fileHandle() ;
			fh->set_volumeid(vID);
			fh->set_inodeno(iN);
			fh->set_generationnum(iGN);
			std::string str1 = std::to_string(fh->volumeid()) + "#" + std::to_string(fh->inodeno()) + "#" + std::to_string(fh->generationnum()) + "#";
			std::string rootPath = std::string(path);
			mtx.lock();
			ht.put(str1,rootPath);
	//		fprintf(backupfile,"%s\t%s\n",(char *)str1.c_str(),(char *)rootPath.c_str());
        //		fsync(backupfd);
			mtx.unlock();
			printf("Key of root %s   Path of root %s\n",str1.c_str(),path);
			response->set_allocated_fh(fh);
			struct stat * buf=(struct stat*)malloc(sizeof(struct stat));
			stat(path,buf);
			Stat* fs = new Stat();
			copyStat(fs, buf);
			response->set_allocated_attr(fs);
			response->set_status(1);
			fclose(fp);
			return;
		}
	}
	fclose(fp);
	response->set_errormsg("Mount failed:File System not found");
	response->set_status(-1);
}



/////////-----------------------------------------------END OF SERVER IMPLEMENTATION _------------------------------------------------------------------------------------


class NfsServiceImpl final : public communication::Service {

	Status sendRequest(ServerContext* context, const Request* request,
			Response* response) override {
		if(strcmp((request->commandname()).c_str(),"null") == 0)
		{
			response->set_commandname("null");
			//response->set_allocated_fh("null");
			response->set_status(1);
			//response->set_allocated_attr("null");
		}
		else if(strcmp((request->commandname()).c_str(),"lookup") == 0)
		{
			printf("lookup matched");
			Lookup(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"mount") == 0)
		{
			printf("mount matched \n");
			NfsMount(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"statfs") == 0)
		{
			printf("statfs matched \n");
			NfsStatFs(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"getattr") == 0)
		{
			printf("getattr matched \n");
			NfsGetAttribute(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"mkdir") == 0)
		{
			printf("mkdir matched \n");
			NfsMkdir(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"commit") == 0)
		{
			printf("commit matched \n");
			NfsCommit(request,response);
		}

		else if(strcmp((request->commandname()).c_str(),"remove") == 0)
		{
			printf("remove matched \n");
			NfsRemove(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"rmdir") == 0)
		{
			printf("rmdir matched \n");
			NfsRmdir(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"create") == 0)
		{
			printf("create matched \n");
			NfsCreate(request,response);
		}
		else if(strcmp((request->commandname()).c_str(),"rename") == 0)
                {
                        printf("rename matched \n");
                        NfsRename(request,response);
                }

		else
		{

		}

		return Status::OK;
	}

	Status readRequest(ServerContext* context, const Request* request,
			ServerWriter<readDataResponse>* writer) override {
		/*		printf("read request called\n");
				readDataResponse* reply= new readDataResponse();
				NfsReadRequest(request,reply, writer);
				printf("read request finished\n");
				return Status::OK;*/
		readDataResponse* reply= new readDataResponse();
		if(strcmp((request->commandname()).c_str(),"readdir") == 0)
		{
			printf("readdir called\n");
			NfsReaddir(request,reply,writer);
		}
		else if(strcmp((request->commandname()).c_str(),"read") == 0)
		{
			printf("read request called\n");
			NfsReadRequest(request,reply, writer);
			printf("read request finished\n");
		}
		return Status::OK;



	}

	Status writeRequest(ServerContext* context, ServerReader<writeDataRequest>* reader,
			Response* reply) override {
		printf("write request called\n");
		writeDataRequest* req = new writeDataRequest();
		NfsWriteRequest(req, reply, reader);
		printf("write request finished\n");
		return Status::OK;
	}

};


void RunServer() {
	//std::string server_address("128.105.37.141:50051");
	std::string server_address("0.0.0.0:50051");
	NfsServiceImpl service;

	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}


void fill_hashTable()
{
	FILE * fp=fopen("HashMap.txt","r");
	char * path=new char[MAX_PATH];
	char * key=new char[MAX_PATH];
	while(!feof(fp)){
		fscanf(fp,"%s\t%s\n",key,path);
		if(strcmp(path,"Delete")==0){
		ht.del(std::string(key));
		}
		else{
		ht.put(std::string(key),std::string(path));
		}
	}
	fclose(fp);

}


int main(int argc, char** argv) {

	fill_hashTable();
	/*int fd=open("Users.txt",);
	 */
	backupfile=fopen("HashMap.txt","a+");
	backupfd=fileno(backupfile);
	//replaycachefd=open("ReplayCache.txt",O_CREAT|O_APPEND|O_RDONLY,0777);
	//replaycacheFile=fdopen(replaycachefd,"a+");
	int buffer_size=0;
 int fileno = open("/home/vish.dhelia/Documents", O_RDONLY);
                        long int generation = 0;
                                                if (ioctl(fileno, FS_IOC_GETVERSION, &generation)) {
                                                        printf("errno: %d\n", errno);
                                                }
                                                printf("ROOT generation: %ld\n", generation);
                        close(fileno);

	RunServer();

	return 0;
}


/*
   int main(int argc, char** argv) {

//fill_hashTable(&ht);//TODO 
 *int fd=open("Users.txt",);

 int buffer_size=0;
 RunServer();

 return 0;
 }*/
