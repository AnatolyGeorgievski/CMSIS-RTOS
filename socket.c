#include <stdint.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "atomic.h"
#include "semaphore.h"


#include "r3_slice.h"

typedef struct _List List_t;
struct _List {
    void* 	data;
    List_t* next;
};

/*
struct _queue {
    volatile List_t *head;
    volatile List_t *tail;
};
static inline void async_queue_init(struct _queue* queue)
{
    queue->head=NULL;
    queue->tail=NULL;
}

static List_t * list_reverse(List_t* list)
{
    List_t* top = NULL;
    while (list) {
        List_t* list_next = list->next;
        list->next = top;
        top = list;
        list = list_next;
    }
    return top;
}
static void* async_queue_get(struct _queue* queue)
{
    void* data = NULL;
    if (queue->head==NULL) {
//        volatile void** ptr = (volatile void**);
        queue->head = atomic_pointer_exchange(&queue->tail, NULL);
        if (queue->head) {
            queue->head = list_reverse((List_t*)queue->head);
        }
    }
    if (queue->head) {
        volatile List_t* list = queue->head;
        queue->head = list->next;
        data = list->data;
		//*size= list->size;
        g_slice_free1(sizeof(List_t), (void*)list);
    }
    return data;
}
static void async_queue_push(struct _queue* queue, void* data)
{
    List_t* tr = g_slice_alloc(sizeof(List_t));
    tr->data = data;
	//tr->size = size;
    volatile void** ptr = (volatile void**)&queue->tail;
    do {
        tr->next = atomic_pointer_get(ptr);
        atomic_mb();
    } while(!atomic_pointer_compare_and_exchange(ptr, tr->next, tr));
}
*/
static void _list_atomic_prepend(List_t ** list, void* data)
{
	List_t* elem = g_slice_alloc(sizeof(List_t));
	elem->data = data;
	volatile void** ptr = (volatile void**)list;
    do {
        elem->next = atomic_pointer_get(ptr);
        atomic_mb();
    } while(!atomic_pointer_compare_and_exchange(ptr, elem->next, elem));
}

#include <sys/socket.h>

#include "../lib/net.h"
typedef struct iodesc TCB; // определен в het.h
#ifndef MAXSOCKET
#define MAXSOCKET 32
#endif
// 256 шт
//static volatile int sockets_count = MAXSOCKET;// семафор
static volatile int sockets_allocated[MAXSOCKET/32]={
	[0 ... (MAXSOCKET/32-1)] = ~0};
static List_t*ip_udp_tcb=NULL;// список активных сокетов UDP
static List_t*ip_tcp_tcb=NULL;// список активных сокетов TCP
struct _socket {
	uint8_t protocol;// IPPROTO_UDP _TCP _ICMP
	uint8_t type;// SOCK_DGRAM
	uint8_t backlog;
	

//	uint16_t recv_timeout; // ms
//	uint16_t send_timeout; // ms
	union {
		struct {
			uint8_t acceptcon:1;
			uint8_t broadcast:1;
			uint8_t keepalive:1;
			uint8_t connect  :1;
			uint8_t shutdown :1;
		}__attribute__((packed));
		uint8_t flags;
	}__attribute__((packed));
	struct iodesc* tcb;
	struct _socket* next;
//	struct _queue queue;
};

static struct _socket* sock[MAXSOCKET]={NULL};// указатели

// int socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

/*! \brief освободить сокет */
static void socket_free(int sock_fd)
{
	int map, idx = sock_fd>>5;
	sock_fd &= 0x1F;// номер бита
	do {
		map = atomic_int_get(&sockets_allocated[idx]);
	} while (!atomic_int_compare_and_exchange(&sockets_allocated[idx], map, map | (1UL<<sock_fd)));
}
/*! \todo увеличить количество сокетов до максимального */
static int socket_alloc()
{
	int sock_fd;
	int map, idx=0;
	do {
		map = atomic_int_get(&sockets_allocated[idx]);
		if (map == 0) return (-1);// нет свободных
		sock_fd = __builtin_ctz(map);
	} while (!atomic_int_compare_and_exchange(&sockets_allocated[idx], map, map & ~(1UL<<sock_fd)));
	return sock_fd | (idx<<5);
}
static void socket_atomic_prepend(struct _socket** next, struct _socket* elem)
{
	volatile void** ptr = (volatile void**)next;
	do {
		elem->next = atomic_pointer_get(ptr);
		__DMB();
	} while (!atomic_pointer_compare_and_exchange(ptr, elem->next, elem));
}


/*! \defgroup POSIX_NETWORKING POSIX: Networking
	\ingroup _posix
accept( ), bind( ), connect( ), endhostent( ), endnetent( ), endprotoent( ), endservent( ),
freeaddrinfo( ), gai_strerror( ), getaddrinfo( ), gethostent( ), gethostname( ), getnameinfo( ),
getnetbyaddr( ), getnetbyname( ), getnetent( ), getpeername( ), getprotobyname( ),
getprotobynumber( ), getprotoent( ), getservbyname( ), getservbyport( ), getservent( ),
getsockname( ), getsockopt( ), htonl( ), htons( ), if_freenameindex( ), if_indextoname( ),
if_nameindex( ), if_nametoindex( ), inet_addr( ), inet_ntoa( ), inet_ntop( ), inet_pton( ), listen( ),
ntohl( ), ntohs( ), recv( ), recvfrom( ), recvmsg( ), send( ), sendmsg( ), sendto( ), sethostent( ),
setnetent( ), setprotoent( ), setservent( ), setsockopt( ), shutdown( ), socket( ), sockatmark( ),
socketpair( )
	\{ */
/*! 

теория операции: номер сокета равен номеру флага при ожидании события 
*/
int socket(int domain, int type, int protocol)
{
	int sock_fd = -1;
	if (domain==AF_UNIX) {
		
	} else
	if (domain==AF_INET) {
		sock_fd = atomic_flag_alloc(socket_flags); // выделяет номер сокета
		if (sock_fd<0) return sock_fd;
//		sock[sock_fd].domain = AF_INET;
		struct _socket *s = [sock_fd];
		if (s==NULL)
			sock[sock_fd] = s = malloc(sizeof(struct _socket));
		__builtin_bzero(s, sizeof (struct _socket));
		s->type     = type;
		s->protocol = protocol;
	} else 
		sock_fd=(-1);// INVALID_SOCKET
	return sock_fd;
}
void closesocket(int sock_fd)
{
	socket_free(sock_fd);
}
int getsockopt(int sock_fd, int level, int option_name, 
		void *restrict option_value, socklen_t *restrict option_len)
{
	int res = -1;
	return res;
}
/*! 	
	All SO_* socket options apply equally to IPv4 and IPv6 
	(except SO_BROADCAST, since broadcast is not implemented in IPv6).
 */
int setsockopt(int sock_fd, int level, int option_name, 
		const void *option_value, socklen_t option_len)
{
	struct _socket* s = DEV_PTR(sock_fd);
	if (s==NULL) return (-1);
	int res = 0;
	if (level==SOL_SOCKET)
	switch (option_name) {
	case SO_ACCEPTCONN:
		s->acceptcon = *(int32_t*)option_value;
		break;
	case SO_BROADCAST:
		s->broadcast = *(int32_t*)option_value;
		break;
	case SO_KEEPALIVE:
		s->keepalive = *(int32_t*)option_value;
		break;
	case SO_RCVTIMEO: // struct timeval recv_timeout
		break;
	case SO_SNDTIMEO: //  struct timeval send_timeout
		//sock[sock_fd].send_timeout = *(uint32_t*)option_value;
		break;
	default:
		res = -1;
		break;
	}
	return res;
}
osStatus osServiceBind(osProcess_t* svc, int socket, uint32_t mask)
{
	struct iodesc* tcb = sock[socket]->tcb;
	if (tcb!=NULL) {
		tcb->process = svc;
		tcb->signal  = mask;// маску можно разделить по событиям
		tcb->rx_count=0;
	} else {
		printf("ACHTUNG TCB==NULL!!! \r\n");
	}
	return osOK;
}
int bind (int socket, const struct sockaddr *address, socklen_t address_len)
{
	int res = -1;
	if (address->sa_family == AF_INET /* && sock[sock_fd].domain==AF_INET */) {
		const struct sockaddr_in* sin = (const struct sockaddr_in*)address;
		// выбрать интерфейс???
		// ETH_service(sock[sock_fd].protocol, sin->sin_port, service);
		if (sock[socket]->protocol==IPPROTO_UDP || sock[socket]->protocol==IPPROTO_TCP) {
			if (sin->sin_port==0) return -1;// 
			struct iodesc* tcb = malloc(sizeof(struct iodesc));
			__builtin_bzero(tcb, sizeof(struct iodesc));
			sock[socket]->tcb = tcb;
			tcb->src_port = (sin->sin_port);
			// выбрать сетевой интерфейс по ip адресу
			tcb->io_netif = netif_get_interface(sin->sin_addr.s_addr);
			tcb->src_ip.s_addr = tcb->io_netif->ip_address;
			
			if (sock[socket]->protocol==IPPROTO_UDP){
				_list_atomic_prepend(&ip_udp_tcb, sock[socket]);
				printf("BIND to UDP port %d\r\n", ntohs(tcb->src_port));
				res=0;
			} else 
			if (sock[socket]->protocol==IPPROTO_TCP) {
				_list_atomic_prepend(&ip_tcp_tcb, sock[socket]);
				printf("BIND to TCP port %d\r\n", ntohs(tcb->src_port));
				res=0;
			}
		}
	} else 
	if (address->sa_family == AF_INET6) {
		
	}
	return res;
}

#if 0
ssize_t recvfrom(int socket, void *restrict buffer, size_t length,
       int flags, struct sockaddr *restrict address,
       socklen_t *restrict address_len)
{
	int rx_size = (-1);
	if (sock[socket].acceptcon) {
		//void* rx_buffer=NULL;
		void* desc = async_queue_get(&sock[socket].queue);//, &rx_size);
		//void* rx_buffer = ...(desc, &rx_size);
		
		//UDP_header* udp_header = (UDP_header*)(rx_buffer+sizeof(IP_header));
		if (rx_size>0) {
			IP_header*  ip_header  = (IP_header*) (rx_buffer + sizeof(ETH_header));
			IP_UDP_header* udp_header = (IP_UDP_header*)(rx_buffer+sizeof(ETH_header)+sizeof(IP_header));
			struct sockaddr_in *sin = (struct sockaddr_in *)address;
			__builtin_memcpy(&sin->sin_addr, ip_header->ip_src, 4);
			sin->sin_family= AF_INET;
			sin->sin_port = udp_header->src_port;
			if (address_len) 
			   *address_len = sizeof(struct sockaddr_in);

			int hsize = sizeof(ETH_header)+ sizeof(IP_header)+sizeof(IP_UDP_header);// FCS
			__builtin_memcpy(buffer, rx_buffer + hsize, rx_size - hsize);
		}
		ETH_ReleaseFrame(desc);
	}
	return rx_size;
}
#endif
// SO_ZEROCOPY MSG_ZEROCOPY 
ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	
	
} 
#if 0
ssize_t sendto(int socket, 
		const void *message, size_t length,
		int flags, 
		const struct sockaddr *dest_addr, socklen_t dest_len)
{
	struct iodesc* tcb =  sock[socket].tcb;// добыть дескриптор по номеру сокета
	if ((flags & MSG_ZEROCOPY)==0) {// копировать буффер
		void* tx_buffer = udp_get_tx_buffer(tcb, length);
		__builtin_memcpy(tx_buffer, message, length);
		message = tx_buffer;
	}
	if (dest_addr!=NULL && dest_addr->sa_family==AF_INET){
		const struct sockaddr_in *peer = (const struct sockaddr_in *)dest_addr;
		tcb->dst_ip.s_addr = peer->sin_addr.s_addr;
		tcb->dst_port = peer->sin_port;
	}
	return udp_sendto(tcb, (void*)message, length);
}
#endif
struct iodesc* socket_iodesc(int socket)
{
	return sock[socket]->tcb;
}
int shutdown(int socket, int how)
{
	sock[socket]->shutdown = 1;
}
/*! 

	function shall mark a connection-mode socket, specified by the socket argument, 
	as accepting connections.

	The backlog argument provides a hint to the implementation 
	which the implementation shall use to limit the number of outstanding connections 
	in the socket's listen queue. Implementations may impose a limit on backlog 
	and silently reduce the specified value. Normally, a larger backlog argument value 
	shall result in a larger or equal length of the listen queue. 
	Implementations shall support values of backlog up to SOMAXCONN, defined in <sys/socket.h>.
 */
int listen (int socket, int backlog)
{
	if(backlog > SOMAXCONN) backlog=SOMAXCONN;
	else 
	if(backlog < SOMINCONN) backlog=SOMINCONN;
		
//	sock[socket]->tcb = tcp_init(sock[socket]->tcb);// create TCB
	
	sock[socket]->backlog = backlog;// количество соединений
	sock[socket]->acceptcon = 1;// разрешить соединение и обработку данных
}
/*! \brief установить соединение, получить адрес 

	function shall extract the first connection on the queue of pending connections, 
	create a new socket with the same socket type protocol and address family as the specified socket, and allocate a new file descriptor for that socket.

*/
int accept (int socket, struct sockaddr *restrict address, socklen_t *restrict address_len)
{
	//sock[socket].acceptcon = 1;// разрешить соединение и обработку данных
	if (sock[socket]->backlog > 0) {
		int sock_fd = socket_alloc();
		if (sock_fd<0) return sock_fd;
		__builtin_memcpy(sock[sock_fd], sock[socket], sizeof(struct _socket));
		//! \todo подправить некоторые поля после копирования
		socket_atomic_prepend(&sock[socket]->next, sock[sock_fd]);
		//sock[sock_fd].tcb = tcp_tcb_new();//
		sock[socket]->backlog--;
		socket = sock_fd;
	}
	
	sock[socket]->acceptcon=0;
	struct iodesc* tcb = sock[socket]->tcb;
	//if (sock[socket].protocol)
	if (address) {
		struct sockaddr_in * peer = (struct sockaddr_in *)address;
		peer->sin_family = AF_INET;
		peer->sin_addr.s_addr = tcb->dst_ip.s_addr;// проверить dst<->src
		peer->sin_port = tcb->dst_port;
	}
	*address_len = sizeof(struct sockaddr_in);
	return socket;
}

int connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
	sock[socket]->connect = 1;
	if (address->sa_family == AF_UNIX) {
		const struct sockaddr_un * sun = (const struct sockaddr_un *)address
		// int fildes = open(sun->sun_path, O_RDWR);
		return fildes;
	}
}

/*!
\todo разделить на две функции

основа - разбор шапки пакета ethernet. Разбор шапки может происходить в прерывании или в приложении.
*/
int socket_in_push(int protocol, uint8_t* buf, size_t framelength, void* desc)
{
	int res = -1;
	uint16_t sport = (*(uint16_t*)(buf+0x22));
	uint16_t dport = (*(uint16_t*)(buf+0x24));
	if (protocol == IPPROTO_UDP) {
		printf(" IPv4: Proto=UDP Src Port=%d Dst Port=%d, Len=%d\r\n", ntohs(sport), ntohs(dport), framelength);
		//uint16_t sport = ntohs(*(uint16_t*)(buf+0x22));
//		printf("UDP Push %d\r\n", dport);

		List_t* list = ip_udp_tcb;
		while (list){
			// можно сразу добыть tcb
			struct _socket* sock = (struct _socket*)list->data;
			struct iodesc* tcb = sock->tcb;
			if (tcb->src_port == dport) {
				tcb->dst_ip.s_addr  = *(in_addr_t*)(buf+14+12);
				__builtin_memcpy(tcb->dst_mac, buf+6, 6);
//				arp_table_set_mac(tcb->io_netif, ip_addr, mac);
				printf("UDP Indication %d\r\n", ntohs(dport));
				res = ether_data_indication(tcb, desc);
				break;
			}
			list = list->next;
		}
		if (list==NULL){
			//отослать icmp_request(ICMP_UNREACHABLE, ICMP_PORT_UNREACHABLE)// 
		}
	} else 
	if (protocol == IPPROTO_TCP) {
		// выполнить обработку состояния. Если соединние установлено обработка данных
		printf(" IPv4: Proto=UDP Src Port=%d Dst Port=%d, Len=%d\r\n", ntohs(sport), ntohs(dport), framelength);

		List_t* list = ip_tcp_tcb;
		while (list){
			struct _socket* sock = (struct _socket*)list->data;
			TCB* tcb = sock->tcb;
			if (tcb->src_port == dport) {
				uint16_t etype;// 0x0800 =IP
				IP_header *ip_header=NULL;
				ssize_t rx_size = readether_zc(tcb, (void**)&ip_header, 0, &etype);
				IP_TCP_header *tcp_header = (IP_TCP_header *)(ip_header+1);
				
				// у нас есть буфер его надо куда-то направить
				// надо найти подходящий сокет в цепочке sock->next;
				// if (sock->acceptcon && ip_header->SYN) -- запрос на соединение
				
				// 
				uint32_t signals=0;// добавить сигналы от приложения
				//if (tcp_state_m(tcb, tcp_header, rx_size-sizeof(IP_header), sock->flags)==0) 
				{// завершить обработку если статус CLOSED
					// очистить очередь пакетов
					free(sock->tcb);
					sock->tcb = NULL;
		//			int sock_fd = socket_fd(sock);// найти индекс сокета
		//			socket_free(sock_fd);
				}
			}
			list = list->next;
		}
	} else 
	if (protocol == IPPROTO_ICMP){
		
	}
	return res;
}
	//!\}