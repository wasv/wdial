#include <stdlib.h>
#include <string.h>
#include "cbuf.h"

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

circ_buff* circ_buff_new(size_t entry_size, size_t entry_count) {
	circ_buff *cbuf = malloc(sizeof(circ_buff));
	cbuf->buf_size = entry_size * entry_count;
	cbuf->entry_size = entry_size;
	cbuf->buffer = calloc(entry_count, entry_size);
	if(cbuf->buffer == NULL) {
		free(cbuf);
		return NULL;
	}
	cbuf->head = cbuf->buffer;
	return cbuf;
}

void circ_buff_write(circ_buff* cbuf, void* data) {
	memcpy(cbuf->head, data, cbuf->entry_size);
	cbuf->head += cbuf->entry_size;
	if(cbuf->head >= (cbuf->buffer + cbuf->buf_size)) {
		cbuf->head = cbuf->buffer;
    }
}

void *circ_buff_dump(circ_buff* cbuf) {
	void *dest = malloc(cbuf->buf_size);
	size_t to_end = (cbuf->buffer + cbuf->buf_size) - cbuf->head;
	memcpy(dest, cbuf->head, to_end);
	memcpy(dest+to_end, cbuf->buffer, cbuf->buf_size - to_end);
	return dest;
}

void circ_buff_clear(circ_buff* cbuf) {
	bzero(cbuf->buffer, cbuf->buf_size);
}

void circ_buff_free(circ_buff* cbuf) {
	free(cbuf->buffer);
	free(cbuf);
}
