#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;
//set constants for read and write ends for clarity
const int READ_END = 0;
const int WRITE_END = 1;

bool checkCodes(string code, string check);
void sieve(int readFD);

int main(int argc, char* argv[]) 
{   
    cout << "Finding maximally distinct codes..." << endl;
    //making our pipe
    int ParentToSieveFd[2];
    if(pipe(ParentToSieveFd) < 0)
    {
        exit(1);
    }
    //we fork here for our first child
    pid_t childId = fork();

    if(childId == 0)
    {
        //child closes write end of pipe that parent will write codes into
        close(ParentToSieveFd[WRITE_END]);
        //run our sieve program 
        sieve(ParentToSieveFd[READ_END]);
        //exit after running the sieve
        exit(0);
    }
    else
    {
        //closes the read end since we wont be reading from this pipe
        close(ParentToSieveFd[READ_END]);
        //writing the codes to the pipe 
        for(int i = 1; i < argc; i++){
            write(ParentToSieveFd[WRITE_END], argv[i], strlen(argv[i]) + 1);
        }
        //close the write end after its all written
        close(ParentToSieveFd[WRITE_END]);
        int status;

        waitpid(childId, &status, 0);
    }
}

bool checkCodes(string code, string check)
{
    bool Diff = true;
    //we check the size of the codes to each other, if different immediately return true
    if(code.size() != check.size()){
        return true;
    }

    int difference = 0;
    for(int i = 0; i < code.size(); i++ ){
        //for every different character we increment 
        if(code[i] != check[i]){
            difference++;
        }
    }
    //if codes are different by more that 1 character it pass otherwise it fails
    if(difference <= 1){
        Diff = false;
    }
    return Diff;
}

void sieve(int readFD){
    //sets strings for our codes
    string filterCode;
    string checkCode;
    //sets the character that we'll store our read character in the pipe to
    char c;
    int sieveFd[2];
    //make our pipe
    if(pipe(sieveFd) < 0){
        exit(1);
    }
    //while loop that reads the first word in the pipe and sets it to our filtercode string
    while(read(readFD, &c, 1) > 0 && c != '\0'){
        filterCode += c;
    }
    //our bool value for if we have made a child already
    bool hasChild = false;
    //bool value to define if we're at the end of the word
    bool endOfWord = false;
    //we just set our childPid here and to -1 so we dont worry about any weird returns
    pid_t childPid = -1;
    //while loop reads words till we get to null value at end of word
    while(read(readFD, &c, 1) > 0){
        if(c == '\0'){
            //when we hit the null at the end of the word we set endOfWord to true
            endOfWord = true;
        }
        if(c != '\0'){
            //we set the read character to checkcode till our word is built
            checkCode += c;
        }
        if(endOfWord){
            //when we get to the end of the word we run the check and will fork from here
            if(checkCodes(filterCode, checkCode)){
                if(hasChild == false){
                    //if we havent made a child yet and we hit a word that passes the filter we fork
                    childPid = fork();
                    if(childPid == 0){
                        //the new child closes the write end as it will not be writing to the current active pipe
                        close(sieveFd[WRITE_END]);
                        //also closes the read to the main parent as it will not be reading from there
                        close(readFD);
                        //opens new sieve program with the current sieves read 
                        sieve(sieveFd[READ_END]);
                        //exits after the sieve finishes
                        exit(0);
                    }
                    //parent process closes the read end to the new child and sets has child to true
                    close(sieveFd[READ_END]);
                    hasChild = true;
                }
                //we make p point to checkCode and write this to our child pipe
                const char *p = checkCode.c_str();
                write(sieveFd[WRITE_END], p, checkCode.length() + 1);
            }
            //reset checkcode to nothing and endofword to false so we can repeat for next code
            checkCode = "";
            endOfWord = false;
        }
    }
    //after we've read, filtered and wrote our codes we close the write and print the active processid and the filter code in it
    close(sieveFd[WRITE_END]);
    cout << "Process: "<< getpid() << " my code is: " << filterCode << endl;
}