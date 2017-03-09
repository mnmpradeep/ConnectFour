/******Program to implement Connect 4 game using a producer-consumer problem***************
Output - A Referee (Producer) checks the game between 2 players (Consumers) and the players contend to play in a random 
         manner
********************************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#define MAXINPUTS 42 //Maximum capacity of board (6*7 = 42)
#define MAXPLAYERS 2 //Maximum number of players in the game

//Variables to count the turns and check game completion
int count_turns[MAXPLAYERS],finish;

time_t t;


//Game board and locks associated with the game for players and referee
struct common
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int board[6][7], entries, ready;
}
boardaccess = { 
                    PTHREAD_MUTEX_INITIALIZER,
                    PTHREAD_COND_INITIALIZER,
                    {   0000000, 0000000, 0000000, 0000000, 0000000, 0000000     },
                    0,
                    0
              }; 


//Lock to ensure synchronization synchronization on referee's moves. 
//Usage is more significant in multiple referres (producers)
struct check_winner
{
    pthread_mutex_t mutexcw;
    int signals;
}
cw = {
            PTHREAD_MUTEX_INITIALIZER,
            0
      };


int main()
{   
    int i;
    
    //Creating threads for players and referee
    pthread_t players[MAXPLAYERS], referee;
    
    //Function to implement for referee
    void* printandcheck();
    
    //Function containing the method of play by the players
    void* play (void* p_no);
    srand( (unsigned) time(&t) );
  
    //Creating players and referee
    pthread_create(&referee, NULL, &printandcheck, NULL);

    for(i = 0; i < MAXPLAYERS; i++)
    {
        count_turns[i] = 0;
        pthread_create (&players[i], NULL, &play, (void *) i); 
    }
    
    sleep(1);
  
    //Sleeping untill all the threads have finished executing
    while (finish == 0)
    {
        sleep(1);
    }
  return 1;
}


void* play (void* p_no)
{
   int i, j, pos, insert = 0;
  
   //continue playing unless game is over
   while(1)
   {
        //Contending to attain a lock on game board
        if(pthread_mutex_trylock(&boardaccess.mutex)==0) 
        {
 
           //waits untill referee has completed checking  
            while(boardaccess.ready == 0)
             pthread_cond_wait(&boardaccess.cond, &boardaccess.mutex);
             insert = 0;
     
             while( insert == 0 && finish == 0)
             {
                   /**randomly generating postion of insertion onto the board***/
                   pos = (int)(rand() % 7);
             
                  //Inserting into the board
                  for(i = 5; i >= 0; i--)
                   {
                        if(boardaccess.board[i][pos] == 0)
                        {
                             boardaccess.board[i][pos] = ((int) p_no) + 1;
                                insert = 1;
                             break;
                        }
                   }
                        
             }       
            
            if(insert == 1)
            {
 
                printf("\n****Printing Board****\n");
                for(i = 0; i < 6; i++)
                {
                    printf("***************************************************\n");
                    for (j = 0; j < 7; j++)
                    {
                        if(boardaccess.board [i][j] == 0 )
                            printf("**     ");
                        else if(boardaccess.board [i][j] == 1 )
                        {
                            //Printing "Y - Yellow" for Player 1 
                            printf("**  Y  ");
                        }
                        else if(boardaccess.board [i][j] == 2 )
                        {
                            //Printing "R - Red" for Player 2
                            printf("**  R  ");
                        }
                        else
                            printf("**  %d  ",boardaccess.board[i][j]);
                            //Incase there are more players, used for printing their turns
                    }
                 
                    printf("**\n");
                }
                printf("***************************************************\n");
                printf("\nPlayer 1  - 'Y' (yellow)");
                printf("\nPlayer 2  - 'R' (red)\n"); 
                printf("\nPlayer %d has played!\n",((int) p_no) + 1);
         
            }
        
        //Releasing lock and letting the players contend for lock once again
        boardaccess.ready = 0;
        pthread_mutex_unlock(&boardaccess.mutex);
        pthread_cond_broadcast(&boardaccess.cond);
        sleep(1);
       
          if(insert == 0)
            break;
       }
    }       
     pthread_exit( NULL );     
}


void* printandcheck()
{
    int player = 0,cons_plays, count_plays = 0, i, j, k, winner = 0;

    while(1)
    {
        /****INITIALIZING THE GAME***/
        if(count_plays == 0)
        {
            printf("\nGAME BEGINS NOW!!");
            //sleep(1);
        }
        
        //Acknowledging game after completion of every round
        else
            printf("\n****Round No %d completed****", count_plays);
        count_plays++;
        
        
        //Attaining locks for checking and accessing the board
        winner = 0;
        if(pthread_mutex_lock(&cw.mutexcw) == 0) 
        {
            cw.signals++;
            if(pthread_mutex_lock(&boardaccess.mutex)==0)
            {
                while(boardaccess.ready == 1)
                     pthread_cond_wait(&boardaccess.cond, &boardaccess.mutex);
            }
           
           
            sleep(2);   
             
            /**Checking if any player has won the game*/ 
                if(winner == 0)
                {
                   //Checking if any player has inserted 4 balls vertically
                    for(j = 0; ( (j < 7) && (winner == 0) ); j++)
                    {
                         cons_plays = 0;
			 player = 0; 
			 for(i = 5; i >= 0; i--)
                        {
                           
                            if((player!=0) && (player == boardaccess.board[i][j]))
                                cons_plays++;
                            else
                            {
                                cons_plays = 1;    
                                
                                //optimizing
                                if( i < 3)
                                 break;
                            }
                            player = boardaccess.board[i][j];
                       
                        
                            if(cons_plays == 4)
                            {
                              winner = player;
                              finish = 1;
                              printf("\nPlayer %d has won!!!",player,i,j);
                              printf("\nGame Over\n");

         
                              boardaccess.ready = 1;
                              pthread_mutex_unlock(&cw.mutexcw);            
                              pthread_mutex_unlock(&boardaccess.mutex);
                              finish = 1;
                              return NULL;
                          
                            }
                        }
                    }   
                }    
            
            //Checking if any player has inserted 4 balls along the right inclined diagnol
            if(winner == 0)
            {    
                //first half of right inclined diagnol
                for(k = 3, cons_plays = 0, player = 0; ( (k < 7) && (winner == 0) ); k++)
                    for(j = k, cons_plays = 0, player = 0 ; ((j >= 0) && (winner == 0) ); j--)
                    {
                        for(i = 0; ( (i < 6) && (j >= 0) ); i++, j--)
                        {
                            if( (player != 0) && (player == boardaccess.board[i][j]) )
                                cons_plays++;
                            else
                            {
                                cons_plays = 1;    
                                
                 
                            }    
                            player = boardaccess.board[i][j];
                                
                            
                            if(cons_plays == 4)
                            {
                                winner = player;
                                finish = 1;
                                printf("\nPlayer %d has won!!!",player);
                                printf("\nGame Over\n");
                                break;
                            }
                        }
                    }  
                   
                   //second half of right inclined diagnol
                   for(k = 1, cons_plays = 0, player = 0; ( (k < 4) && (winner == 0) ); k++)
                    for(j = 6, cons_plays = 0, player = 0 ; ((j > 0) && (winner == 0) );  )
                    {
                        for(i = 1; ( (i < 6) && (j >= 0) ); i++, j--)
                        {
                            if( (player != 0) && (player == boardaccess.board[i][j]) )
                                cons_plays++;
                            else
                            {
                                cons_plays = 1;    
                                
                 
                            }    
                            player = boardaccess.board[i][j];
                                
                            
                            if(cons_plays == 4)
                            {
                                winner = player;
                               finish = 1;
                               printf("\nPlayer %d has won!!!",player);
                               printf("\nGame Over\n");
                     
                                break;
                            }
                        }
                    }
             }     
             
                         
            //Checking if any player has inserted 4 balls consectively along the left inclined diagnol
            if(winner == 0)
            {    
                //first half of left inclined diagnol
                for(k = 3, cons_plays = 0, player = 0; ( (k < 7) && (winner == 0) ); k++)
                    for(j = k, cons_plays = 0, player = 0 ; ((j >= 0) && (winner == 0) ); j--)
                    {
                        for(i = 5; ( (i >= 0) && (j >= 0) ); i--, j--)
                        {
                            if( (player != 0) && (player == boardaccess.board[i][j]) )
                                cons_plays++;
                            else
                            {
                                cons_plays = 1;    
                                
                         
                            }    
                            player = boardaccess.board[i][j];
                                
                            
                            if(cons_plays == 4)
                            {
                                winner = player;
                                finish = 1;
                                printf("\nPlayer %d has won!!!",player);
                                printf("\nGame Over\n");
                                break;
                            }
                        }
                    }  
                    
                    //second half of left inclined diagnol
                    for(k = 2, cons_plays = 0, player = 0; ( (k < 5) && (winner == 0) ); k++)
                    for(j = 6, cons_plays = 0, player = 0 ; ((j >= 0) && (winner == 0) ); )
                    {
                        for(i = 4; ( (i >= 0) && (j >= 0) ); i--, j--)
                        {
                            if( (player != 0) && (player == boardaccess.board[i][j]) )
                                cons_plays++;
                            else
                            {
                                cons_plays = 1;    
                                
                         
                            }    
                            player = boardaccess.board[i][j];
                      
                            
                            if(cons_plays == 4)
                            {
                                winner = player;
                                finish = 1;
                                printf("\nPlayer %d has won!!!",player);
                                printf("\nGame Over\n");
                   
                                break;
                            }
                        }
                    }  
           }    
           
           //If some player has won, releasing locks and exiting the threads                             
           if(winner != 0)
           {
             
               boardaccess.ready = 1;
               pthread_mutex_unlock(&cw.mutexcw);            
               pthread_mutex_unlock(&boardaccess.mutex);
               finish = 1;
               return NULL;
           }
           
           
           //If the boards if full and ends in a draw
           else if(count_plays >= MAXINPUTS)
           {
               printf("\n Board is full. Its a draw!!\n");
               boardaccess.ready = 1;
               pthread_mutex_unlock(&cw.mutexcw);            
               pthread_mutex_unlock(&boardaccess.mutex);
               finish = 1;
               return NULL;
           }
           
           //If there is more space on the board to play
           else 
           {
               printf("\nNo player has won, let the play resume!!\n");
               sleep(2);
           }
         boardaccess.ready = 1;
         pthread_mutex_unlock(&cw.mutexcw);            
         pthread_mutex_unlock(&boardaccess.mutex); 
         pthread_cond_broadcast(&boardaccess.cond);
         sleep(1);

        }
    }
 return NULL;
}	


