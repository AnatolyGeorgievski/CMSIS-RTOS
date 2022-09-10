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
