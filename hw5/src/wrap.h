#ifndef wrap_h
#define wrap_h

#define RIO_BUFSIZE 8192
#define USRDISCONN -22

/* Persistent state for the robust I/O (Rio) package */
typedef struct rio_t{
    int   rio_fd;                /* Descriptor for this internal buf */
    int   rio_cnt;               /* Unread bytes in internal buf */
    char  *rio_bufptr;          /* Next unread byte in internal buf */
    char  rio_buf[RIO_BUFSIZE]; /* Internal buffer */
} rio_t;

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>



int GetAddrInfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

void Freeaddrinfo(struct addrinfo *res);

int GetNameInfo(const struct sockaddr *sa, socklen_t salen,
                       char *host, socklen_t hostlen,
                       char *serv, socklen_t servlen, int flags);

void *MMap(void *addr, size_t length, int port, int flags,
                  int fd, off_t offset);

int MUnmap(void *addr, size_t length);

int Close(int fildes);

int Accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);

int Socket(int domain, int type, int protocol);

int MakeSocketNotBlocking(int sfd);

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Connect(int socket, const struct sockaddr *address, socklen_t address_len);

int Listen(int sockfd, int backlog);

ssize_t Send(int sockfd, const void *buf, size_t len, int flags);

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t Sendmsg(int sockfd, const struct msghdr *msg, int flags);

/*Thread wrappers*/

int Thread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);

int Thread_join(pthread_t thread, void **value_ptr);

pthread_t Thread_self(void);

int Thread_cancel(pthread_t thread);

void Thread_exit(void *value_ptr);

int Thread_detach(pthread_t thread);

int Thread_attr_init(pthread_attr_t *attr);

int Thread_attr_destroy(pthread_attr_t *attr);

ssize_t Write(int fd, void* buf, size_t count);

ssize_t Read(int fd, void* buf, size_t count);

ssize_t Recv(int sockfd, void *buf, size_t len, int flags);

int Select(int numFileDescriptors, fd_set* setOfReadyFileDescriptors, struct timeval *timeout);

int Epoll_create (int __size);

int Epoll_ctl (int __epfd, int __op, int __fd, struct epoll_event *__event);

int Epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout);

ssize_t rio_read(rio_t *rioPointer, char *userBuffer, size_t nBytesRequested);

ssize_t rio_readlineb(rio_t *rioPointer, void *userBuffer, size_t maxlen);

ssize_t rio_writen(int fileDescriptor, void *userBuffer, size_t nBytes);


int Stat(char *path);

int StatSize(char *path);



#endif
