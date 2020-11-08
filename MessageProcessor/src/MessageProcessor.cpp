#include "MessageProcessor.h"
using namespace std;
MessageProcessor::MessageProcessor()
{
    //ctor
    initDone = false;
    messageCount = 0;
}

MessageProcessor::~MessageProcessor()
{
    //dtor
}

//Function to initialize the MessageParser
void MessageProcessor::init(string path, int maxMessages, unsigned long perTimeInMillSec)
{
    //initialize reading thread
    directoryPath = path;
    maxMessagesAllowedPerTime = maxMessages;
    perTimeIntervalInMilliSec = perTimeInMillSec;
    initDone = true;
}

//Function to start the reading thread
void MessageProcessor::startReading()
{
    if(!initDone)
    {
        cout<<"\nMessageProcessor not initialized. Exiting\n";
        return;
    }
    bool result = read_directory();
    if(result == true)
    {
        pthread_t readthread;
        pthread_create(&readthread, nullptr, MessageProcessor::readMessageThreadHelper, this);
        pthread_join(readthread, nullptr);
    }

}

//To search for filenames in specified directory
bool MessageProcessor::read_directory()
{
    DIR* dirp = opendir(directoryPath.c_str());
    if(dirp == nullptr)
    {
        cout<<"\nDirectory does not exist!. Exiting Program";
        return false;
    }
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        if(strcmp(dp->d_name,"outFile.txt")!=0
           && strcmp(dp->d_name,".")!=0
           && strcmp(dp->d_name,"..")!=0)
        filenames.push_back(dp->d_name);
    }
    closedir(dirp);
    return true;
}

//Thread which will continuously read messages from directory
void* MessageProcessor::readMessageThread()
{
    cout<<"\nDirectory Found. Start Reading ";
    while(1)
    {
        if(filenames.size() != 0)
        {
            startTime = std::chrono::system_clock::now();
            for(auto itr: filenames)
            {
                string tempName = directoryPath + "\\" + itr;
                cout<<"\nFilename is: "<<tempName;
                ifstream newFile(tempName);
                if(!newFile) {
                    cout << "file not opened";
                    continue;
                }
                else
                    cout<<" File opened successfully";

                //Read content from file
                readFile(newFile,startTime);

                if (remove(tempName.c_str()) != 0) {
                    cout << "\nFile deletion failed" << endl;
                }
                else
                    cout<<"\nFile Deleted ";

            }
            GenerateOutFile();
            filenames.clear();
        }
        else
        {

            char value;
            cout<<"\nDirectory is empty, do you want to continue press N to Exit:"<<endl;
            cin.getline(&value,2);

            const char* b = &value;

            //cout<<"\n value is: "<<b<<" compare result:"<<stricmp(b,"n");
            if(stricmp(b,"n")==0)
            {
                //cout<<"case compared";
                return 0;
            }
            else
                cout<<"\nContinue reading";
        }

        read_directory();

    }

    return 0;
}

//Function to read messages from file.
void MessageProcessor::readFile(ifstream& newFile, std::chrono::time_point<std::chrono::system_clock>& startTime)
{
    newFile.seekg(0,ios::beg);

    while(!newFile.eof())
    {
        if(canProcessMoreMessages(startTime))
        {

            //cout<<"\n\n\tRECORD:";
            MessageStructure m1;
            newFile>>m1.msgType;
            newFile>>m1.price;
            newFile>>m1.seq_number;

            if(m1.seq_number == 0)
                continue;
            newFile>>m1.side;
            newFile>>m1.size;
            newFile>>m1.ticker;
            //getline(newFile,m1.ticker,'\r');

            //cout<<'\n'<<m1.msgType<<" "<<m1.price<<" "<<m1.seq_number<<" "<<m1.side<<" "<<m1.size<<" "<<m1.ticker;
            if(messageMap.find(m1.seq_number) == messageMap.end())
            {
                 messageMap.insert(std::make_pair(m1.seq_number,m1));
            }
            messageCount++;
        }

    }

    newFile.close();
}

//Function to generate output file without duplicates and in order based on sequence numbers
void MessageProcessor::GenerateOutFile()
{
     string outfilename = directoryPath + "\\" + "outFile.txt";
    ofstream outfile(outfilename,ios::app);

    if(!outfile)
    {
        cout<<"\n Unable to create new outfile:";
    }

    for(auto mapitr: messageMap)
    {
        MessageStructure message1  = mapitr.second;
        outfile<<message1.msgType<<"\n";
        outfile<<message1.price<<"\n";
        outfile<<message1.seq_number<<"\n";
        outfile<<message1.side<<"\n";
        outfile<<message1.size<<"\n";
        outfile<<message1.ticker<<"\n";
    }
    cout<<"\n\nOutFile generated successfully for above files:";
    outfile.close();
    messageMap.clear();

}

//Function to check current rate limit and reset time and message count accordingly
bool MessageProcessor::canProcessMoreMessages(std::chrono::time_point<std::chrono::system_clock>& startTime)
{
    auto starttime_t = std::chrono::system_clock::to_time_t(startTime);
    auto currentTime = std::chrono::system_clock::now();
    auto currentTime_t = std::chrono::system_clock::to_time_t(currentTime);

    //cout<<"\nStart time is: "<<std::ctime(&starttime_t)<<"Current time: "<<std::ctime(&currentTime_t);

    typedef std::chrono::duration<int,std::milli> milliSecDuration ;
    milliSecDuration timeDuration = std::chrono::duration_cast<milliSecDuration>(currentTime-startTime);

    unsigned long timediff = timeDuration.count();
    //cout<<"Time difference is:"<<timediff;

    if(timediff<= perTimeIntervalInMilliSec && messageCount <= maxMessagesAllowedPerTime)
    {
        return true;
    }
    else if(timediff >= perTimeIntervalInMilliSec)
    {

        cout<<"\n\nStart time is: "<<std::ctime(&starttime_t)<<"Current time: "<<std::ctime(&currentTime_t);
        cout<<"\tRESENTING TIME AND MESSAGE COUNT";

         messageCount = 0; //Reset message count
         startTime = std::chrono::system_clock::now();
        return true;
    }
    else       // //means time is there but message count has exceeded
    {

        cout<<"\n\nCurrent message count: "<<messageCount<<" exceeds maxMessagesLimit: "<<maxMessagesAllowedPerTime;

        cout<<"\nWaiting for next time slot to begin after: "<<perTimeIntervalInMilliSec-timediff<<" milliseconds";
        usleep((perTimeIntervalInMilliSec-timediff)*1000);

        return false;
    }
}

//Function to generate input file
void MessageProcessor::writeDataToFiles()
{
    string outfilename = directoryPath + "\\" + "abc.txt";
            ofstream newFile(outfilename,ios::app);
                if(!newFile) {
                    cout << "file not opened";
                    //continue;
                }
                else
                    cout<<"File opened successfully";


                MessageStructure message1;

                message1.ticker = "deepak swasd";
                message1.seq_number = 99;
                message1.msgType = 'A';
                message1.price = 24.22;
                message1.size = 123;
                message1.side = 'B';

                newFile<<message1.msgType;
                newFile<<message1.price;
                newFile<<message1.seq_number;
                newFile<<message1.side;
                newFile<<message1.size;
                newFile<<message1.ticker;
                newFile.close();

}
