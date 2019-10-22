/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "xrp_ledger.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using io::xpring::GetAccountInfoRequest;
using io::xpring::AccountInfo;
using io::xpring::GetFeeRequest;
using io::xpring::Fee;
using io::xpring::XRPLedgerAPI;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(XRPLedgerAPI::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string GetAccountInfo(const std::string& address) {
    // Data we are sending to the server.
    GetAccountInfoRequest request;
    request.set_address(address);
    //request.set_name(user);

    // Container for the data we expect from the server.
    AccountInfo reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetAccountInfo(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.DebugString();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string GetFee()
  {
      GetFeeRequest request;

      Fee reply;

      ClientContext context;
      Status status = stub_->GetFee(&context, request, &reply);

      if(status.ok())
      {
        return reply.DebugString();
      } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
      }

  
  }

 private:
  std::unique_ptr<XRPLedgerAPI::Stub> stub_;
};

int main(int argc, char** argv) {
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).


    std::string method;
    if(argc < 2)
    {
        std::cout << "not enough args. usage: [method] [account] [num threads]"
            << std::endl;
        return -1;
    }

    method = argv[1];

    std::string account;
    int thread_args = 2;
    if(method == "account_info")
    {
        if(argc < 2)
        {
            std::cout << "not enough args. pass account" << std::endl;
            return -1;
        }
        account = argv[2];
        thread_args = 3;
    }

    int num_threads = 1;
    int num_loops = 1;
    if(argc > thread_args)
    {
        std::cout << "parsing thread args" << std::endl;
    
        num_threads = std::stoi(argv[thread_args]);
        if(argc > thread_args+1)
        {
            num_loops = std::stoi(argv[thread_args+1]);
        }
    }

    std::vector<std::thread> threads;

    for(size_t i = 0; i < num_threads; ++i)
    {
    threads.emplace_back([method,account,num_loops]()
        {
            for(size_t i = 0; i < num_loops; ++i)
            {
                auto start = std::chrono::system_clock::now();
                GreeterClient greeter(grpc::CreateChannel(
                            "localhost:50051", grpc::InsecureChannelCredentials()));
                if(method == "account_info")
                {
                std::string user(account);
                std::string reply = greeter.GetAccountInfo(user);
                std::cout << "Greeter received: " << reply << std::endl;
                }
                else if(method == "fee")
                {
                std::string reply = greeter.GetFee();
                std::cout << "Greeter received: " << reply << std::endl;

                if(reply.find("disconnect") != std::string::npos)
                {
                    break;
                }
                }


                // Some computation here
                auto end = std::chrono::system_clock::now();
                //
                std::chrono::duration<double> elapsed_seconds = end-start;
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);

                std::cout 
                << "elapsed time: " << elapsed_seconds.count() << "s\n";
            }
        });
    }
    for(auto& t : threads)
    {
        t.join();
    }
    return 0;
}
