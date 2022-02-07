/***************************************************************************
* File: program1.c
* Author: Zesheng Xu
* Procedures:
 *main - carry out the program, creates child and parent process, child process delivers query to user
 * and parent returns result based on user input, they communicate through 2 message queues
 *
 *
 *
***************************************************************************/



#include <iostream>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<sys/wait.h>

#include <sys/resource.h>
#include <sys/utsname.h>

#define MSG_SIZE   100 // pre-defining the max size of the messages

using namespace  std;
int main() {

/***************************************************************************
* int main( )
* Author: Zesheng Xu
* Date: 5 Feb 2022
* Description: Creates 2 messages queues for IPC and creates both child and parent process.
 *      Child process querys user and deliver input to parent queue through message queue. While parent
 *      queue returns result through message queue to child queue.
*
* Parameters:
*
**************************************************************************/

    cout<<("This is the start of the program")<<endl;
    char  buf[MSG_SIZE]; // create the buffer variable
    struct mq_attr attr; // initialize the attribute for message queues

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 101;
    attr.mq_curmsgs = 0;

    mqd_t mq_c,mq_p ;
    mode_t  mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH;

    unsigned int prio_c, prio_p = 0;

// clear the queue before we use them incase there are queues from previous testing
    mq_unlink("/child_queue");
    mq_unlink("/parent_queue");


    mq_c = mq_open("/child_queue",O_CREAT | O_RDWR | O_NONBLOCK,mode,&attr); // open the message queue
    mq_p = mq_open("/parent_queue",O_CREAT | O_RDWR | O_NONBLOCK ,mode, &attr); // open the message queue



    if(mq_p == -1 || mq_c == -1) // if unable to create the queues
    {
        printf("failed%s", strerror(errno));
        cout<<("Failed to open a queue")<<endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        //printf("Queue creation successful\n");
    }






    bool quit = false; //bool to determine when to exit the program





        //cout<<("Forking")<<endl;
        pid_t fid; // id to record result of fork
        fid = fork(); // forking - creating a child process

        if (fid == -1) // if fork unsuccessful
        {
            // print the error and exit the program with error signal
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (fid == 0) // if this is a child process
        {

            //cout<<("This is child")<<endl;

            while(true) {

                if (mq_receive(mq_p, buf, 101, NULL) == -1) { // if there are 0 current messages in the queue
                    // prompt user to enter what they want to do
                    cout << ("Enter your choices: \n") << endl;
                    cout << ("1. Display Current Domain. \n") << endl;
                    cout << ("2. Display Current Host. \n") << endl;
                    cout << ("3. Display Current User. \n") << endl;
                    cout << ("4. Exit.") << endl;

                    char choice[64]; //input choice from user
                    cin.clear();
                    cin >> choice;
                    cout << "You chose: " << choice << endl;
                    // try to send the choice to parent
                    // if fail, exit and print error statement

                    if (mq_send(mq_c, choice, sizeof(choice), prio_c) == -1) {
                        cout << "send error" << endl;
                        perror("mq send error");
                        exit(EXIT_FAILURE);
                    } else {
                        prio_c +=1;
                        cout << "Sent to queue" << endl;
                    }

                    sleep(1); // sleep for 1 seoncds to wait response from parent
                } else // the queue is not empty and we should display the message
                {
                    int exit_call ; // check if parent delivered the exit signal to child
                    sscanf(buf, "%d", &exit_call); // turn the content from char[] to int

                    if(exit_call == 4) { // if the parent did ask the child to exit, we exit
                        cout << ("Exiting child") << endl;

                        exit(0);
                    }
                    else // else we print out the content of the message 
                            {
                                cout << ("Message received from server: ") << buf << "\n" << endl;

                            }

                }

            }

        } else // then it is the parent process
        {
            //cout << ("This is the Server!") << endl;


            while(not quit) // looping so it is always ready to be updated until quit
            {

            // if we detected a request from child
            // we send requested information

                if (mq_receive(mq_c, buf, 101, NULL) != -1) {

                int choice;
                sscanf(buf, "%d", &choice);
                cout<<"Server received: " <<(choice)<<endl; // debugging to see what the parent recevived
                switch (choice) {
                    case 1: // send domain info
                        // if we cant send, print error and exit
                        getdomainname(buf, sizeof (buf));
                        if (mq_send(mq_p,  buf, sizeof(buf), prio_p) == -1) {
                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }

                        break;
                    case 2:// send host info
                        gethostname(buf, sizeof (buf));
                        // if we cant send, print error and exit
                        if (mq_send(mq_p, buf, sizeof(buf), prio_p) == -1) {
                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        break;
                    case 3: // send user info
                        struct utsname unameData; // creating a uname struct
                        uname(&unameData); // return uname value
                        // if we cant send, print error and exit
                        if (mq_send(mq_p, unameData.sysname, sizeof (unameData.sysname), prio_p) == -1) {

                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        if (mq_send(mq_p, unameData.nodename, sizeof (unameData.nodename), prio_p) == -1) {

                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        if (mq_send(mq_p, unameData.release, sizeof (unameData.release), prio_p) == -1) {

                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        if (mq_send(mq_p, unameData.version, sizeof (unameData.version), prio_p) == -1) {

                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        if (mq_send(mq_p, unameData.machine, sizeof (unameData.machine), prio_p) == -1) {

                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        if (mq_send(mq_p, unameData.domainname, sizeof (unameData.domainname), prio_p) == -1) {

                            perror("mq send error");
                            exit(EXIT_FAILURE);
                        }
                        break;
                    case 4:// exit
                        mq_send(mq_p, "4", sizeof ("4"), prio_p); // send quit signal to child
                        quit = true;

                        break;

                }
                //cout<<"Message sent to child"<<endl;
                prio_p+=1;

            }

        }
        }


// remove the message queue
    mq_close(mq_p);
    mq_close(mq_c);
    mq_unlink("/child_queue");
    mq_unlink("/parent_queue");

    cout<<("Exiting parent")<<endl;

    exit(0);


}

