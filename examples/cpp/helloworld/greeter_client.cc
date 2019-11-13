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
#include "rpc/v1/xrp_ledger.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using rpc::v1::GetAccountInfoRequest;
using rpc::v1::GetAccountInfoResponse;
using rpc::v1::GetFeeRequest;
using rpc::v1::GetFeeResponse;
using rpc::v1::XRPLedgerAPIService;
using rpc::v1::SubmitTransactionRequest;
using rpc::v1::SubmitTransactionResponse;
using rpc::v1::TxRequest;
using rpc::v1::TxResponse;

std::string actualBlobToTextBlob(std::string const& blob)
{
    std::string text_blob;
    std::cout << "blob size is " << blob.size() << std::endl;
    for(size_t i = 0; i < blob.size(); ++i)
    {
        int byte = blob[i];
        int low = byte & 0x0F;
        int hi = (byte & 0xF0) >> 4;

        char c1;
        if(hi >= 10)
        {
            c1 = 'A' + hi - 10;
        }
        else
        {
            c1 = '0' + hi;
        }
        text_blob.push_back(c1);
        char c2;
        if(low >= 10)
        {
            c2 = 'A' + low - 10;
        }
        else
        {
            c2 = '0' + low;
        }                     

        text_blob.push_back(c2);

    }
    return text_blob;

}


class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(XRPLedgerAPIService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string GetAccountInfo(const std::string& address) {
    // Data we are sending to the server.
    GetAccountInfoRequest request;
    request.set_address(address);
    //request.set_name(user);

    // Container for the data we expect from the server.
    GetAccountInfoResponse reply;

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

      GetFeeResponse reply;

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

  std::string Submit(std::string const& blob)
  {
      SubmitTransactionRequest request;

      SubmitTransactionResponse reply;

      request.set_signed_transaction(blob);

      ClientContext context;

      Status status = stub_->SubmitTransaction(&context,request,&reply);
      std::cout << "sent via stub" << std::endl;

      if(status.ok())
      {

                    std::string const& hash = reply.hash();

                    std::string text_hash;
                    for(size_t i = 0; i < hash.size(); ++i)
                    {
                        int byte = hash[i];
                        int low = byte & 0x0F;
                        int hi = (byte & 0xF0) >> 4;
                        
                        char c1;
                        if(hi >= 10)
                        {
                            c1 = 'A' + hi - 10;
                        }
                        else
                        {
                            c1 = '0' + hi;
                        }
                        text_hash.push_back(c1);
                        char c2;
                        if(low >= 10)
                        {
                            c2 = 'A' + low - 10;
                        }
                        else
                        {
                            c2 = '0' + low;
                        }                     

                        text_hash.push_back(c2);

                    }
                    std::cout << "text hash = " << text_hash << std::endl;

        return reply.DebugString();
      } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
      }
  
  }

  std::string Tx(std::string const& hash)
  {
      TxRequest request;

      TxResponse reply;

      request.set_hash(hash);

      ClientContext context;

      Status status = stub_->Tx(&context,request,&reply);
       if(status.ok())
      {
          std::cout << reply.DebugString() << std::endl;
          std::cout << "account is " << actualBlobToTextBlob(reply.tx().account()) << std::endl;
          std::cout << "dest is " << actualBlobToTextBlob(reply.tx().payment().destination()) << std::endl;


        return reply.DebugString();
      } else {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
      } 
  }

 private:
  std::unique_ptr<XRPLedgerAPIService::Stub> stub_;
};

std::string textBlobToActualBlob(std::string const& blob)
{
    std::string actual_blob;
    for(size_t i = 0; i < blob.size(); ++i)
    {
        unsigned int c;
        if(blob[i] >= 'A')
        {
            c = blob[i] - 'A' + 10;
        }
        else
        {
            c = blob[i] - '0';
        }
        c = c << 4;
        ++i;
        if(blob[i] >= 'A')
        {
            c += blob[i] - 'A' + 10;
        }
        else
        {
            c += blob[i] - '0';
        }
        actual_blob.push_back(c);
    }
    return actual_blob;

}

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

    std::string blob;
    if(method == "submit")
    {
        if(argc < 2)
        {
            std::cout << "not enough args. pass account" << std::endl;
            return -1;
        }
        blob = argv[2];
        thread_args = 3;
    }


    bool loop_until_found = false;
    bool queue = false;
    bool signer_list = false;
    if(method == "account_info")
        {
            std::cout << "argc is " << argc << std::endl;
            if(argc > 3)
            {
                std::string next_arg = argv[3];
                if(next_arg == "loop")
                {
                    std::cout << "parsing loop" << std::endl;
                    loop_until_found = true;
                    thread_args = 4;
                }
                int arg_num = 3;
                while(arg_num < argc && arg_num < 5)
                {
                    next_arg = argv[arg_num];
                    arg_num++;
                if(next_arg == "queue")
                {
                    queue = true;
                }
                if(next_arg == "signer_list")
                {
                    signer_list = true;
                }
                }
                thread_args = arg_num;
            }
        }

    std::string hash;
    if(method == "tx")
    {
        hash = argv[2];
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
    std::vector<double> times;
    times.resize(num_threads);

    for(size_t i = 0; i < num_threads; ++i)
    {
    threads.emplace_back([method,account,blob,num_loops,&times,i,loop_until_found,hash]()
        {
        double total_time = 0;
            for(size_t i = 0; i < num_loops; ++i)
            {
                auto start = std::chrono::system_clock::now();
                GreeterClient greeter(grpc::CreateChannel(
                            "localhost:50051", grpc::InsecureChannelCredentials()));
                if(method == "account_info")
                {

                bool not_found = true;
                while(not_found)
                {
                
                std::string user(account);
                std::string reply = greeter.GetAccountInfo(user);
                std::cout << "Greeter received: " << reply << std::endl;
                if(reply == "RPC failed")
                {
                    if(!loop_until_found)
                        break;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                else
                    not_found = false;
                }
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
                else if(method=="submit")
                {
                    std::string actual_blob;
                    for(size_t i = 0; i < blob.size(); ++i)
                    {
                        unsigned int c;
                        if(blob[i] >= 'A')
                        {
                            c = blob[i] - 'A' + 10;
                        }
                        else
                        {
                            c = blob[i] - '0';
                        }
                        c = c << 4;
                        ++i;
                        if(blob[i] >= 'A')
                        {
                            c += blob[i] - 'A' + 10;
                        }
                        else
                        {
                            c += blob[i] - '0';
                        }
                        actual_blob.push_back(c);

                    } 
                    std::cout << "made blob, sending" << std::endl;

                    std::string reply = greeter.Submit(actual_blob);
                    std::cout << "Greeter received : " << reply << std::endl;
                }
                else if(method == "tx")
                {
                    std::string hash_actual = textBlobToActualBlob(hash);
                    std::string reply = greeter.Tx(hash_actual);
                    std::cout << "Greeter received : " << reply
                        << std::endl;

                }

                // Some computation here
                auto end = std::chrono::system_clock::now();
                //
                std::chrono::duration<double> elapsed_seconds = end-start;
                std::time_t end_time = std::chrono::system_clock::to_time_t(end);

                std::cout 
                << "elapsed time: " << elapsed_seconds.count() << "s\n";
                total_time += elapsed_seconds.count();
            }

            times[i] = total_time;
        });
    }
    for(auto& t : threads)
    {
        t.join();
    }

    double total_total_time = 0;
    for(int i = 0; i < times.size(); ++i)
    {
        std::cout << "total time = " << times[i] << std::endl;
        std::cout << "avg time = " << (times[i] / num_loops) << std::endl;


        total_total_time += times[i];
    }

    std::cout << "global avg = " << (total_total_time / (num_loops *num_threads)) << std::endl;
    return 0;
}
