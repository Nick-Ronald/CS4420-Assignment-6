#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <sys/wait.h>

/**
* @name - get_voters
* @brief - This function gets the number of voters via user input
*  
* @return - The number of voters
*/
int get_voters()
{
    int voters;

    printf("Enter the number of voters: ");
    scanf("%d", &voters);

    return voters;
}

/**
* @name - get_rounds
* @brief - This function gets the number of rounds via user input
*
* @return - The number of rounds
*/
int get_rounds()
{
    int rounds;

    printf("Enter the number of rounds: ");
    scanf("%d", &rounds);

    return rounds;
}

/**
* @name - vote
* @brief - This function generates a random number and uses modulus 2 to make 0 or 1, correlating to a true YES
*          vote for a false NO vote
*
*  
* @return - A boolean true or false
*/
bool vote()
{
    bool vote = rand() % 2;
    return vote;
}

int main()
{
    int rounds, voters, voter_id = -1; //Parent process gets voter_id = -1

    //Get the number of voters and rounds via user input
    voters = get_voters();
    rounds = get_rounds();
    
    int pipes[voters][2]; //Prepares all pipes for each voter

    for(int i = 0; i < voters; i++) //Creates all pipes
    {
        if(pipe(pipes[i]) == -1)
        {
            perror("Error creating pipe");
        }
    }

    pid_t pid = 1; //Sets a default pid for the parent process

    for(int i = 0; i < voters; i++) //Creates all child processes
    {
        if(pid != 0) //If this isn't a child
        {
            pid = fork(); //Create a child

            if (pid == -1) //Error handling
            {
                perror("Error forking process");
                close(pipes[i][0]);
                close(pipes[i][1]);
                return 1;
            }

            if(pid == 0) //If this is the child that was just created
            {
                srand(time(NULL)^getpid()); //Seed the random output per child
                voter_id = i; //Set the voter id to the loop iteration (0-<voters - 1>)
                close(pipes[i][0]); //Close the read end of this child's pipe (children write into pipes)

                for(int j = 0; j < voters; j++) //Closes all of the other pipes on each child that don't correlate to them
                {
                    if(j != i)
                    {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                }
            }
        }
    }

    for(int i = 0; i < rounds; i++) //For each round of voting
    {
        if(voter_id > -1) //Child/Voter processes, voters vote and write their vote through the pipe
        {
            bool choice = vote(); 
            write(pipes[voter_id][1], &choice, sizeof(choice));
        }
        if(voter_id == -1) //Judge Process
        {
            int YES = 0, NO = 0; //Set the YES and NO counts to 0 per round
            bool vote;
            printf("------------Round %d------------\n", i+1);
            for(int j = 0; j < voters; j++) //For each child, read in their vote, and count it on YES or NO
            {
                read(pipes[j][0], &vote, sizeof(vote));
                char *vote_text = vote ? "YES" : "NO"; //Uses the vote's boolean value as an indicator of YES or NO
                if(vote) {YES++;} else {NO++;} //Counts votes
                printf("Round %d: Voter %d votes %s\n", i+1, j, vote_text); // Prints the child's vote
            }
            if(YES > NO) //Prints the relevant voting result
            {
                printf("\nRound %d: The voting result is a YES\n", i+1);
            }
            else if (NO > YES)
            {
                printf("\nRound %d: The voting result is a NO\n", i+1);
            }
            else
            {
                printf("\nRound %d: The voting result is a TIE\n", i+1);
            }
            printf("-------------------------------\n\n");
        }
    }

}