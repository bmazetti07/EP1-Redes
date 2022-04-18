/* Por Prof. Daniel Batista <batista@ime.usp.br>
 * Em 16/3/2022
 * 
 * Um código simples de um servidor de eco a ser usado como base para
 * o EP1. Ele recebe uma linha de um cliente e devolve a mesma linha.
 * Teste ele assim depois de compilar:
 * 
 * ./ep1-servidor-exemplo 8000
 * 
 * Com este comando o servidor ficará escutando por conexões na porta
 * 8000 TCP (Se você quiser fazer o servidor escutar em uma porta
 * menor que 1024 você precisará ser root ou ter as permissões
 * necessárias para rodar o código com 'sudo').
 *
 * Depois conecte no servidor via telnet. Rode em outro terminal:
 * 
 * telnet 127.0.0.1 8000
 * 
 * Escreva sequências de caracteres seguidas de ENTER. Você verá que o
 * telnet exibe a mesma linha em seguida. Esta repetição da linha é
 * enviada pelo servidor. O servidor também exibe no terminal onde ele
 * estiver rodando as linhas enviadas pelos clientes.
 * 
 * Obs.: Você pode conectar no servidor remotamente também. Basta
 * saber o endereço IP remoto da máquina onde o servidor está rodando
 * e não pode haver nenhum firewall no meio do caminho bloqueando
 * conexões na porta escolhida.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

#include "auxiliar.h"

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096



int main (int argc, char **argv) {
    /* Os sockets. Um que será o socket que vai escutar pelas conexões
     * e o outro que vai ser o socket específico de cada conexão */
    int listenfd, connfd;
    /* Informações sobre o socket (endereço e porta) ficam nesta struct */
    struct sockaddr_in servaddr;
    /* Retorno da função fork para saber quem é o processo filho e
     * quem é o processo pai */
    pid_t childpid;
    /* Armazena linhas recebidas do cliente */
    char recvline[MAXLINE + 1];
    /* Armazena o tamanho da string lida do cliente */
    ssize_t n;
    /** Descritor de arquivo para o pipe **/
    int meu_pipe_fd[MAXTOPICS];
    /** Nome do arquivo temporário que vai ser criado **/
    char meu_pipe[MAXTOPICS][MAXDATASIZE + 1];

    char endChar = 'X';
   
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    /* Criação de um socket. É como se fosse um descritor de arquivo.
     * É possível fazer operações como read, write e close. Neste caso o
     * socket criado é um socket IPv4 (por causa do AF_INET), que vai
     * usar TCP (por causa do SOCK_STREAM), já que o MQTT funciona sobre
     * TCP, e será usado para uma aplicação convencional sobre a Internet
     * (por causa do número 0) */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    /* Agora é necessário informar os endereços associados a este
     * socket. É necessário informar o endereço / interface e a porta,
     * pois mais adiante o socket ficará esperando conexões nesta porta
     * e neste(s) endereços. Para isso é necessário preencher a struct
     * servaddr. É necessário colocar lá o tipo de socket (No nosso
     * caso AF_INET porque é IPv4), em qual endereço / interface serão
     * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
     * qual a porta. Neste caso será a porta que foi passada como
     * argumento no shell (atoi(argv[1]))
     */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    /* Como este código é o código de um servidor, o socket será um
     * socket passivo. Para isto é necessário chamar a função listen
     * que define que este é um socket de servidor que ficará esperando
     * por conexões nos endereços definidos na função bind. */
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
    /* O servidor no final das contas é um loop infinito de espera por
     * conexões e processamento de cada uma individualmente */
	for (;;) {
        /* O socket inicial que foi criado é o socket que vai aguardar
         * pela conexão na porta especificada. Mas pode ser que existam
         * diversos clientes conectando no servidor. Por isso deve-se
         * utilizar a função accept. Esta função vai retirar uma conexão
         * da fila de conexões que foram aceitas no socket listenfd e
         * vai criar um socket específico para esta conexão. O descritor
         * deste novo socket é o retorno da função accept. */
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }
      
        /* Agora o servidor precisa tratar este cliente de forma
         * separada. Para isto é criado um processo filho usando a
         * função fork. O processo vai ser uma cópia deste. Depois da
         * função fork, os dois processos (pai e filho) estarão no mesmo
         * ponto do código, mas cada um terá um PID diferente. Assim é
         * possível diferenciar o que cada processo terá que fazer. O
         * filho tem que processar a requisição do cliente. O pai tem
         * que voltar no loop para continuar aceitando novas conexões.
         * Se o retorno da função fork for zero, é porque está no
         * processo filho. */
        if ( (childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            printf("[Uma conexão aberta]\n");
            /* Já que está no processo filho, não precisa mais do socket
             * listenfd. Só o processo pai precisa deste socket. */
            close(listenfd);
         
            /* Agora pode ler do socket e escrever no socket. Isto tem
             * que ser feito em sincronia com o cliente. Não faz sentido
             * ler sem ter o que ler. Ou seja, neste caso está sendo
             * considerado que o cliente vai enviar algo para o servidor.
             * O servidor vai processar o que tiver sido enviado e vai
             * enviar uma resposta para o cliente (Que precisará estar
             * esperando por esta resposta) 
             */

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 INÍCIO                        */
            /* ========================================================= */
            /* ========================================================= */
            /* TODO: É esta parte do código que terá que ser modificada
             * para que este servidor consiga interpretar comandos MQTT  */
            while ((n=read(connfd, recvline, MAXLINE)) > 0) {
                int byte[8], size = 0;
                int indiceTopico;
                int tam = 0, x;
                int numBytes;
                char header[MAXLINE + 1];
                char topico[MAXDATASIZE+1] = "", c;
                char auxFile[MAXDATASIZE + 10];


                recvline[n]=0;

                /* Tamanho do pacote recebido */
                toBit (recvline[1], byte);
                size = calculaBit (byte, 8) + 2;
                printf ("Tamanho do n == %ld\n", n);
                printf ("Valor de size == %d\n", size);

                printf ("Pacote recebido:\n");
                for (int i = 0; i < size; i ++){
                    toBit (recvline[i], byte);

                    /* printf ("\n%c == ", recvline[i]); */
                    for (int j = 0; j < 8; j ++)
                        printf ("%d", byte[j]);
                    printf ("\n");
                }

                /* Identificação do tipo do pacote recebido */
                toBit (recvline[0], byte);
                int packetType = 0;
                packetType = calculaBit (byte, 4);
                
                printf ("Packet control type -> %d\n", packetType);

/* ----------------------------------------- CONNECT ------------------------------------------ */

                switch (packetType) {
                case 1:
                    /* Cliente enviou um pacote CONNECT */
                    /* Resposta do broker enviando um CONNACK */
                    printf ("Cliente requisitou uma conexão\n");
                    numBytes = 4;
                    strcpy (header, "");

                    header[0] = 32; /* primeiros 4 bits -> 0010; últimos 4 bits -> 0000 */
                    header[1] = 2;  /* Comprimento restante -> restam dois bytes no pacote */
                    header[2] = 0;  /* Flags de reconhecimento de conexão (Connect Acknowledge Flags) */
                    header[3] = 0;  /* Código de retorno -> 0 == COnexão bem sucedida */

                    write (connfd, header, numBytes);

                    break;

/* -------------------------------------------- PUBLISH -------------------------------------------- */

                case 3:
                    /* Cliente enviou um pacote PUBLISH */
                    /* Resposta do broke enviando a mensagem para os clientes inscritos no tópico */

                    printf ("Cliente está publicando uma mensagem\n");

                    tam = 0;
                    toBit (recvline[3], byte);
                    tam = calculaBit (byte, 8);

                    
                    strcpy (topico, "");
                    
                    // mosquitto_pub -h 192.168.1.108 -t 'topico' -p 8000 -m 'eae kraio'
                    for (int j = 4; j < 4 + tam; j ++) {
                        toBit (recvline[j], byte);
                        x = calculaBit (byte, 8);
                        c = x;
                        strncat (topico, &c, 1);
                    }

                    printf ("Tópico == %s\n", topico);

                    indiceTopico = procuraTopico(topico, topicos);
                    printf ("Indice tópico pub == %d\n", indiceTopico);


                    char mensagem[MAXDATASIZE + 1] = "";
                    for (int j = 4 + tam; j < size; j ++) {
                        toBit (recvline[j], byte);
                        x = calculaBit (byte, 8);

                        c = x;
                        strncat (mensagem, &c, 1);
                    }

                    printf ("Mensagem == %s\n", mensagem);


                    /* strncpy (meu_pipe[indiceTopico], auxFile, strlen (auxFile));
                    meu_pipe_fd[indiceTopico] = open (meu_pipe[indiceTopico], O_WRONLY);
                    unlink((const char *) meu_pipe[indiceTopico]); */


                    strcpy (auxFile, "");
                    strcat (topico, "-");
                    for (int i = 0; i < 6; i ++)
                        strncat (topico, &endChar, 1);
                    strncpy (auxFile, topico, strlen (topico));
                    strncpy (meu_pipe[indiceTopico], auxFile, strlen (auxFile));
                    
                    if (mkstemp (auxFile) < 1) {
                        printf ("Erro na criação do arquivo de pipe\n");
                        return 1;
                    }
                    else {
                        printf ("Arquivo de pipe temporário %s criado\n", auxFile);
                    }

                    meu_pipe_fd[indiceTopico] = open (auxFile, O_WRONLY);
                    unlink((const char *) auxFile);
                    write(meu_pipe_fd[indiceTopico], recvline, strlen(recvline));
                    close (meu_pipe_fd[indiceTopico]);
                    break;

/* -----------------------------------------  SUBSCRIBE --------------------------------------- */

                case 8:
                    numBytes = 5;
                    static char header[MAXLINE + 1];

                    header[0] = 144; /* primeiros 4 bits -> 1001; últimos 4 bits -> 0000 */
                    header[1] = 3;
                    toBit (recvline[2], byte);
                    header[2] = calculaBit (byte, 8);
                    toBit (recvline[3], byte);
                    header[3] = calculaBit (byte, 8);
                    header[4] = 0;

                    write (connfd, header, numBytes);


                    printf ("Cliente está se inscrevendo em um tópico\n");

                    tam = 0;
                    toBit (recvline[5], byte);
                    tam = calculaBit (byte, 8);

                    strcpy (topico, "");
                    // mosquitto_sub -h 192.168.1.108 -t 'topico' -p 8000
                    for (int j = 6; j < 6 + tam; j ++) {
                        toBit (recvline[j], byte);
                        x = calculaBit (byte, 8);
                        c = x;
                        strncat (topico, &c, 1);
                    }

                    printf ("Tópico == %s\n", topico);
                    indiceTopico = procuraTopico(topico, topicos);
                        printf ("Indice tópico sub == %d\n", indiceTopico);
                    printf ("Tópico selecionado -> %s\n", topicos[indiceTopico]);

                    meu_pipe_fd[indiceTopico] = open(meu_pipe[indiceTopico],O_RDONLY);
                    printf ("meu_pipe_fd[%d] == %d", indiceTopico, meu_pipe_fd[indiceTopico]);
                    while ((n=read(meu_pipe_fd[indiceTopico], recvline, MAXLINE)) > 0) {
                        recvline[n]=0;
                        printf ("Tamanho do n lido no subscribe == %ld\n", n);
                        write(connfd,         recvline, strlen(recvline));
                        printf ("\n\n\n\n\n\n\n\n");
                        toBit (recvline[1], byte);
                        size = calculaBit (byte, 8) + 2;
                        printf ("Tamanho do n == %ld\n", n);
                        printf ("Valor de size == %d\n", size);

                        printf ("Pacote recebido:\n");
                        for (int i = 0; i < size; i ++){
                            toBit (recvline[i], byte);

                            /* printf ("\n%c == ", recvline[i]); */
                            for (int j = 0; j < 8; j ++)
                                printf ("%d", byte[j]);
                            printf ("\n");
                        }

                        /* Identificação do tipo do pacote recebido */
                        toBit (recvline[0], byte);
                        int packetType = 0;
                        packetType = calculaBit (byte, 4);
                        
                        printf ("Packet control type -> %d\n", packetType);
                        printf ("\n\n\n\n\n\n\n\n");
                    }
                    close(meu_pipe_fd[indiceTopico]);
                    break;

/* ------------------------------------- DISCONNECT ------------------------------------------ */

                case 14:
                    printf ("Cliente está se desconectando\n");
                    break;
                
                default:
                    break;
                }

                /* printf("[Cliente conectado no processo filho %d enviou:] ",getpid());
                if ((fputs(recvline,stdout)) == EOF) {
                    perror("fputs :( \n");
                    exit(6);
                }
                write(connfd, recvline, strlen(recvline)); */
            }
            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 FIM                           */
            /* ========================================================= */
            /* ========================================================= */

            /* Após ter feito toda a troca de informação com o cliente,
             * pode finalizar o processo filho */
            printf("[Uma conexão fechada]\n");
            exit(0);
        }
        else
            /**** PROCESSO PAI ****/
            /* Se for o pai, a única coisa a ser feita é fechar o socket
             * connfd (ele é o socket do cliente específico que será tratado
             * pelo processo filho) */
            close(connfd);
    }
    exit(0);
}
