#ifndef SYSCALL_H
#define SYSCALL_H

void SWIHandler();
void sys_reboot();
void sys_wait(unsigned int nbQuantums);
void sys_kill(unsigned process_id);
void sys_wait_pid(unsigned int process_id);


#endif
