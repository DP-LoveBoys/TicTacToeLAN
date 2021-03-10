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

int winning[3] = {0, 0, 0};

typedef struct{
    GtkWidget *imgs[9];
    GtkWidget *label_status;
}app_widgets;

app_widgets *widgets;

enum{
    Move,
    Wait,
    Over
}states;

enum{
    Server,
    Client
}identities;

int state=Wait;
int identity;

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
    
    if((board[1] == board[2]) && (board[2] == board[3]))
    {
        winning[0] = 1; 
        winning[1] = 2;
        winning[2] = 3;
        return 1;
    }
    else if((board[4] == board[5]) && (board[5] == board[6]))
    {
        winning[0] = 4; 
        winning[1] = 5;
        winning[2] = 6;
        return 1;
    }
    else if((board[7] == board[8]) && (board[8] == board[9]))
    {
        winning[0] = 7; 
        winning[1] = 8;
        winning[2] = 9;
        return 1;
    }
    
    return 0;
}

int checkCols()
{
    if((board[1] == board[4]) && (board[4] == board[7]))
    {
        winning[0] = 1; 
        winning[1] = 4;
        winning[2] = 7;
        return 1;
    }
    else if((board[2] == board[5]) && (board[5] == board[8]))
    {
        winning[0] = 2; 
        winning[1] = 5;
        winning[2] = 8;
        return 1;
    }
    else if((board[3] == board[6]) && (board[6] == board[9]))
    {
        winning[0] = 3; 
        winning[1] = 6;
        winning[2] = 9;
        return 1;
    }
    return 0;
}

int checkDiags()
{
    if((board[1] == board[5]) && (board[5] == board[9]))
    {
        winning[0] = 1; 
        winning[1] = 5;
        winning[2] = 9;
        return 1;
    }
    if((board[3] == board[5]) && (board[5] == board[7]))
    {
        winning[0] = 3; 
        winning[1] = 5;
        winning[2] = 7;
        return 1;
    }
    
    return 0;
}
int checkWin()
{    
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

/*void read_position()      // used to read input from keyboard, deprecated
{
    int position;
    
    state=Move;
    
    printf("It's time to make a move, %c\nYour move: ", PlayerChar);
    while(move_done==0); //commented code only for reading values from terminal
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
    
    //current_move = position;
    state=Wait;
}*/

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

int your_turn()
{
    char move[MESSAGE_SIZE];
    int other_socket;
    //read_position();
    
    prevMove = playGame(PlayerChar, prevMove);
    
    sprintf(move, "%d", current_move);
        
    if(identity==Server){         // who am I?
        other_socket=client_socket;
    }
    else{
        other_socket=server_socket;
    }
    
    //make necessary checks
    if (checkWin() == 1)
    {
        state=Over;
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "You are the winner");
        printf("%d %d %d\n", winning[0], winning[1], winning[2]);
        printf("You won!\n");
    }
    if (checkDraw() == 1)
    {
        state=Over;
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "Draw. Exit game manu");
        printf("Draw!\n");
    }
    
    
    //send move to opponent
    if(send(other_socket, &move, MESSAGE_SIZE, 0) == -1)
    {
        printf("Can't send move\n");
        exit(-11);
    }
    
    printf("Sent move\n");
    
    
    
    return 0;
}

int opponent_turn()
{
    char move[MESSAGE_SIZE];
    int opponent_socket;
    int my_socket;
    
    printf("Preparing to listen.\n");
    
    if(identity==Server){ // who am I?
        
        my_socket=server_socket;
        opponent_socket=client_socket;
    }
    else{
        
        my_socket=client_socket;
        opponent_socket=server_socket;
    }
    
    
    gtk_label_set_text(GTK_LABEL(widgets->label_status), "Wait for opponent to move...");
    if(listen(my_socket, 5) == -1)
    {
        
        perror("listen: ");
        exit(-12);
    }

    printf("Wainting for the opponent's move...\n");
    if(recv(opponent_socket, &move, MESSAGE_SIZE, 0) == -1)
    {
        printf("Can't receive message\n");
        perror("recv: ");
        exit(-13);
    }

    if(strlen(move)!=1)
    {
        printf("Opponent forfeit\n");
        exit(0);
    }
    
    printf("Opponent's move: %c\n", move[0]);
    current_move = atoi(move);
    prevMove = playGame(OpponentChar, prevMove);
    gtk_image_set_from_file(GTK_IMAGE(widgets->imgs[current_move-1]), OpponentChar == '0' ? "images/0.png" : "images/x.png");
    
    if (checkWin() == 1)
    {
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "You are a loser");
        printf("Your opponent won!\n");
        printf("%d %d %d\n", winning[0], winning[1], winning[2]);
        state=Over;
    }
    if (checkDraw() == 1)
    {
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "Draw. Exit game manually");
        printf("Draw!\n");
        state=Over;
    }
    
    if(state!=Over){
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "Make a move");
        state=Move;
    }
    
    return 0;
}

/*int Game(char PlayerChar, char OpponentChar, int current_socket, int other_socket)
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
}*/

void on_main_window_destroy(){
    
    exit(0);
}

void on_btn_exit_game_clicked(){
  
    exit(0);
}

void on_btn_clicked(GtkButton *button,app_widgets *widgets){
    printf("Clicked\n");
    
    int btn_num;
    
    if(state!=Move){
        return;
    }
    btn_num = gtk_widget_get_name(GTK_WIDGET(button))[0] - '0';// find button that was pressed
    btn_num++; // in glade index starts at 0
    
    printf("btn_num=%d\n",btn_num);
    
    
    //necessary checks
    if(!validInput(btn_num)){
        printf("Invalid move, try again\n");
        return;
    }
    if(!validPosition(btn_num)){
        printf("That spot is already taken, try again\n");
        return;
    }
    
    // move is ok, put image 
    gtk_image_set_from_file(GTK_IMAGE(widgets->imgs[btn_num-1]), PlayerChar == '0' ? "images/0.png" : "images/x.png");
    
    current_move=btn_num;
    
    state=Wait; // block the other buttons until opponent made his move
    
    gtk_label_set_text(GTK_LABEL(widgets->label_status), "Wait for opponent to move...");
    
    your_turn(); // send move to opponent
    
    if(state!=Over){ // if game is not over listen for next opponent move
        opponent_turn(); 
    }
    
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
    
    widgets=g_slice_new(app_widgets);
    if(!widgets){
        printf("Error allocating memory!\n");
        exit(-15);
    }
    
    gchar str_img[] = "img_0";
    
     // Get a pointer to each image
    for (gint i = 0; i < 9; i++) {
        str_img[4] = i + '0';
        widgets->imgs[i] = GTK_WIDGET(gtk_builder_get_object(builder, str_img));
    }
    // Get a pointer to the status label
    widgets->label_status = GTK_WIDGET(gtk_builder_get_object(builder, "label_status"));
    
    gtk_builder_connect_signals(builder, widgets);
    
    
    
    //network connection
    int PORT = atoi(argv[2]); // PORT from command line
    char selection = argv[1][0];
    
    if(selection == 's') // server (host) part
    {
        identity=Server;
        srand(time(0));
        int random_number = rand() % 2; //Random number to determine which player goes first 
        
        if (random_number == 0)
        {
            PlayerChar = 'X';
            OpponentChar = '0';
            state=Move;
        }
        else
        {
            state=Wait;
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
        printf("Server socket=%d\n",server_socket);
        printf("Client sockect=%d\n",client_socket);
        
        char message[MESSAGE_SIZE];
        sprintf(message, "%c", OpponentChar);
        
        printf("You are playing as %c\n", PlayerChar);
        if(send(client_socket, message, MESSAGE_SIZE, 0) == -1)
        {
            printf("Error when sending message to client\n");
            exit(-5);
        }
        
        initializeBoard();
        /*while(1)
        {
            if (Game(PlayerChar, OpponentChar, server_socket, client_socket) == 1)
                break;
        }*/
        
        
        
        if(state==Wait){
            gtk_label_set_text(GTK_LABEL(widgets->label_status), "Wait for opponent to move...");
            opponent_turn();
            
        }
        
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "Make a move");
        
        gtk_widget_show((GtkWidget*)window);
        gtk_main();
        
        
        //close connection
        close(server_socket); //if error, it closes anyway
    }
    else //client side
    {
        int conn;
        identity=Client;
        
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
        printf("Server socket=%d\n",server_socket);
        printf("Client sockect=%d\n",client_socket);
        
        
        
        char message3[MESSAGE_SIZE];
        
        if(recv(server_socket, &message3, MESSAGE_SIZE, 0) == -1)
        {
            printf("Error fetching data from server\n");
            exit(-9);
        }
        printf("You are playing as %s\n", message3);
        
        if (message3[0] == '0')
        {
            state=Wait;
            PlayerChar = '0';
            OpponentChar = 'X';
        }
        else
        {
            state=Move;
            PlayerChar = 'X';
            OpponentChar = '0';
        }
        
        initializeBoard();
        
        
        
        /*while(1)
        {
            if (Game(PlayerChar, OpponentChar, client_socket, server_socket) == 1)
                break;
        }*/
        
        
        
        if(state==Wait){
            gtk_label_set_text(GTK_LABEL(widgets->label_status), "Wait for opponent to move...");
            opponent_turn();
        }
        
        gtk_label_set_text(GTK_LABEL(widgets->label_status), "Make a move");
        
        gtk_widget_show((GtkWidget*)window);
        gtk_main();
        
    }
    
    g_object_unref(builder);
    g_slice_free(app_widgets,widgets);
    return 0;
}

















