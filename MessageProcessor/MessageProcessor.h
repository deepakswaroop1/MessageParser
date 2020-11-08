#ifndef MESSAGEPROCESSOR_H
#define MESSAGEPROCESSOR_H

using namespace std;

#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#include <pthread.h>
#include <fstream>
#include <map>
#include <chrono>
#include <ctime>
#include <unistd.h>

struct MessageStructure
{
    double price;
    unsigned long seq_number;
    unsigned long size;
    char msgType;
    char side;
    string ticker;
    MessageStructure()
    {
        price = 0.0;
        seq_number = 0;
        size = 0;
        msgType = 0;
        side = 0;
        ticker = "";
    }
};
class MessageProcessor
{
    public:
        MessageProcessor();
        void init(string,int,unsigned long);
        virtual ~MessageProcessor();
        void startReading();
        void writeDataToFiles();

    protected:

    private:
        string directoryPath;
        unsigned int maxMessagesAllowedPerTime;
        unsigned int messageCount;
        unsigned long perTimeIntervalInMilliSec;
        vector<string> filenames;
        bool initDone;
        bool read_directory();
        void* readMessageThread();
        static void* readMessageThreadHelper(void* p)
        {
            return ((MessageProcessor*)p)->readMessageThread();
        }
        map<unsigned long,MessageStructure> messageMap;
        void GenerateOutFile();
        void readFile(ifstream&, std::chrono::time_point<std::chrono::system_clock>& startTime);
        bool canProcessMoreMessages(std::chrono::time_point<std::chrono::system_clock>& startTime);
        std::chrono::time_point<std::chrono::system_clock> startTime;
};

#endif // MESSAGEPROCESSOR_H
