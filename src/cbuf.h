#ifndef _CBUF_H_
#define _CBUF_H_
#include <stdlib.h>

typedef struct {
	size_t buf_size;
	size_t entry_size;
	void* head;
	void* buffer;
} circ_buff;

circ_buff* circ_buff_new(size_t entry_size, size_t entry_count);
void  circ_buff_write(circ_buff* cbuf, void* data);
void* circ_buff_dump(circ_buff* cbuf);
void  circ_buff_clear(circ_buff* cbuf);
void  circ_buff_free(circ_buff* cbuf);
#endif
