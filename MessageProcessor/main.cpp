#include <MessageProcessor.h>
#define messagesLimitPerTime 100
#define perTimeInMilliSec 10000

int main()
{
    string path;
    cout << "Enter the directory path for example \"C:\\temp\" ::" << endl;
    std::getline(std::cin,path);

    //cout<<"Entered Path is :"<<path;
    MessageProcessor m_parser;
    m_parser.init(path,messagesLimitPerTime,perTimeInMilliSec);
    //m_parser.writeDataToFiles();
    m_parser.startReading();

    getchar();
    return 0;
}
