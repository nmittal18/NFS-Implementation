#include"nfs_fuse_client.h"
#include <errno.h>
#include <unordered_map>

using namespace std;
int globalLookupResult, globalLookupErr;


void split(const string &s, const char* delim, vector<string> & v){
	// to avoid modifying original string
	// first duplicate the original string and return a char pointer then free the memory
	char * dup = strdup(s.c_str());
	char * token = strtok(dup, delim);
	while(token != NULL){
		v.push_back(string(token));
		// the call is treated as a subsequent calls to strtok:
		// the function continues from where it left in previous invocation
		token = strtok(NULL, delim);
	}
	free(dup);
}


class Hashtable {
	std::unordered_map<std::string, fileHandle> htmap;

	public:
	void put(std::string key, fileHandle value) {
		printf("HASHTABLE put call %s\n", key.c_str());
		std::pair<std::string, fileHandle> entry(key,(fileHandle)value);
		htmap.insert(entry);
	}

	int get(std::string key, fileHandle *fh) {
	printf("HASHTABLE key get %s  \n", key.c_str());
		for(auto kv : htmap) {

			if(strcmp((kv.first).c_str(), key.c_str())==0)
			{        
				fh->set_inodeno(kv.second.inodeno());
				fh->set_generationnum(kv.second.generationnum());
				fh->set_volumeid(kv.second.volumeid());
				return 1;
			}
		}
		return -1;
	}


	int del(std::string key){
		auto t = htmap.find(key);
		if (t == htmap.end()) return 0;
		else
		{
			htmap.erase(key);
			return 1;
		}
	}

};

int nfs_helper(const char * path,fileHandle * fh_path);


std::string mount_path;
int username;
int fd;
int * status;
nfscommunication* client;
int fdcount;
Hashtable pathToFhMap;
Maptable Fh_to_Fd;


std::string get_key(fileHandle *fh)
{
	std :: string S=std::to_string(fh->volumeid())+std::to_string(fh->generationnum()) + std::to_string(fh->inodeno());
	return(S);
}



static int nfs_getattr(const char *path, struct stat *statbuf)
{

	std::string fpath ;
	int ret;
	Request* req=new Request();
	req->set_commandname("getattr");
	req->set_username(username);

	printf("nfs_getattr path = %s\n", path);

	fileHandle* fh = new fileHandle();

	ret = nfs_helper(path, fh);
	if(ret == -1)
	{
		printf("fh not found in hashmap, hence returmed NULL \n");
		fh->set_volumeid(0) ;
		fh->set_inodeno(0) ;
		fh->set_generationnum(0) ; 
	}
	printf("The filhandle entries are %d,%d,%d for path %s\n", fh->volumeid(), fh->inodeno() ,fh->generationnum(), path ) ;

	req->set_allocated_fh(fh);

	//RPC call
	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}	
	}
	printf("The inode recived by client's gettar is %d \n", reply.attr().sta_ino()) ;

	copy_stat(reply.attr(),statbuf);

	ret = reply.status();

	if(ret<0) { 
		errno=reply.err();
		//printf("Errno :%d \n",errno);
		return (-errno) ;
	}
	else
	{
		return ret ;
	}
}


/*int nfs_setattr(const char *path, struct stat *statbuf)
  {
  char fpath[PATH_MAX];
  nfs_fullpath(fpath, path);
  struct fileHandle * fh;
  Request* req=new Request();
  req->set_commandname("getattr");
  req->set_username(username);
  req->set_allocated_fh((pathToFhMap.get((char *)fpath)));
  Response reply = client->sendRequest(*req);
  copy_stat(reply.attr(),statbuf);
  return reply.status();
  }
 */

int nfs_rename(const char *from, const char *to, unsigned int flags)
{
	std::string fpath;
	if (flags)
	{
		return -EINVAL;
	}

	fpath = mount_path + std::string(from) ;
	printf("RENAME fullpath to dir on server side = %s\n", fpath.c_str()) ;
	std::string truncatedPath = fpath;
	int len=truncatedPath.length();
	int i = len-1;
	int pos =len -1;
	char pp[PATH_MAX];
	for(pos=len-1;truncatedPath[pos] != '/';pos--) ;
	int j,k=0;
	for(j=pos+1;fpath[j]!=0;j++)
	{
		pp[k]=fpath[j];
		k++;
	}
	pp[k]=0;
	for(i=len-1;truncatedPath[i] != '/';i--) {
		truncatedPath[i] = 0 ;
	}
	printf("REMOVE trunc dir is %s\n", truncatedPath.c_str()) ;
	std::string root_path = mount_path + "/";
	printf("REMOVE root dir is %s\n", root_path.c_str()) ;
	if(strcmp(truncatedPath.c_str(),root_path.c_str())!=0)
	{
		truncatedPath[i]=0;
		printf("entered inside\n");
	}
	printf("Rename from parent dir is %s\n", truncatedPath.c_str()) ;
	fileHandle* fh = new fileHandle();
	int ret = pathToFhMap.get(truncatedPath, fh) ;
	if(ret == -1) {
		printf("fh not found, so returned null \n");
		fh->set_volumeid(0) ;
		fh->set_inodeno(0) ;
		fh->set_generationnum(0) ;
	}
	fpath = mount_path + std::string(to) ;
	std::string pathforhelper=std::string(to) ;
	int pos2=0,len2=strlen(to);
	printf("RENAME fullpath to dir on server side = %s\n", fpath.c_str()) ;
	truncatedPath = fpath;
	len=truncatedPath.length();
	i = len-1;
	pos =len -1;
	char pp2[PATH_MAX];
	k=0;
	for(pos2=len2-1;pathforhelper[pos2]!='/';pos2--);
	pathforhelper[pos2]=0;
	for(pos=len-1;truncatedPath[pos] != '/';pos--) ;
	for(j=pos+1;fpath[j]!=0;j++)
	{
		pp2[k]=fpath[j];
		k++;
	}
	pp2[k]=0;
	for(i=len-1;truncatedPath[i] != '/';i--) {
		truncatedPath[i] = 0 ;
	}
	printf("Rename trunc dir is %s\n", truncatedPath.c_str()) ;

	if(strcmp(truncatedPath.c_str(),root_path.c_str())!=0)
	{
		truncatedPath[i]=0;
		printf("entered inside\n");
	}
	printf("Rename from parent dir is %s\n", truncatedPath.c_str()) ;
	fileHandle* fh2 = new fileHandle();
	ret = pathToFhMap.get(truncatedPath, fh2) ;
	if(ret==-1){
		ret=nfs_helper(pathforhelper.c_str(),fh2);
		printf("Done with helper");}
	printf("Got File Handle\n");
	if(ret == -1) {
		printf("fh2 not found, so returned null \n");
		fh2->set_volumeid(0) ;
		fh2->set_inodeno(0) ;
		fh2->set_generationnum(0) ;
	}
	Request* req = new Request();
	req->set_commandname("rename");
	req->set_name(pp);
	printf("New name %s",pp2);
	req->set_name2(pp2);
	req->set_username(username);
	printf("Rename The filhandle entries from hashmap are %d,%d,%d", fh->volumeid(), fh->inodeno() ,fh->generationnum()    ) ;
	req->set_allocated_fh(fh);
	req->set_allocated_fh2(fh2);
	printf("Reached till here at least\n");
	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	//pathToFhMap.del(fpath, reply.fh()); TODO delete
	printf("rename status %d", reply.status() ) ;
	ret = 0;
	if(reply.status()<0)
	{
		ret=-reply.err();
	}
	else
	{
		//int test=pathToFhMap.del(fpath);
		ret = reply.status();
	}
	return(ret);
}

void copy_statvfs(Statfs statf,struct statvfs *statv)
{
	statv->f_bsize=statf.f_bsize();
	statv->f_frsize=statf.f_frsize();
	statv->f_blocks=statf.f_blocks();
	statv->f_bfree=statf.f_bfree();
	statv->f_bavail=statf.f_bavail();
	statv->f_files=statf.f_files();
	statv->f_ffree=statf.f_ffree();
	statv->f_favail=statf.f_favail();
	statv->f_fsid=statf.f_fsid();
	statv->f_flag=statf.f_flag();
	statv->f_namemax=statf.f_namemax();

}
int nfs_statfs(const char *path, struct statvfs *statv)
{
	std::string fpath;
	int ret;
	//TODO check if we are getting correct path
	fpath =  mount_path + std::string(path) ;
	printf("STATFS fpath %s\n", fpath.c_str());
	Request* req=new Request();
	req->set_commandname("statfs");
	req->set_username(username);
	fileHandle* fh = new fileHandle();
	ret = pathToFhMap.get(fpath, fh);
	if(ret != -1)
		req->set_allocated_fh(fh);
	else 
		printf("fileHandle not found in hashmap for fpath = %s\n", fpath.c_str());
	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	copy_statvfs(reply.filestatus(),statv);
	ret = reply.status();
	return ret;
}
static int nfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi,
		enum fuse_readdir_flags flags)
{
	Request* req = new Request();
	req->set_commandname("readdir");
	std::string fpath;
	fpath= mount_path + std::string(path) ;
	fileHandle * fh=new fileHandle();
	pathToFhMap.get(fpath,fh);
	printf("readdir got called with path = %s\n", path);//TODO path not found
	req->set_allocated_fh(fh);
	readDataResponse reply = client->readRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->readRequest(*req);
		}
	}

	//printf("readdir data returned = %s\n", reply.data().c_str());//TODO path not found
	//	int length=strlen((char *)reply.data().c_str());
	//	int count=0;
	//	int i;
	//	char * dd=(char *)reply.data().c_str();
	std::string reply_names = reply.data();	
	std::vector<std::string> strs;
	split(reply_names, "#", strs);
	int i ;
	for(i=0 ; i< strs.size() ; i++ ) {
		//printf("READDIR NEHA inter  = %s\n", strs[i].c_str());//TODO path not found
		if (filler(buf, strs[i].c_str(), NULL, 0, (fuse_fill_dir_flags)0))
		{

			return 0;
		}

	}
	//	char inter[PATH_MAX];
	/*	while(count<length)
		{
		for(i=count;dd[i]!='#';i++) // extracting content names and putting it in filler later
		{
		inter[i-count]=dd[i];

		}
		inter[i]=0;
		printf("READDIR NEHA inter  = %s\n", inter);//TODO path not found
		if (filler(buf, inter, NULL, 0, (fuse_fill_dir_flags)0))
		{

		return 0;
		}

		count=count+i;

		}
	 */
	int Ret = 0;
	if(reply.status()<0)
	{
		Ret = -reply.err();
	}
	else
	{
		Ret = 0;
	}
	return Ret;

}


void nfs_lookup(char * break_path,char * ppath,fileHandle * fh_child)
{

	Request* req = new Request();
	fileHandle * fh_parent=new fileHandle();
	req->set_commandname("lookup");
	std::string str(break_path);
	printf(" LOOKUP : nfs_lookup path = %s\n", break_path);
	req->set_name(str);
	req->set_username(username);
	fileHandle* fh = new fileHandle();
	std::string key(ppath);
	int ret = pathToFhMap.get(key, fh_parent);
	if(ret == -1)
	{
		fh_parent->set_inodeno(0);
		fh_parent->set_generationnum(0);
		fh_parent->set_volumeid(0);
	}
	req->set_allocated_fh(fh_parent);
	Response reply = client->sendRequest(*req);
	printf("LOOKUP response recieved with error code = %d\n", reply.grpc_error_code());
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	fh_child->set_inodeno(reply.fh().inodeno());
	fh_child->set_generationnum(reply.fh().generationnum());
	fh_child->set_volumeid(reply.fh().volumeid());
	globalLookupResult = reply.status();
	globalLookupErr = -(reply.err());
}


int count_pathname(const char * path)
{
	int count=0;
	int i;
	for(i=0;path[i]!=0;i++)
	{
		if(path[i]=='/'){
			count++;
		}
	}
	return count;
}

int nfs_open(const char *path, struct fuse_file_info *fi)
{

	printf("Entered OPEN \n") ;
	fi->flags = 00777;
	char breakup_path[4096];
	int index = 1;
	int length=strlen(path);
	int i, ret;
	fileHandle* fh_parent = new fileHandle();
	fileHandle* fh_child = new fileHandle();
	fileHandle* copy_fh = new fileHandle();
	std::string fpath, ppath,cpath ;
	std::string  placeholder;
	placeholder = mount_path + "/" ;
	ppath=mount_path;
	ret = pathToFhMap.get(placeholder, fh_parent);
	int k=0;
	int count = count_pathname(path);
	while(index<length)
	{
		for(i=index;path[i]!='/'&& path[i]!=0;i++)
		{
			breakup_path[i-index]=path[i];
			k++;
		}
		breakup_path[i-index]=0;
		printf("OPEN : File to look for%s\n",breakup_path);
		std::string bpath = std::string(breakup_path);
		cpath = ppath +"/"+ bpath;
		printf("OPEN Child path %s \t Parent path %s\n",cpath.c_str(),ppath.c_str());
		ret = pathToFhMap.get(cpath, fh_child);//Trying to find whether child already exists
		globalLookupResult = 0;

		if(ret==-1)
		{

			nfs_lookup(breakup_path,(char *)placeholder.c_str(),fh_child);
			copy_fh->set_inodeno(fh_child->inodeno());
			copy_fh->set_generationnum(fh_child->generationnum());
			copy_fh->set_volumeid(fh_child->volumeid());
			if(fh_child->inodeno()>0){
				pathToFhMap.put(cpath,*copy_fh);//Put child in HashMap
			}
		}
		fh_parent->set_inodeno(fh_child->inodeno());//For next iteration 
		fh_parent->set_generationnum(fh_child->generationnum());//For next iteration 
		fh_parent->set_volumeid(fh_child->volumeid());//For next iteration 
		ppath=cpath;
		placeholder=ppath;
		index=index+k+1;
		k=0;
		count--;

	}


	int rettt = 0;
	if(globalLookupResult < 0)
	{
		rettt = globalLookupErr;
	}
	else
	{
		rettt = 0;
	}
	return(rettt);
}


/*
   int nfs_open(const char *path, struct fuse_file_info *fi)
   {

   printf("OPEN :Entered open \n") ;
   char breakup_path[4096];
   int index = 1;
   int length=strlen(path);
   int i, ret;
   fileHandle* fh_parent = new fileHandle();
   fileHandle* fh_child = new fileHandle();
   std::string fpath, ppath,cpath ;
//fpath =  mount_path + std::string(path) ;
//nfs_fullpath(fpath,path);
ppath = mount_path + "/" ;
ret = pathToFhMap.get(ppath, fh_parent);
int k=0;
while(index<length)
{
for(i=index;path[i]!='/';i++)
{
breakup_path[i-index]=path[i];
k++;
}
breakup_path[i-index]=0;
printf("File to look for%s\n",breakup_path);
cpath=ppath+std::string(breakup_path);
printf("Child path %s \t Parent path %s\n",cpath.c_str(),ppath.c_str());
ret = pathToFhMap.get(cpath, fh_child);//Trying to find whether child already exists
if(ret==-1)
{ nfs_lookup(breakup_path,(char *)ppath.c_str(),fh_child);
if(fh_child->inodeno()>0){			pathToFhMap.put(cpath,*fh_child);//Put child in HashMap
}}
fh_parent->set_inodeno(fh_child->inodeno());//For next iteration 
fh_parent->set_generationnum(fh_child->generationnum());//For next iteration 
fh_parent->set_volumeid(fh_child->volumeid());//For next iteration 
if(index>1){
ppath=ppath+"/";}
index=index+k+1;
k=0;

}
std::string fh_key=std::to_string(fh_child->volumeid())+"#"+std::to_string(fh_child->inodeno())+"#"+std::to_string(fh_child->generationnum())+"#";
int f=Fh_to_Fd.get(fh_key);
if(f==-1)
{
Fh_to_Fd.put(fh_key,fdcount);
f=fdcount;
fdcount++;
}
fi->fh=f;
}
 */


static int nfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{

	printf("Entered CREATE \n") ;
	char breakup_path[4096];
	int index = 1;
	int length=strlen(path);
	int i, ret;
	fileHandle* fh_parent = new fileHandle();
	fileHandle* fh_child = new fileHandle();
	fileHandle* copy_fh = new fileHandle();
	std::string fpath, ppath,cpath ;
	std::string  placeholder;
	placeholder = mount_path + "/" ;
	ppath=mount_path;
	ret = pathToFhMap.get(placeholder, fh_parent);
	int k=0;
	int count = count_pathname(path);
	while(index<length)
	{
		for(i=index;path[i]!='/'&& path[i]!=0;i++)
		{
			breakup_path[i-index]=path[i];
			k++;
		}
		breakup_path[i-index]=0;
		printf("File to look for%s\n",breakup_path);
		std::string bpath = std::string(breakup_path);
		cpath = ppath +"/"+ bpath;
		printf("Child path %s \t Parent path %s\n",cpath.c_str(),ppath.c_str());
		if(count==1)
		{
			break;
		}
		ret = pathToFhMap.get(cpath, fh_child);//Trying to find whether child already exists
		if(ret==-1)
		{

			nfs_lookup(breakup_path,(char *)placeholder.c_str(),fh_child);
			copy_fh->set_inodeno(fh_child->inodeno());
			copy_fh->set_generationnum(fh_child->generationnum());
			copy_fh->set_volumeid(fh_child->volumeid());
			if(fh_child->inodeno()>0){
				pathToFhMap.put(cpath,*copy_fh);//Put child in HashMap
			}
		}
		fh_parent->set_inodeno(fh_child->inodeno());//For next iteration 
		fh_parent->set_generationnum(fh_child->generationnum());//For next iteration 
		fh_parent->set_volumeid(fh_child->volumeid());//For next iteration 

		if(count==1){
			ppath=cpath;}
		else
		{
			ppath=cpath;
		}
		placeholder=ppath;
		index=index+k+1;
		k=0;
		count--;

	}

	Request* req = new Request();
	req->set_commandname("create");
	req->set_name(std::string(breakup_path));
	req->set_username(username);
	printf("create parent handle %d\n",fh_parent->inodeno());
	req->set_allocated_fh(fh_parent);
	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	if(reply.fh().inodeno()>0)
	{
		pathToFhMap.put(cpath,reply.fh());
	}
	//std::string fh_key=std::to_string(fh_child->volumeid())+"#"+std::to_string(fh_child->inodeno())+"#"+std::to_string(fh_child->generationnum())+"#";
	//	int f=Fh_to_Fd.get(fh_key);
	//	if(f==-1)
	//	{
	//		Fh_to_Fd.put(fh_key,fdcount);
	//		f=fdcount;
	//		fdcount++;
	//	}
	//	fi->fh=f;  //TODO uncomment may be
	int Ret = 0;
	if(reply.status()<0)
	{
		Ret = -reply.err();
	}
	else
	{
		Ret = 0;
	}
	return Ret;
}
int nfs_helper(const char * path, fileHandle * fh_path)
{
	printf("HELPER:Entered helper %s \n",path) ;
	char breakup_path[4096];
	int index = 1;
	int length=strlen(path);
	int i, ret;
	fileHandle* fh_parent = new fileHandle();
	fileHandle* fh_child = new fileHandle();
	fileHandle* copy_fh = new fileHandle();
	std::string ppath,cpath ;
	//fpath =  mount_path + std::string(path) ;
	//nfs_fullpath(fpath,path);
	std::string  placeholder;
	placeholder = mount_path + "/" ;
	ppath=mount_path;

	ret = pathToFhMap.get(placeholder, fh_parent);
	printf( "found mount path %d",ret);
	if(ret ==-1)
	{
		fh_parent->set_inodeno(0);//For next iteration 
		fh_parent->set_generationnum(0);//For next iteration 
		fh_parent->set_volumeid(0);//For next iteration 

	}
	int k=0;
	int count = count_pathname(path);
	std::string bpath;
	printf("Parent path %s\n", ppath.c_str());
	while(index<length)
	{
		for(i=index;path[i]!='/'&& path[i]!=0;i++)
		{
			breakup_path[i-index]=path[i];
			k++;
		}
		breakup_path[i-index]=0;
		printf("File to look for%s\n",breakup_path);
		bpath = std::string(breakup_path);
		cpath = ppath +"/"+ bpath;
		printf("Child path %s \t Parent path %s\n",cpath.c_str(),ppath.c_str());
		/* if(count==0)
		   {
		   break;
		   }*/
		//placeholder=cpath;
		printf("Child path %s \t Parent path %s\n",cpath.c_str(),ppath.c_str());
		ret = pathToFhMap.get(cpath, fh_child);//Trying to find whether child already exists
		if(ret==-1)
		{
			printf("path called %s\n",ppath.c_str());
			nfs_lookup(breakup_path,(char *)placeholder.c_str(),fh_child);
			copy_fh->set_inodeno(fh_child->inodeno());
			copy_fh->set_generationnum(fh_child->generationnum());
			copy_fh->set_volumeid(fh_child->volumeid());
			printf("fhchild inode %d \n", fh_child->inodeno());
			placeholder=cpath;

			if(copy_fh->inodeno()){ //Put only if non zero
				pathToFhMap.put(placeholder,*copy_fh);
			}//Put child in HashMap
		}
		fh_parent->set_inodeno(fh_child->inodeno());//For next iteration 
		fh_parent->set_generationnum(fh_child->generationnum());//For next iteration 
		fh_parent->set_volumeid(fh_child->volumeid());//For next iteration 
		if(count==1){
			ppath=cpath;}
		else
		{
			ppath=cpath;
		}
		placeholder=ppath;
		index=index+k+1;
		k=0;

		//              std::string  placeholder=ppath+"/";
		//		ppath=placeholder;

	}

	fh_path->set_inodeno(fh_parent->inodeno());//For next iteration 
	fh_path->set_generationnum(fh_parent->generationnum());//For next iteration 
	fh_path->set_volumeid(fh_parent->volumeid());//Fo
	int result=0;
	if(fh_path->inodeno()==0)
	{
		result=-1;
	}
	return (result);

}


static int nfs_mount(char *path)
{
	int ret;
	Request* req = new Request();
	req->set_commandname("mount");
	std::string str(path);
	req->set_name(str);
	req->set_username(username);
	//RPC call
	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	printf("MOUNT rpc complete with status = %d\n", reply.status());
	//Adding "/" just in mount call because "/" is getting added from somewhere
	std::string fpath = std::string(path) + "/" ;
	pathToFhMap.put(fpath, reply.fh());
	printf("The filehandle after mount entries are %d,%d,%d for path = %s\n", reply.fh().volumeid(), reply.fh().inodeno() ,reply.fh().generationnum(), fpath.c_str() ) ;
	ret = reply.status();
	printf("mount ret %d\n", ret);
	return ret;	
}


static void* nfs_init(struct fuse_conn_info *conn)
{
	int result = nfs_mount((char*)mount_path.c_str());
	printf("INIT: mount complete with ret = %d\n", result);
	if(!result)
	{
		printf("Mount Fail\n");
		//	exit(0);
	}
	int* status = &result;
	return (void *)status;
}




static int nfs_mkdir(const char *path, mode_t mode)
{
	std::string fpath;
	fpath = mount_path + std::string(path) ;
	printf("MKDIR fullpath to dir on server side = %s\n", fpath.c_str()) ;
	std::string truncatedPath = fpath;
	int len=truncatedPath.length();
	int i = len-1;
	int pos =len -1;
	char pp[PATH_MAX];
	for(pos=len-1;truncatedPath[pos] != '/';pos--) ;

	int j,k=0;
	for(j=pos+1;fpath[j]!=0;j++)
	{
		pp[k]=fpath[j];
		k++;
	}

	pp[k]=0;
	for(i=len-1;truncatedPath[i] != '/';i--) {
		truncatedPath[i] = 0 ;
	}
	printf("MKDIR trunc dir is %s\n", truncatedPath.c_str()) ;

	std::string root_path = mount_path + "/";
	printf("MKDIR root dir is %s\n", root_path.c_str()) ;
	if(strcmp(truncatedPath.c_str(),root_path.c_str())!=0)
	{
		truncatedPath[i]=0;
	}
	printf("MKDIR parent dir is %s\n", truncatedPath.c_str()) ;
	fileHandle* fh = new fileHandle();
	int ret = pathToFhMap.get(truncatedPath, fh) ;
	if(ret == -1) {
		printf("fh not found, so returned null \n");
		fh->set_volumeid(0) ;
		fh->set_inodeno(0) ;
		fh->set_generationnum(0) ;
	}

	Request* req = new Request();
	req->set_commandname("mkdir");
	req->set_name(pp);
	req->set_username(username);
	printf("MKDIR The filhandle entries from hashmap are %d,%d,%d", fh->volumeid(), fh->inodeno() ,fh->generationnum()    ) ;
	req->set_allocated_fh(fh);

	Response reply = client->sendRequest(*req);
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}


	pathToFhMap.put(fpath, reply.fh());
	printf("The filhandle entries in the reply after mkdir are %d,%d,%d", reply.fh().volumeid(), reply.fh().inodeno() ,reply.fh().generationnum()    ) ;
	ret = reply.status();
	return ret;
}



static int nfs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	printf("Read got called\n");
	fi->flags = 00777;
	fileHandle *fh_parent = new fileHandle();
	std::string fpath;
	fpath= mount_path + std::string(path) ;
	int ret = pathToFhMap.get(fpath, fh_parent);
	Request* req = new Request();
	req->set_commandname("read");
	req->set_username(username);
	req->set_offset(offset);
	req->set_count(static_cast<int>(size));
	if(ret != -1)
		req->set_allocated_fh(fh_parent);
	else
		printf("READ fileHandle not found for path = %s\n", fpath.c_str());
	readDataResponse reply = client->readRequest(*req); //TODO recheck offset logic + recheck returned data
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->readRequest(*req);
		}
	}

	printf("nfs_read size = %d offset = %d", size, offset);
	printf("nfs_read data = %s", reply.data().c_str());
	char* data = (char*)reply.data().c_str();

	int len = strlen(data);
	memcpy(buf, data, len);
	if(reply.status()<0)
	{
		ret=-reply.err();
	}
	else
	{
		ret=reply.status();
	}
	return(ret);
}



static int nfs_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	printf("Write got called\n");
	fi->flags = 00777;
	fileHandle *fh_parent = new fileHandle();
	std::string fpath;
	fpath= mount_path + std::string(path) ;
	int ret = pathToFhMap.get(fpath, fh_parent);

	writeDataRequest* req = new writeDataRequest();
	req->set_commandname("write");
	req->set_username(username);
	req->set_offset(offset);
	req->set_count(static_cast<int>(size));
	if(ret != -1)
		req->set_allocated_fh(fh_parent);
	else
		printf("WRITE fileHandle not found for path = %s\n", fpath.c_str());
	req->set_data(buf);
	Response reply = client->writeRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->writeRequest(*req);
		}
	}

	if(reply.status()<0)
	{
		ret=-reply.err();
	}
	else
	{
		ret = reply.status();
	}
	return(ret);

}

static int nfs_fsync(const char *path, int isdatasync,
		struct fuse_file_info *fi)
{
	printf("commit got called\n");
	//fi->flags = 00777;
	fileHandle *fh_parent = new fileHandle();
	std::string fpath;
	fpath= mount_path + std::string(path) ;
	int ret = pathToFhMap.get(fpath, fh_parent);

	Request* req = new Request();
	req->set_commandname("commit");
	req->set_username(username);
	//req->set_offset(offset);
	//req->set_count(static_cast<int>(size));
	if(ret != -1)
		req->set_allocated_fh(fh_parent);
	else
		printf("COMMIT fileHandle not found for path = %s\n", fpath.c_str());

	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}


	if(reply.status()<0)
	{
		ret=-reply.err();
	}
	else
	{
		ret = reply.status();
	}
	return(ret);


}


static int nfs_rmdir(const char *path)
{
	std::string fpath;
	fpath = mount_path + std::string(path) ;
	printf("RMDIR fullpath to dir on server side = %s\n", fpath.c_str()) ;
	std::string truncatedPath = fpath;
	int len=truncatedPath.length();
	int i = len-1;
	int pos =len -1;
	char pp[PATH_MAX];
	for(pos=len-1;truncatedPath[pos] != '/';pos--) ;
	int j,k=0;
	for(j=pos+1;fpath[j]!=0;j++)
	{
		pp[k]=fpath[j];
		k++;
	}
	pp[k]=0;
	for(i=len-1;truncatedPath[i] != '/';i--) {
		truncatedPath[i] = 0 ;
	}

	std::string root_path = mount_path + "/";
	printf("RMDIR root dir is %s\n", root_path.c_str()) ;
	if(strcmp(truncatedPath.c_str(),root_path.c_str())!=0)
	{
		truncatedPath[i]=0;
		printf("entered inside\n");
	}
	printf("RMDIR parent dir is %s\n", truncatedPath.c_str()) ;
	fileHandle* fh = new fileHandle();
	int ret = pathToFhMap.get(truncatedPath, fh) ;
	if(ret == -1) {
		printf("fh not found, so returned null \n");
		fh->set_volumeid(0) ;
		fh->set_inodeno(0) ;
		fh->set_generationnum(0) ;
	}

	Request* req = new Request();
	req->set_commandname("rmdir");
	req->set_name(pp);
	req->set_username(username);
	printf("RMDIR The filhandle entries from hashmap are %d,%d,%d", fh->volumeid(), fh->inodeno() ,fh->generationnum()    ) ;
	req->set_allocated_fh(fh);

	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	//pathToFhMap.del(fpath, reply.fh()); TODO delete
	printf("rmdir status %d", reply.status() ) ;
	ret = 0;
	if(reply.status()<0)
	{
		ret=-reply.err();
	}
	else
	{
		int test=pathToFhMap.del(fpath); 
		ret = reply.status();
	}
	return(ret);

}

static int nfs_remove(const char *path)
{	// TODO : if full paths FH exist send that directly! Why not , that way can delete that FH on server side
	std::string fpath;
	fpath = mount_path + std::string(path) ;
	printf("REMOVE fullpath to dir on server side = %s\n", fpath.c_str()) ;
	std::string truncatedPath = fpath;
	int len=truncatedPath.length();
	int i = len-1;
	int pos =len -1;
	char pp[PATH_MAX];
	for(pos=len-1;truncatedPath[pos] != '/';pos--) ;
	int j,k=0;
	for(j=pos+1;fpath[j]!=0;j++)
	{
		pp[k]=fpath[j];
		k++;
	}
	pp[k]=0;
	for(i=len-1;truncatedPath[i] != '/';i--) {
		truncatedPath[i] = 0 ;
	}

	std::string root_path = mount_path + "/";
	printf("REMOVE root dir is %s\n", root_path.c_str()) ;
	if(strcmp(truncatedPath.c_str(),root_path.c_str())!=0)
	{
		truncatedPath[i]=0;
	}
	printf("RMDIR parent dir is %s\n", truncatedPath.c_str()) ;
	fileHandle* fh = new fileHandle();
	int ret = pathToFhMap.get(truncatedPath, fh) ;
	if(ret == -1) {
		printf("fh not found, so returned null \n");
		fh->set_volumeid(0) ;
		fh->set_inodeno(0) ;
		fh->set_generationnum(0) ;
	}

	Request* req = new Request();
	req->set_commandname("remove");
	req->set_name(pp);
	req->set_username(username);
	printf("REMOVE The filhandle entries from hashmap are %d,%d,%d", fh->volumeid(), fh->inodeno() ,fh->generationnum()    ) ;
	req->set_allocated_fh(fh);

	Response reply = client->sendRequest(*req);
	//Retry
	if(reply.grpc_error_code()==14)
	{
		while(reply.grpc_error_code()==14) {
			reply = client->sendRequest(*req);
		}
	}

	//pathToFhMap.del(fpath, reply.fh()); TODO delete
	printf("remove status %d", reply.status() ) ;
	ret = 0;
	if(reply.status()<0)
	{
		ret=-reply.err();
	}
	else
	{
		int test=pathToFhMap.del(fpath);
		ret = reply.status();
	}
	return(ret);

}



//int nfs_opendir(const char *path, struct fuse_file_info *fi)
//{
//	//DIR *dp;
//	printf("opendir got called\n");
//	int retstat = 0;
//	std::string fpath;
//	fpath = mount_path + std::string(path) ;
//	// since opendir returns a pointer, takes some custom handling of
//	// return status.
//	/*dp = opendir(fpath);
//	  log_msg("    opendir returned 0x%p\n", dp);
//	  if (dp == NULL)
//	  retstat = log_error("nfs_opendir opendir");
//
//	  fi->fh = (intptr_t) dp;
//
//	  log_fi(fi);*/
//
//	return retstat;
//}






static  struct nfs_operations : fuse_operations {
	nfs_operations() {
		init = nfs_init;
		open=nfs_open;
		mkdir=nfs_mkdir;
		//access=nfs_access;
		statfs=nfs_statfs;
		getattr=nfs_getattr;
		read=nfs_read;
		write=nfs_write;
		fsync=nfs_fsync;
		//opendir=nfs_opendir;		
		create=nfs_create;		
		rmdir=nfs_rmdir;		
		unlink=nfs_remove;
		readdir=nfs_readdir;
		rename=nfs_rename;
	}

} nfs_oper;


int main(int argc, char *argv[])
{
	client = new nfscommunication(grpc::CreateChannel(
				"localhost:50051", grpc::InsecureChannelCredentials()));
	// nfs::fileHandle* dirh;
	// char *name = "grpc";
	// Request req = create_message_lookup(dirh, name);

	// Response reply = client.sendRequest(req);
	// printf("reply.status %d\n", reply.status());
	fdcount=102422;

	mount_path = std::string(argv[1]);
	username = atoi(argv[2]);
	argv[argc-4] = argv[argc-2];
	argv[argc-3] = argv[argc-1];
	argv[argc-2] = NULL;
	argv[argc-1] = NULL;
	argc=argc-2;
	printf("mount point   %s    %s    %d \n",argv[1],argv[2],argc);
	// argv[argc-3] = argv[argc-1];
	// argv[argc-2] = NULL;
	// argv[argc-1] = NULL;:
	return fuse_main(argc, argv, &nfs_oper, NULL);
}

