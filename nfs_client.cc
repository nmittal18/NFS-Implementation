#include "nfs_client.h"
using namespace nfs;

// Assambles the client's payload, sends it and presents the response back
// from the server.
Response nfscommunication::sendRequest(const Request& req) {

	// Data we are sending to the server.
	Request request = req;

	// Container for the data we expect from the server.
	Response* reply = new Response();;

	// Context for the client. It could be used to convey extra information to
	// the server and/or tweak certain RPC behaviors.
	ClientContext context;

	// The actual RPC.
	Status status = stub_->sendRequest(&context, request, reply);

	if (status.ok()) {
		reply->set_grpc_error_code(status.error_code());
		std::cout << "sendRequest rpc succeeded." << std::endl;
		return *reply;
	}
	else {
		reply->set_grpc_error_code(status.error_code());
		std::cout << "sendRequest rpc failed with error code =" << status.error_code() <<std::endl;
		return *reply;
	} 
}

readDataResponse nfscommunication::readRequest(const Request& req) {
	// Data we are sending to the server.
	Request request = req;

	// Container for the data we expect from the server.
	readDataResponse* reply = new readDataResponse();

	// Context for the client. It could be used to convey extra information to
	// the server and/or tweak certain RPC behaviors.
	ClientContext context;

	std::unique_ptr<ClientReader<readDataResponse> > reader(
			stub_->readRequest(&context, request));
	while (reader->Read(reply)) {}
	Status status = reader->Finish();

	if (status.ok()) {
		std::cout << "readrequest rpc succeeded." << std::endl;
		reply->set_grpc_error_code(status.error_code());
		return *reply;
	}
	else {
		std::cout << "readRequest rpc failed with error code =" << status.error_code() <<std::endl;
		reply->set_grpc_error_code(status.error_code());
		return *reply;
	} 
}

Response nfscommunication::writeRequest(const writeDataRequest& req) {
	// Data we are sending to the server.
	writeDataRequest request= req;

	// Container for the data we expect from the server.
	Response* reply = new Response();

	// Context for the client. It could be used to convey extra information to
	// the server and/or tweak certain RPC behaviors.
	ClientContext context;

	// The actual RPC.
	//Status status = stub_->sayClient(&context, request, &reply);

	std::unique_ptr<ClientWriter<writeDataRequest> > writer(
			stub_->writeRequest(&context, reply));
	if (!writer->Write(request)) {
		// Broken stream.
		printf("write failed \n");
	}
	writer->WritesDone();
	Status status = writer->Finish();

	if (status.ok()) {
		std::cout << "writeRequest rpc succeeded"<< std::endl;
		reply->set_grpc_error_code(status.error_code());
		return *reply;
	}
	else {
		std::cout << "writeRequest rpc failed with status = %s" << status.error_code() <<std::endl;
		reply->set_grpc_error_code(status.error_code());
		return *reply;
	}

}




//	private:
//		std::unique_ptr<communication::Stub> stub_;
//};



//int main()
//{
// printf ("Vishakha") ;
//}
//	nfscommunication client(grpc::CreateChannel(
//				"localhost:50051", grpc::InsecureChannelCredentials()));
//	nfs::fileHandle* dirh;
//	char *name = "dummyFile";
//	Request req = create_message_remove(dirh, name);
//
//	Response reply = client.sendRequest(req);
//	printf("reply.status %d\n", reply.status());
//}
