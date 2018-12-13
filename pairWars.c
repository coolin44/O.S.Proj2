#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

struct card {
  int num;
  struct card * next;
};
struct card * head;
struct card * tail;
struct card * hand[3];
int games = -1;
int turn = -1;
int deck[52];
enum Role{PLAYER1, PLAYER2, PLAYER3};
bool playersDone[3];
bool isDealing;
bool isFinished;
bool isWinner;
pthread_mutex_t m_isDealing;
pthread_cond_t c_isDealing;
pthread_mutex_t m_isPlaying;
pthread_cond_t c_isPlaying;

void initializeDeck(){
        int j = 2;
        int i = 0;
        for(i; i < 52; i++){
                if((i % 4) == 0 && i != 0){
                        j++;
                }
                deck[i] = j;
        }
}

void initialize()
{
    pthread_mutex_init(&m_isPlaying, NULL);
    pthread_cond_init(&c_isPlaying, NULL);
    pthread_mutex_init (&m_isDealing, NULL);
    pthread_cond_init (&c_isDealing, NULL);
    hand[PLAYER1] = NULL;
    hand[PLAYER2] = NULL;
    hand[PLAYER3] = NULL;
    head = NULL;
    tail = NULL;
    playersDone[PLAYER1] = false;
    playersDone[PLAYER2] = false;
    playersDone[PLAYER3] = false;
    isDealing = true;
    isWinner = false;
    isFinished = false;
}

void shuffle()
{
    initializeDeck();
    int cardsLeft = 52;
    while(cardsLeft != 0)
    {
        int randPos = rand() % 52;
        if(deck[randPos] != -1)
        {
            struct card * temp;
            temp = (struct card *)malloc(sizeof(struct card));
            temp->num = deck[randPos];
            deck[randPos] = -1;
            if(head == NULL)
            {
                head = temp;
                tail = head;
                head->next = NULL;
            } 
            else
            {
                tail->next = temp;
                tail = temp;
                tail->next = NULL;
            }
            cardsLeft--;
        }
    }
}

void swap(int *a, int *b)
{
        int temp = *a;
        *a = *b;
        *b = temp;
}

void Deal(){
    hand[PLAYER1] = head;
    head = head->next;
    hand[PLAYER1]->next = NULL;
    hand[PLAYER2] = head;
    head = head->next;
    hand[PLAYER2]->next = NULL;
    hand[PLAYER3] = head;
    head = head->next;
    hand[PLAYER3]->next = NULL;
}

void takeTopCard(int playerid)
{
    struct card * temp = head;
    head = head->next;
    hand[playerid]->next = temp;
    temp->next = NULL;
    FILE * file = fopen("Log.txt", "a");
    fprintf(file, "PLAYER %ld: draws ", playerid);
    fprintf(file, "%d\n", temp->num);
    fclose(file);
    temp = NULL;
}

bool isPair(int playerid)
{
    if(hand[playerid]->num == hand[playerid]->next->num)
        return true;
    else
        return false;
}

void removeCard(int playerid)
{
    int disCard = rand() % 2 + 1;
    if(disCard == 1)
    {
        tail->next = hand[playerid];
        hand[playerid] = hand[playerid]->next;
        tail = tail->next;
        tail->next = NULL;
    }
    else
    {
        struct card * temp = hand[playerid]->next;
        hand[playerid]->next = NULL;
        tail->next = temp;
        tail = temp;
        tail->next = NULL;
        temp = NULL;
    }
    FILE * file = fopen("Log.txt", "a");
    fprintf(file, "PLAYER %ld: discards ", playerid);
    fprintf(file, "%d\n", tail->num);
    fclose(file);
}

void destroyDeck()
{
    struct card * temp = head;
    int num;
    for(num = 0; num < 3; num++)
    {
        if(hand[num] != NULL)
        {
            if(hand[num]->next != NULL)
            {
                tail->next = hand[num];
                tail = hand[num]->next;
            } 
            else
            {
                tail->next = hand[num];
                tail = tail->next;
            } 
            hand[num] = NULL;
        }
    }
    while(head != tail)
    {
      head = head->next;
      free(temp);
      temp = head;
    }
    free(head);
    head = NULL;
    tail = NULL;
    temp = NULL;
    playersDone[PLAYER1] = false;
    playersDone[PLAYER2] = false;
    playersDone[PLAYER3] = false;
}

void writeDeckToLog()
{
    struct card * temp = head;
    FILE * file = fopen("Log.txt", "a");
    fprintf(file, "\nDECK: ");
    while(temp != tail)
    {
        fprintf(file, " %d", temp->num);
        temp = temp->next;
    }
    fprintf(file, " %d\n\n", temp->num);
    temp = NULL;
    fclose(file);
}

void writeDeckToConsole(){
    struct card * temp = head;
    printf("DECK: ");
    while(temp != tail)
    {
        printf(" %d", temp->num);
        temp = temp->next;
    }
    printf(" %d\n", temp->num);
    temp = NULL;
}

void writeWinningHandToConsole(int playerId){
     printf("%d", hand[playerId]->num);
     printf(", %d", hand[playerId]->next->num);
}

void writeHandToConsole(int playerId){
        printf("%d", hand[playerId]->num);
}

void writeHandToLog(int playerId)
{
    FILE * file = fopen("Log.txt", "a");
    fprintf(file, "Player %ld: Hand ", playerId);
    if(hand[playerId]->next == NULL)
    {
        fprintf(file, "%d\n", hand[playerId]->num);
    }
    else
    {
        fprintf(file, "%d", hand[playerId]->num);
        fprintf(file, ", %d\n", hand[playerId]->next->num);
    }
    fclose(file);
}

void *deal()
{
    do
    {
        pthread_mutex_lock(&m_isDealing);
        destroyDeck();
        if(games < 2)
        {
            printf("The Dealer has begun shuffling...\n");
            FILE * file = fopen("Log.txt", "a");
            fprintf(file, "The Dealer has begun shuffling...\n");
            fclose(file);
        }
        shuffle();
        Deal();
        games++;
        turn = games;
        isWinner = false;
        isDealing = false;
        if(games < 3)
        {
            printf("**************************************************");
            printf("\n GAME %ld\n", games);
            printf("**************************************************\n");

            FILE * file = fopen("Log.txt", "a");
            fprintf(file, "**************************************************");
            fprintf(file, "\n GAME %ld\n", games);
            fprintf(file, "**************************************************\n");
            fclose(file);
            writeDeckToLog();
        }
        pthread_cond_signal(&c_isPlaying);
        pthread_mutex_unlock(&m_isDealing);
        if(games < 3)
        {
            pthread_mutex_lock(&m_isDealing);
            pthread_cond_wait(&c_isDealing, &m_isDealing);
            pthread_mutex_unlock(&m_isDealing);
        }
    } while(games < 3);
    pthread_exit(NULL);
} 

void *player(void *playerId)
{
    while(games < 3)
    {
        while(isWinner == false)
        {
            pthread_mutex_lock(&m_isDealing);
            while(turn != playerId)
            {
                pthread_cond_wait(&c_isPlaying, &m_isDealing);
                if(turn != playerId)
                    pthread_cond_signal(&c_isPlaying);
            }
            if(isWinner == false)
            {
                writeHandToLog(playerId);
                printf("PLAYER %ld: is drawing.\n", playerId);
                takeTopCard(playerId);
                if(isPair(playerId) == true)
                {
                    FILE * file = fopen("Log.txt", "a");
                    int id = playerId;
                    switch(id){
                        case 0:
                            writeHandToLog(1);
                            writeHandToLog(2);
                            writeHandToLog(0);
                            printf("PLAYER 0;\n");
                            printf("    HAND ");
                            writeWinningHandToConsole(0);
                            printf("\n    WIN yes\n");
                            printf("PLAYER 1:\n");
                            printf("    HAND ");
                            writeHandToConsole(1);
                            printf("\n    WIN no\n");
                            printf("PLAYER 2:\n");
                            printf("    HAND ");
                            writeHandToConsole(2);
                            printf("\n    WIN no\n");
                            writeDeckToConsole();
                            fprintf(file ,"PLAYER %ld: wins and exits.\n", playerId);
                            printf("PLAYER %ld: wins and exits.\n", playerId);
                            fprintf(file ,"PLAYER 1: exits.\n");
                            printf("PLAYER 1: exits.\n");
                            fprintf(file ,"PLAYER 2: exits.\n");
                            printf("PLAYER 2: exits.\n");
                            fclose(file);
                            break;
                        case 1: 
                            writeHandToLog(0);
                            writeHandToLog(2);
                            writeHandToLog(1);
                            printf("PLAYER 0;\n");
                            printf("    HAND ");
                            writeHandToConsole(0);
                            printf("\n    WIN no\n");
                            printf("PLAYER 1:\n");
                            printf("    HAND ");
                            writeWinningHandToConsole(1);
                            printf("\n    WIN yes\n");
                            printf("PLAYER 2:\n");
                            printf("    HAND ");
                            writeHandToConsole(2);
                            printf("\n    WIN no\n");
                            writeDeckToConsole();
                            fprintf(file ,"PLAYER %ld: wins and exits.\n", playerId);
                            printf("PLAYER %ld: wins and exits.\n", playerId);
                            fprintf(file ,"PLAYER 0: exits.\n");
                            printf("PLAYER 0: exits.\n");
                            fprintf(file ,"PLAYER 2: exits.\n");
                            printf("PLAYER 2: exits.\n");
                            fclose(file);
                            break;
                        case 2:
                            writeHandToLog(0);
                            writeHandToLog(1);
                            writeHandToLog(2);
                            printf("PLAYER 0;\n");
                            printf("    HAND ");
                            writeHandToConsole(0);
                            printf("\n    WIN no\n");
                            printf("PLAYER 1:\n");
                            printf("    HAND ");
                            writeHandToConsole(1);
                            printf("\n    WIN no\n");
                            printf("PLAYER 2:\n");
                            printf("    HAND ");
                            writeWinningHandToConsole(2);
                            printf("\n    WIN yes\n");
                            writeDeckToConsole();
                            fprintf(file ,"PLAYER %ld: wins and exits.\n", playerId);
                            printf("PLAYER %ld: wins and exits.\n", playerId);
                            fprintf(file ,"PLAYER 0: exits.\n");
                            printf("PLAYER 0: exits.\n");
                            fprintf(file ,"PLAYER 1: exits.\n");
                            printf("PLAYER 1: exits.\n");
                            fclose(file);
                            break;
                    }
                    playersDone[(int)playerId] = true;
                    isWinner = true;
                    isDealing = true;
                }
                else
                {
                    removeCard(playerId);
                    writeHandToLog(playerId);
                }
                int temp = turn + 1;
                if(temp == 3)
                    turn = PLAYER1;
                else{
                    turn += 1;
                }
                writeDeckToLog();
                pthread_cond_signal(&c_isPlaying);
                pthread_mutex_unlock(&m_isDealing);
            }
        }
        if(playersDone[(int)playerId] != true)
        {
            switch((int)playerId)
            {
                case PLAYER1:
                    if(playersDone[PLAYER2] == true && playersDone[PLAYER3] == true )
                    {
                        playersDone[PLAYER1] = true;
                        pthread_cond_signal(&c_isDealing);
                        pthread_mutex_unlock(&m_isDealing);
                    }
                    else
                    {
                        playersDone[PLAYER1] = true;
                        turn = PLAYER2;
                        pthread_cond_broadcast(&c_isPlaying);
                        pthread_mutex_unlock(&m_isDealing);
                    }
                    break;
                case PLAYER2:
                    if(playersDone[PLAYER1] == true && playersDone[PLAYER3] == true )
                    {
                        playersDone[PLAYER2] = true;
                        pthread_cond_signal(&c_isDealing);
                        pthread_mutex_unlock(&m_isDealing);
                    }
                    else
                    {
                        playersDone[PLAYER2] = true;
                        turn = PLAYER3;
                        pthread_cond_broadcast(&c_isPlaying);
                        pthread_mutex_unlock(&m_isDealing);
                    }
                    break;
                    case PLAYER3:
                        if(playersDone[PLAYER2] == true && playersDone[PLAYER1] == true )
                        {
                            playersDone[PLAYER3] = true;
                            pthread_cond_signal(&c_isDealing);
                            pthread_mutex_unlock(&m_isDealing);
                        }
                        else
                        {
                            playersDone[PLAYER3] = true;
                            turn = PLAYER1;
                            pthread_cond_broadcast(&c_isPlaying);
                            pthread_mutex_unlock(&m_isDealing);
                        }
                        break;
                    default:
                        break;
                }
        }
        while(isDealing == true)
        {
        } 
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    initialize();
    pthread_t players[3];
    pthread_t dealer;
    if(argv[1] > 1){
        srand(atoi(argv[1]));
    } 
    else{
        printf("The argument must be greater than 1, exiting...");
        exit(44);
    }   
    FILE * file = fopen("Log.txt", "w");
    fprintf(file, "***STARTING GAME***\n");
    int err;
    err = pthread_create(&dealer, NULL, deal, NULL);
    if (err) {
        exit(44);
    }
    int t;
    for(t=0; t< 3; t++)
    {
        printf("CREATING THREAD: %ld\n", (t));
        err = pthread_create(&players[t], NULL, player, (void *)t);
        if (err) {
            exit(44);
        }
    }
    err = pthread_join(dealer, NULL);
    if (err) {
         exit(44);
    }
    for(t=0; t<3; t++) {
        err = pthread_join(players[t], NULL);
        if (err) {
            exit(44);
        }
    } 
    printf("ALL 3 GAMES ARE FINISHED\n");
    printf(file, "ALL 3 GAMES ARE FINISHED\n");
    fclose(file);
    return 0;
}
