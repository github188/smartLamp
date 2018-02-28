#include <stdio.h>
#include <stdlib.h>
#include <netinf.h>

extern char errorMessage[1024];

int main(int argc, char *argv[])
{
    int    listen_sockfd;
    int    sockfd;                 /* ����socket: sock_fd */
    int    new_fd;                 /* ���ݴ���socket: new_fd */
    struct sockaddr_in my_addr;    /* ������ַ��Ϣ */
    struct sockaddr_in their_addr; /* �ͻ���ַ��Ϣ */
    unsigned int sin_size;

    listen_sockfd = OpenSCPServer(8888, 1000, 8 * 1024, 8 * 1024, 0, 1);
    if(listen_sockfd < 0)
    {
        printf("OpenSCPServer failed! ret=%d, msg[%s]\n", listen_sockfd, errorMessage);
        return -1;
    }
    
    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        
        printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));
    }

    return 0;
}

