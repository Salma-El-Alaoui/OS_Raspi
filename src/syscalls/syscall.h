#ifndef SYSCALL_H
#define SYSCALL_H

void SWIHandler();
void sys_reboot();
void sys_wait(unsigned int nbQuantums);
void sys_kill(unsigned process_ID);
void sys_waitpid(unsigned int process_ID);

#endif
