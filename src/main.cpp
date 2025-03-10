#include "blockchain/CChain.h"
#include "blockchain/storage/CStorageLocal.h"
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <signal.h>
#include <map>
#include <string>

using namespace std;
using namespace blockchain;

CChain *gChain;

void interruptCallback(int sig)
{
    cout << "\n";
    gChain->stop();
}

bool tobool(std::string str)
{
    for (int n = 0; n < str.size(); n++)
        str[n] = std::tolower(str[n]);

    if (str == "true" || str == "t" || str == "1")
        return true;
    return false;
}

void printChain(CChain* chain) {
    CBlock *cur = chain->getCurrentBlock();
    do
    {
        time_t ts = cur->getCreatedTS();
        string tstr(ctime(&ts));
        tstr.resize(tstr.size() - 1);
        if(cur == chain->getCurrentBlock())
            cout << "CURRENT\t" << cur->getHashStr() << "\tTimeStamp " << tstr << "\tData Size " << cur->getDataSize() << "\n";
        else
            cout << "Block\t" << cur->getHashStr() << "\tTimeStamp " << tstr << "\tData Size " << cur->getDataSize() << "\n";
    } while (cur = cur->getPrevBlock());
}

void storeInputInBlock(CChain& chain, const string& input) {
    // Convert the input string to a byte array
    uint8_t* data = new uint8_t[input.size()];
    copy(input.begin(), input.end(), data);

    // Append the byte array to the current block
    chain.appendToCurrentBlock(data, input.size());
    delete[] data;

    // Mine the next block
    chain.nextBlock();

    cout << "Stored input in new block. Current block count: " << chain.getBlockCount() << "\n";
}

string readDataFromBlock(CBlock* block) {
    uint8_t* data = block->getData();
    size_t dataSize = block->getDataSize();
    return string(data, data + dataSize);
}

CBlock* getBlockByHash(CChain& chain, const string& hashStr) {
    CBlock* block = chain.getCurrentBlock();
    while (block != nullptr) {
        if (block->getHashStr() == hashStr) {
            return block;
        }
        block = block->getPrevBlock();
    }
    return nullptr;
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    if (argc == 0)
    {
        cout << "Error: no binary parameter passed by system.\n";
        return 1;
    }
    string binName(argv[0]);
    if (argc == 1)
    {
        cout << "Usage:\n"
             << binName + " -hYOURHOST -cCONNECTTO -nFALSE\n\n-h\tHOSTNAME\tYour host entry point.\n-c\tHOSTNAME\tConnect to node entrypoint hostname.\n-n\ttrue | false\tIs this a new chain or not.\n\n";
        return 1;
    }

    map<string, string> params;

    for (int n = 0; n < argc; n++)
    {
        string param(argv[n]);
        if (param.size() > 2 && param[0] == '-')
        {
            string varName(param.substr(1, 1));
            params[varName] = param.substr(2);
        }
    }

    if (params.count("h") == 0)
    {
        cout << "You must specify host entrypoint for your node using -h:\nExample: " + binName + " -h127.0.0.1\n\n";
        return 1;
    }

    if (params.count("n") == 0)
    {
        if (params.count("c") == 0)
            params["n"] = "true";
        else
            params["n"] = "false";
    }
    bool isNewChain = tobool(params["n"]);

    if (!isNewChain && params.count("c") == 0)
    {
        cout << "If this is an existing chain. You must specify which node to connect to using -c:\nExample: " + binName + " -cchain.solusek.com\n\n";
        return 1;
    }

    uint32_t hostPort = 7698, connectPort = 7698;
    std::string host(params["h"]), connectTo(params["c"]);
    size_t pos = params["h"].find(':');
    if (pos != std::string::npos)
    {
        host = params["h"].substr(0, pos);
        hostPort = (uint32_t)std::stoi(params["h"].substr(pos + 1));
    }
    pos = params["c"].find(':');
    if (pos != std::string::npos)
    {
        connectTo = params["c"].substr(0, pos);
        connectPort = (uint32_t)std::stoi(params["c"].substr(pos + 1));
    }

    storage::E_STORAGE_TYPE storageType(storage::EST_LOCAL);

    if (params.count("s") != 0)
    {
        if (params["s"] == "none")
            storageType = storage::EST_NONE;
        else
            storage::CStorageLocal::setDefaultBasePath(params["s"]);
    }

    cout << "Start.\n";

    CChain chain(host, hostPort, isNewChain, connectTo, 1, storageType, connectPort);
    gChain = &chain;

    cout << "Chain initialized!\n";
    cout << "Current block count: " << chain.getBlockCount() << "\n";

    if (chain.isValid())
        cout << "Chain is valid!\n";
    else
    {
        cout << "INVALID CHAIN\n";
        return 1;
    }

    // Interrupt Signal
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = interruptCallback;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGQUIT, &sigIntHandler, NULL);

    CBlock* printedBlock = chain.getCurrentBlock();

    while (chain.isRunning()) {
        cout << "Enter 'enter' to store a sentence, 'read' to read from a block, or 'exit' to quit: ";
        string command;
        getline(cin, command);

        if (command == "enter") {
            cout << "Enter a sentence to store in the blockchain: ";
            string input;
            getline(cin, input);
            storeInputInBlock(chain, input);
            cout << "Stored input: " << input << "\n";
            printChain(&chain);

        } else if (command == "read") {
            cout << "Enter the block hash to read: ";
            string blockHash;
            getline(cin, blockHash);

            CBlock* block = getBlockByHash(chain, blockHash);
            if (block != nullptr) {
                string data = readDataFromBlock(block);
                cout << "Data in block " << blockHash << ": " << data << "\n";
            } else {
                cout << "Block not found: " << blockHash << "\n";
            }

        } else if (command == "exit") {
            break;
        }

        usleep(5000);
        if (printedBlock != chain.getCurrentBlock()) {
            printChain(&chain);
            printedBlock = chain.getCurrentBlock();
        }
    }

    cout << "\nExit.\n";

    return 0;
}
