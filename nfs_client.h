#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "nfs.grpc.pb.h"


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using nfs::Request;
using nfs::Response;
using nfs::readDataResponse;
using nfs::writeDataRequest;
using nfs::communication;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;


class nfscommunication {
	public:
//		nfscommunication();
		nfscommunication(std::shared_ptr<Channel> channel)
			: stub_(communication::NewStub(channel)) {}
//		 nfscommunication(const nfscommunication &ob);
           


		// Assambles the client's payload, sends it and presents the response back
		// from the server.
		Response sendRequest(const Request& req);

	 	readDataResponse readRequest(const Request& req);

		Response writeRequest(const writeDataRequest& req);




	private:
		std::unique_ptr<communication::Stub> stub_;
};


//Request create_message_lookup(nfs::fileHandle* dirh,char *name);
