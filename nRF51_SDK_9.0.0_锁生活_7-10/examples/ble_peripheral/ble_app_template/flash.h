#ifndef __FLASH_H
#define	__FLASH_H
	
#include "main.h"

void flash_init(void);
void receive_phone_num_update(uint8_t * update_num);
void receive_phone_num_read(uint8_t * read_num);
void receive_password_num_update(uint8_t * update_num);
void receive_password_num_read(uint8_t * read_num);
void time_begin_Lock_update(uint8_t * update_num);
void time_begin_Lock_read(uint8_t * read_num);

#endif /* __FLASH_H */
