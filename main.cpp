#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

const int READ_END = 0;
const int WRITE_END = 1;

void childSieve(int readFD);
bool checkCodes(string code, string check);
void sieve(int readFD);

int main(int argc, char* argv[]) 
{    
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
    int sieveFd[2];

    if(pipe(sieveFd) < 0)
    {
        exit(1);
    }
    close(sieveFd[READ_END]);
    // read one character at a time until the '\0' that ends each code
    string filterCode;
    string checkCode;
    char c;
    int index = 0;
    bool hasChild = false;
    bool endOfWord = false;

    while(read(readFD, &c, 1) > 0){
        if(c == '\0'){
            index++;
            endOfWord = true;
            
        }

        if(index == 0){
            filterCode += c;
        } else{
            checkCode += c;
        }

        if(endOfWord && index != 1){
            cout << filterCode << " " << checkCode << endl;
            if(checkCodes(filterCode, checkCode)){
                //Distinct
                write(sieveFd[WRITE_END], &checkCode, checkCode.length() + 1);
                if(hasChild == false){
                    childSieve(sieveFd[READ_END]);
                    hasChild == true;
                }

            }
            checkCode = "";
        }

        endOfWord = false;
        
    }
}
void childSieve(int readFD){
    int sieveFd[2];

    if(pipe(sieveFd) < 0)
    {
        exit(1);
    }
    pid_t childId = fork();

    if(childId == 0)
    {
        int sieveFd[2];

        if(pipe(sieveFd) < 0)
        {
            exit(1);
        }
        close(sieveFd[READ_END]);
        // read one character at a time until the '\0' that ends each code
        string filterCode;
        string checkCode;
        char c;
        int index = 0;
        bool hasChild = false;
        bool endOfWord = false;

        while(read(readFD, &c, 1) > 0){
            if(c == '\0'){
                index++;
                endOfWord = true;
            
            }

            if(index == 0){
                filterCode += c;
            } else{
                checkCode += c;
            }

            if(endOfWord && index != 1){
                cout << filterCode << " " << checkCode << endl;
                if(checkCodes(filterCode, checkCode)){
                    //Distinct
                    write(sieveFd[WRITE_END], &checkCode, checkCode.length() + 1);
                    if(hasChild == false){
                        childSieve(sieveFd[READ_END]);
                        hasChild == true;
                    }

                }
            checkCode = "";
            }

            endOfWord = false;
        
        }
    }
}