#ifndef __MOTOR_H
#define	__MOTOR_H

#include "main.h"

#define motor_z			4
#define motor_f			3
#define motor_en		2

void motor_init(void);
void motor_key_state(uint8_t state_flag);
void Vmotor_choose(bool Vmotor_flag);

#endif /* __MOTOR_H */
