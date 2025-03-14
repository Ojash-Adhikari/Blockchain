#ifndef __C_CHAIN_INCLUDED__
#define __C_CHAIN_INCLUDED__
#include "CBlock.h"
#include "storage/EStorageType.h"
#include "storage/IStorage.h"
#include "net/CServer.h"
#include "net/CClient.h"
#include "CLog.h"
#include <vector>

namespace blockchain
{

    class CChain
    {
    private:
        std::vector<CBlock*> mChain; // List of blocks
        CBlock* mCurrentBlock;      // Pointer to the current block &mChain.last()
        int mDifficulty;            // Difficulty
        storage::IStorage* mStorage; //
        std::string mHostName;
        uint32_t mNetPort;
        net::CServer* mServer;
        std::vector<net::CClient*> mClients;
        bool mRunning;
        bool mStopped;
        bool mReady;
        CLog mLog;
    public:
        CChain(const std::string& hostname, uint32_t hostPort = 7698, int difficulty = 0, storage::E_STORAGE_TYPE storageType = storage::EST_NONE);
        CChain(const std::string& hostname, uint32_t hostPort = 7698, bool newChain = false, const std::string& connectToNode = std::string(), int difficulty = 0, storage::E_STORAGE_TYPE storageType = storage::EST_NONE, uint32_t connectPort = 7698);     //
        ~CChain();                                                                          //
        void appendToCurrentBlock(uint8_t* data, uint32_t size); 
        void nextBlock(bool save = true, bool distribute = true);       // Continue to next block
        void distributeBlock(CBlock* block);   // Distribute written block to other nodes
        CBlock* getCurrentBlock(); // Gets a pointer to the current block
        CBlock* getGenesisBlock();
        void load();                                                                          // load the chain
        std::vector<CBlock*>* getChainPtr();
        size_t getBlockCount();                                                           // return the number of blocks
        bool isValid();                                                                 // if the chain is   valid
        void stop();    
        bool isRunning();
        std::string getHostName();
        uint32_t getNetPort();
        net::CClient* connectNewClient(const std::string& hostname, uint32_t port, bool child = false);
        std::vector<net::CClient*>* getClientsPtr();
        bool isReady();
        void insertBlock(CBlock* block);
        void pushBlock(CBlock* block);
        void clear();
        bool hasHash(uint8_t* hash, uint32_t depth);
    };

}

#endif                      