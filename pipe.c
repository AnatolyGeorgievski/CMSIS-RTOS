#include <stddef.h>
#include "semaphore.h"
typedef struct _osPipe  *osPipeId;
struct _osPipe {
	volatile int count;
	uint16_t write_pos;
	uint16_t read_pos;
	uint16_t length;
	void* base[0];
};
int pipe_write(osPipeId pipe, void* data)
{
	if (pipe->count == pipe->length) return 0;
	if (pipe->write_pos==pipe->length) pipe->write_pos=0;
	pipe->base[pipe->write_pos++] = data;
	return semaphore_leave(&pipe->count);
}
void* pipe_read(osPipeId pipe)
{
	uint32_t count = semaphore_enter(&pipe->count);
	if (count==0) return NULL;
	if (pipe->read_pos==pipe->length) pipe->read_pos=0;
	return pipe->base[pipe->read_pos++];
}
int pipe_create (osPipeId pipe, size_t size, uint32_t length)
{
	pipe->count = 0;
//	pipe->array = malloc(length*(size+sizeof(void*)));
	pipe->write_pos = 0;
	pipe->read_pos = 0;
	pipe->length = length;
	return 0;
}
#if 0
void* pipe0_read(void** ptr) 
{
	return atomic_pointer_exchange(ptr, NULL);
}
void* pipe0_write(void** ptr, void*data) 
{
	return atomic_pointer_exchange(ptr, data);
}

#include "cmsis_os.h"
#include "thread.h"

typedef struct _Peer Peer_t;
struct _Peer {
	osProcess_t* thr;
	uint8_t signal_map[0];
	
};
void osSignalBind(Peer_t* peer, osProcess_t* thr, uint32_t signals)
{
	peer->thr = thr;
	int i=0;
	while (signals) {
		int idx = __builtin_ctz(~signals);// номер сигнала принимающей стороны
		peer->signal_map[i++] = idx;
		signals &= ~(1UL<<idx);
	}
}
#endif

/*
int pipe_release(osPipeId pipe, void* data)
{
	idx = (data - pipe->base)/pipe->size;
	pipe->release(, data);
}*/