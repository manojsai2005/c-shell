#include "headers.h"
#include "que.h"

void stripHTML(char *htmlContent)
{
    char *readPtr = htmlContent;
    char *writePtr = htmlContent;
    int insideTag = 0;

    while (*readPtr)
    {
        if (*readPtr == '<')
        {
            insideTag = 1;
        }
        else if (*readPtr == '>')
        {
            insideTag = 0;
            readPtr++;
            continue;
        }
        if (!insideTag)
        {
            *writePtr++ = *readPtr;
        }
        readPtr++;
    }
    *writePtr = '\0';
}

void fetchManPage(const char *url,char *command)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("man.he.net", "80", &hints, &res) != 0)
    {
        perror("getaddrinfo");
        exit(1);
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    char request[4096];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: man.he.net\r\nConnection: close\r\n\r\n", url);

    if (send(sockfd, request, strlen(request), 0) == -1)
    {
        perror("send");
        close(sockfd);
        exit(1);
    }

    char buffer[4096];
    int bytes_received;
    char *htmlContent = (char *)malloc(50000);
    if (!htmlContent)
    {
        perror("malloc");
        close(sockfd);
        return;
    }
    strcpy(htmlContent, "");
    int totalLength = 0;

    while ((bytes_received = recv(sockfd, buffer, 4095, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        strcat(htmlContent, buffer);
        totalLength += bytes_received;
        // if (totalLength > 99000)
        //     break;
    }

    if (bytes_received < 0)
    {
        perror("recv");
    }

    close(sockfd);
    freeaddrinfo(res);
    stripHTML(htmlContent);
    printf("%s\n", htmlContent);
    free(htmlContent);
}

void iMan(char *command)
{
    if (command == NULL)
    {
        printf("Usage: iMan <command>\n");
        return;
    }

    char *url = (char *)malloc(sizeof(char) * 4096);
    if (url == NULL)
    {
        perror("malloc");
        return;
    }

    snprintf(url, 4096, "http://man.he.net/?topic=%s&section=all", command);

    fetchManPage(url,command);
    free(url);
}