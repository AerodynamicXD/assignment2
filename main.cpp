#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

const int READ_END = 0;
const int WRITE_END = 1;

bool checkCodes(string code, string check);
void sieve(int readFD);

int main(int argc, char* argv[]) 
{   
    cout << "Finding maximally distinct codes..." << endl;
    int ParentToSieveFd[2];
    if(pipe(ParentToSieveFd) < 0)
    {
        exit(1);
    }
    pid_t childId = fork();

    if(childId == 0)
    {
        close(ParentToSieveFd[WRITE_END]);
        sieve(ParentToSieveFd[READ_END]);
        exit(0);
    }
    else
    {
        close(ParentToSieveFd[READ_END]);

        for(int i = 1; i < argc; i++){
            write(ParentToSieveFd[WRITE_END], argv[i], strlen(argv[i]) + 1);
        }
        //cout << "done" << endl;
        close(ParentToSieveFd[WRITE_END]);
        int status;

        waitpid(childId, &status, 0);
    }
}

bool checkCodes(string code, string check)
{
    bool Diff = true;

    if(code.size() != check.size()){
        return true;
    }

    int difference = 0;
    for(int i = 0; i < code.length(); i++ ){
        if(code[i] != check[i]){
            difference += 1;
        }
    }
    if(difference <= 1){
        Diff = false;
    }
    return Diff;
}

void sieve(int readFD){
    string filterCode;
    string checkCode;
    char c;
    int sieveFd[2];

    if(pipe(sieveFd) < 0){
        exit(1);
    }
    // first word received is this process's filter
    while(read(readFD, &c, 1) > 0 && c != '\0'){
        filterCode += c;
    }

    bool hasChild = false;
    bool endOfWord = false;
    pid_t childPid = -1;

    while(read(readFD, &c, 1) > 0){
        if(c == '\0'){
            endOfWord = true;
        }
        if(c != '\0'){
            checkCode += c;
        }
        if(endOfWord){
            if(checkCodes(filterCode, checkCode)){
                if(hasChild == false){
                    childPid = fork();
                    if(childPid == 0){
                        close(sieveFd[WRITE_END]);
                        close(readFD);
                        sieve(sieveFd[READ_END]);
                        exit(0);
                    }
                    close(sieveFd[READ_END]);
                    hasChild = true;
                }
                const char *p = checkCode.c_str();
                write(sieveFd[WRITE_END], p, checkCode.length() + 1);
            }
            checkCode = "";
            endOfWord = false;
        }
    }
    close(sieveFd[WRITE_END]);
    cout << "Process: "<< getpid() << " my code is: " << filterCode << endl;
}