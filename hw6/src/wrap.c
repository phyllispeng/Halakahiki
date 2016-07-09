#include "wrap.h"




int GetAddrInfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res){

	int ret = getaddrinfo(node,service,hints,res);

	if(ret!=0){
		printf("%s\n", gai_strerror(ret));
	}
	return ret;
}

void Freeaddrinfo(struct addrinfo *res){
	freeaddrinfo(res);
}

int GetNameInfo(const struct sockaddr *sa, socklen_t salen,
				char *host, socklen_t hostlen,
				char *serv, socklen_t servlen, int flags){

	int ret = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);

	if( ret!= 0){
		printf("%s\n", gai_strerror(ret));
	}
	return ret;
}

void *MMap(void *addr, size_t length, int port, int flags,
                  int fd, off_t offset){

	void* ret = mmap(addr, length, port, flags, fd, offset);

	if(ret == MAP_FAILED){
		perror("Mapping Memory Failed");
	}

	return ret;
}

int MUnmap(void *addr, size_t length){
	int ret = munmap(addr, length);

	if(ret<0){
		perror("Unmapping Memory Failed (Panic Now)");
	}
	return ret;
}

int Close(int fildes){
	int ret = close(fildes);
	if(ret<0){
		perror("Closing file failed");
	}
	return ret;
}

int Open(const char *path, int oflags){
	int ret = open(path, oflags);
	if(ret<0){
		perror("Opening file failed");
	}
	return ret;
}

int Accept(int socket, struct sockaddr *restrict address,
			socklen_t *restrict address_len){
	int ret = accept(socket, address, address_len);

	if(ret<0){
		perror("Socket Failed Acceptance");
	}
	return ret;
}

int Socket(int domain, int type, int protocol){

	int ret = socket(domain, type, protocol);

	if(ret<0){
		perror("Failed to Create Socket");
	}
	return ret;
}

int MakeSocketNotBlocking (int sfd){
  int flags;

  flags = fcntl (sfd, F_GETFL, 0);

  flags |= O_NONBLOCK;

 fcntl (sfd, F_SETFL, flags);

  return 0;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){

	int ret = bind(sockfd, addr, addrlen);

	if(ret<0){
		perror("Failed to Bind");
                exit(0);
	}

	return ret;
}

int Listen(int sockfd, int backlog){

	int ret = listen(sockfd, backlog);

	if(ret<0){
		perror("Failed to Listen");
	}

	return ret;
}

int Connect(int socket, const struct sockaddr *address, socklen_t address_len){

	int ret = connect(socket, address, address_len);

	if(ret<0){
		perror("Failed To Connect");
	}

	return ret;

}

ssize_t Send(int sockfd, const void *buf, size_t len){

	ssize_t ret = send(sockfd, buf, len, 0);

	if(ret < 0 && errno == EBADF){
		ret = USRDISCONN;
	}else if(ret<0){
		perror("Failed To Send");
	}

	return ret;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen){

	ssize_t ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen);

	if(ret < 0){
		perror("Failed to SendTo");
	}

	return ret;
}

ssize_t Sendmsg(int sockfd, const struct msghdr *msg, int flags){

	ssize_t ret = sendmsg(sockfd, msg, flags);

	if(ret < 0){
		perror("Failed to SendMsg");
	}

	return ret;
}


int Thread_create(pthread_t *threadid, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg){
        
    if(threadid == NULL){
        pthread_t local;
        threadid = &local;
    }
    
    int ret = pthread_create(threadid, attr, start_routine, arg);

    if(ret < 0){
            perror("Failed to Create Thread");
    }

    return ret;
}

int Thread_join(pthread_t thread, void **value_ptr){

	int ret = pthread_join(thread, value_ptr);

	if(ret < 0){
		perror("Failed to Join Thread");
	}

	return ret;
}

int Unlock(pthread_mutex_t *mutex){
    int ret = pthread_mutex_unlock(mutex); 
    if(ret != 0){
        perror("Failed To Unlock");
    }
    return ret;
} 

int Lock(pthread_mutex_t *mutex){
    int ret = pthread_mutex_lock(mutex); 
    if(ret != 0){
        perror("Failed To Unlock");
    }
    return ret;
}

pthread_t Thread_self(void){
	return pthread_self();
}

int Thread_cancel(pthread_t thread){

	int ret = pthread_cancel(thread);

	if(ret < 0){
		perror("Failed to Cancel Thread");
	}

	return ret;
}

void Thread_exit(void *value_ptr){
	pthread_exit(value_ptr);
}

int Thread_detach(pthread_t thread){

	int ret = pthread_detach(thread);

	if(ret < 0){
		perror("Failed to Detach Thread");
	}

	return ret;
}

int Cond_init(pthread_cond_t *cond){
    int ret = pthread_cond_init(cond,NULL);
    if(ret!=0){
        perror("Failed to init Conditions");
    }
    return ret;
}

int Mutex_init(pthread_mutex_t *mutex){
    int ret = pthread_mutex_init(mutex, NULL);
    
    if(ret != 0){
        perror("Failed to init Mutex");
    }
    return ret;
}

int Thread_attr_init(pthread_attr_t *attr){

	int ret = pthread_attr_init(attr);

	if(ret < 0){
		perror("Failed to Init Attribute for Thread");
	}

	return ret;
}

int Thread_attr_destroy(pthread_attr_t *attr){

	int ret = pthread_attr_destroy(attr);

	if(ret < 0){
		perror("Failed to Destroy Attribute for Thread");
	}

	return ret;
}

int setThreadName(const char *name){
    int ret;
    
    if( strlen(name) > 16 ){
        ret = -1;
        return ret;
    }
    
    ret = prctl (PR_SET_NAME, name, 0, 0, 0);
    
    return ret;
}

ssize_t Write(int fd, void* buf, size_t count){

	ssize_t ret = write(fd,buf, count);

	if(ret<0){
		perror("Failed to Write");
	}
	return ret;
}

ssize_t Read(int fd, void* buf, size_t count){

	ssize_t ret = read(fd, buf, count);

	if(ret<0){
		perror("Failed to Read");
	}
	return ret;
}

ssize_t Recv(int sockfd, void *buf, size_t len){

	memset(buf, 0, len);

	ssize_t ret = recv( sockfd,  buf,  len,  0);

	if(ret < 0){
		perror("Recv Error / Peer is probably dead");
	}
	if(*(char*)buf == '\0'){
		ret = -2;
	}

	return ret;
}

int Select(int numFileDescriptors, fd_set* setOfReadyFileDescriptors, struct timeval *timeout){
    /*
      int select(int nfds, fd_set *readfds, fd_set *writefds,
          fd_set *exceptfds, struct timeval *timeout);
    */
    int ret;
    //errno = 0; // DON'T DO THIS:
    // if for some reason there is a previous error in our program that went unhandled
    // resetting errno will hide it from us.
    // when project is complete there should never be errnos set so we'll never have errounous
    // errros
    ret = select(numFileDescriptors, setOfReadyFileDescriptors, NULL, NULL, timeout);

	if(ret < 0){
	  perror("There was an error with select.\r\n");
	  return 0; //failure
	}

    /*
        On success, select() and pselect() return the number of file descriptors
        contained in the three returned descriptor sets
        (that is, the total number of bits that are set in
        readfds, writefds, exceptfds) which may be zero if
        the timeout expires before anything interesting happens.
        On error, −1 is returned, and errno is set to indicate the error;
        the file descriptor sets are unmodified, and timeout becomes undefined.
    */
    return ret; //success!
}


int Stat(char *path){
/*
	#include <sys/stat.h>
	int stat(const char *restrict path, struct stat *restrict buf);

  Upon successful completion, 0 shall be returned.
	Otherwise, -1 shall be returned and errno set to indicate the error.
 */

	struct stat *myStat;
	myStat = malloc(sizeof(struct stat));
	int statVal;

	statVal = stat(path, myStat);

	if(statVal == 0){
		/* the file exists! */
    free(myStat);
		return 1;
	}else{
		int errnum = errno;
		fprintf(stderr, "Stat error:%d\n", errnum);
    free(myStat);
		return 0;
	}
}

int StatSize(char *path){

	struct stat *myStat;
	myStat = malloc(sizeof(struct stat));
	int statVal;

	statVal = stat(path, myStat);

	if(statVal == 0){
		/* the file exists! */
    //what's the size?
    int size = myStat->st_size;

    free(myStat);
		return size;
	}else{
		int errnum = errno;
		fprintf(stderr, "Stat error:%d\n", errnum);
    free(myStat);
		return 0;
	}
}






int Epoll_create (int flag){
	int ret = epoll_create1(flag);
	if(ret<0){
		perror("Unable to create Epoll");
	}
	return ret;
}

int Epoll_ctl (int __epfd, int __op, int __fd, struct epoll_event *__event){
	int ret = epoll_ctl(__epfd, __op, __fd, __event);
	if(ret<0){
		perror("Unable to Epoll ctl");
	}
	return ret;
}

int Epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout){
	int ret = epoll_wait(__epfd, __events, __maxevents, __timeout);
	if(ret<0){
		perror("Unable to wait on Epoll");
	}
	return ret;
}
