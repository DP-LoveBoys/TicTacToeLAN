#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <gtk/gtk.h>


#define MESSAGE_SIZE 512 //size of message
char board[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

void initializeBoard(){
        printf(" _ _ _ _ _ _ \n");
        printf("| %c | %c | %c |\n", board[1], board[2], board[3]);
        printf(" -----------\n");
        printf("| %c | %c | %c |\n", board[4], board[5], board[6]);
        printf(" -----------\n");
        printf("| %c | %c | %c |\n", board[7], board[8], board[9]);
        printf(" -----------\n");
    
}

int current_move;
char prevMove = '-';

char PlayerChar;
char OpponentChar;

int server_socket;
int client_socket;

int validInput(int position)
{
    if(position < 0 || position > 9)
        return 0;
    return 1;
}

int validPosition(int position)
{
    if(board[position] == '0' || board[position] == 'X')
        return 0;
    return 1;
}

int checkLines()
{
    if(board[1] == board[2])
        if(board[2] == board[3])
            return 1;
    if(
        ((board[1] == board[2]) && (board[2] == board[3])) || ((board[4] == board[5]) && (board[5] == board[6])) || ((board[7] == board[8]) && (board[8] == board[9]))
    ) 
        return 1;
    
    return 0;
}

int checkCols()
{
    if(
        ((board[1] == board[4]) && (board[4] == board[7])) || 
        ((board[2] == board[5]) && (board[5] == board[8])) || 
        ((board[3] == board[6]) && (board[6] == board[9]))
    ) 
        return 1;
    
    return 0;
}

int checkDiags()
{
    if(
        ((board[1] == board[5]) && (board[5] == board[9])) || 
        ((board[3] == board[5]) && (board[5] == board[7]))
    ) 
        return 1;
    
    return 0;
}
int checkWin()
{
    //printf("Checking\n");
    
    if(checkLines() || checkCols() || checkDiags())
        return 1;
    return 0;
}
int checkDraw()
{
    int draw;
    for(int i = 1; i < 10; i++)
    {
        if(board[i] == 'X' || board[i] == '0')
            draw = 1;
        else
        {
            draw = 0;
            break;
        }
    }
    
    return draw;
}

void read_position()
{
    int position;
    
    printf("It's time to make a move, %c\nYour move: ", PlayerChar);
    while(1)
    {
        scanf("%d", &position);
        if(!validInput(position))
        {
            printf("Invalid move, try again\n");
            continue;
        }
        if(!validPosition(position))
        {
            printf("That spot is already taken, try again\n");
            continue;
        }
        break;
    }
    
    current_move = position;
}

char playGame(char player, char prevMove)
{
    char move;
    
    if(prevMove == '-')
    {
        if(player == 'x' || player == 'X')
            move = 'X';
        else
            move = '0';
    }
    else
    {
        if(prevMove == 'X')
            move = '0';
        else
            move = 'X';
    }
    
    board[current_move] = move;
    initializeBoard();
    
    return move;
}

int your_turn(char PlayerChar, int other_socket)
{
    char move[MESSAGE_SIZE];
    
    read_position();
    prevMove = playGame(PlayerChar, prevMove);
    
    sprintf(move, "%d", current_move);
    if(send(other_socket, &move, MESSAGE_SIZE, 0) == -1)
    {
        printf("Can't send move\n");
        exit(-11);
    }
    
    if (checkWin() == 1)
    {
        printf("You won!\n");
        return 1;
    }
    if (checkDraw() == 1)
    {
        printf("Draw!\n");
        return 1;
    }
    
    return 0;
}

int opponent_turn(char OpponentChar, int other_socket)
{
    char move[MESSAGE_SIZE];
    
    if (other_socket != server_socket)
    if(listen(server_socket, 5) == -1)
    {
        printf("Failed listening for a message\n");
        exit(-12);
    }
    printf("Wainting for the opponent's move...\n");
    if(recv(other_socket, &move, MESSAGE_SIZE, 0) == -1)
    {
        printf("Can't receive message\n");
        exit(-13);
    }
    
    printf("Opponent's move: %c\n", move[0]);
    current_move = atoi(move);
    prevMove = playGame(OpponentChar, prevMove);
    
    if (checkWin() == 1)
    {
        printf("Your opponent won!\n");
        return 1;
    }
    if (checkDraw() == 1)
    {
        printf("Draw!\n");
        return 1;
    }   
    
    return 0;
}

int Game(char PlayerChar, char OpponentChar, int current_socket, int other_socket)
{
    if (PlayerChar == 'X')
    {
        if(your_turn(PlayerChar, other_socket) == 1 || opponent_turn(OpponentChar, other_socket) == 1)
            return 1;
    }
    else
    {
        if(opponent_turn(OpponentChar, other_socket) == 1 || your_turn(PlayerChar, other_socket) == 1)
            return 1;
    }
    return 0;
}

void on_main_window_destroy(){
    exit(0);
}

void on_btn_exit_game_clicked(){
    exit(0);
}


int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("\tCorrect usage: program_name s/c portnumber\n");
        exit(-10);
    }
    if(argv[1][0] !='c' && argv[1][0] != 's')
    {
        printf("Use s/c (server/client)\n");
    }
    
    //interface elements
    GObject *window;
    GObject *exit_button;
    GtkBuilder *builder;
    GError *err=NULL;
    
    //interface initialization
    gtk_init(&argc,&argv);
    
    builder=gtk_builder_new();
    if(gtk_builder_add_from_file(builder,"glade/window_main.glade",&err)==0){
        g_printerr("Error loading glade file: %s\n",err->message);
        g_clear_error(&err);
        exit(-1);
    }
    
    window=gtk_builder_get_object(builder,"window_main");
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit), NULL);
    
    exit_button=gtk_builder_get_object(builder,"btn_exit_game");
    g_signal_connect(exit_button,"clicked",G_CALLBACK(on_btn_exit_game_clicked), NULL);
    
    
    
    
    //network connection
    int PORT = atoi(argv[2]); // PORT from command line
    char selection = argv[1][0];
    
    if(selection == 's') // server (host) part
    {
        srand(time(0));
        int random_number = rand() % 2; //Random number to determine which player goes first 
        
        if (random_number == 0)
        {
            PlayerChar = 'X';
            OpponentChar = '0';
        }
        else
        {
            PlayerChar = '0';
            OpponentChar = 'X';
        }
        
        if((server_socket=socket(AF_INET,SOCK_STREAM, 0)) == -1)
        {
            printf("Socket error\n");
            exit(-2);
        }

        struct sockaddr_in server_adr;
        server_adr.sin_family = AF_INET;
        server_adr.sin_port = htons(PORT);
        server_adr.sin_addr.s_addr = INADDR_ANY;

        if(bind(server_socket, (struct sockaddr *)&server_adr, sizeof(server_adr)) == -1)
        {
            printf("Error at bind, server side\n");
            exit(-2);
        }
        
        printf("Waiting for the opponent to connect...\n");
        if(listen(server_socket, 5) == -1)
        {
            printf("Error while listening\n");
            exit(-3);
        }

        client_socket = accept(server_socket, NULL, NULL); //NULL, we don't use peer to peer
        if(client_socket == -1)
        {
            printf("Error while connecting\n");
            exit(-4);
        }
        printf("The game is now started!\n");
        
        g_object_unref(builder);
        gtk_widget_show((GtkWidget*)window);
        gtk_main();
        
        char message[MESSAGE_SIZE];
        sprintf(message, "%c", OpponentChar);
        
        printf("You are playing as %c\n", PlayerChar);
        if(send(client_socket, message, MESSAGE_SIZE, 0) == -1)
        {
            printf("Error when sending message to client\n");
            exit(-5);
        }
        
        initializeBoard();
        while(1)
        {
            if (Game(PlayerChar, OpponentChar, server_socket, client_socket) == 1)
                break;
        }
        
        //close connection
        close(server_socket); //if error, it closes anyway
    }
    else //client side
    {
        int conn;
        
        if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            printf("Socket error, client side\n");
            exit(-6);
        }

        struct sockaddr_in server_adr;
        server_adr.sin_family=AF_INET;
        server_adr.sin_port= htons(PORT);
        server_adr.sin_addr.s_addr = INADDR_ANY;


        if( (conn = connect(server_socket, (struct sockaddr *)&server_adr, sizeof(server_adr))) == -1)
        {
            printf("Error connecting to server\n");
            exit(-7);
        }
        
        client_socket = socket(PF_INET, SOCK_STREAM, 0);
        if(client_socket == -1)
        {
            printf("Error building client socket\n");
            exit(-8);
        }
        printf("The game is now started!\n");
        
        g_object_unref(builder);
        gtk_widget_show((GtkWidget*)window);
        gtk_main();
        
        char message3[MESSAGE_SIZE];
        
        if(recv(server_socket, &message3, MESSAGE_SIZE, 0) == -1)
        {
            printf("Error fetching data from server\n");
            exit(-9);
        }
        printf("You are playing as %s\n", message3);
        
        if (message3[0] == '0')
        {
            PlayerChar = '0';
            OpponentChar = 'X';
        }
        else
        {
            PlayerChar = 'X';
            OpponentChar = '0';
        }
        
        initializeBoard();
        
        while(1)
        {
            if (Game(PlayerChar, OpponentChar, client_socket, server_socket) == 1)
                break;
        }
    }
    return 0;
}

















