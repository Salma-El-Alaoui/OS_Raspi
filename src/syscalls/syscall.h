#ifndef SYSCALL_H
#define SYSCALL_H

void SWIHandler();
void sys_reboot();
void sys_wait(unsigned int nbQuantums);
void doSysCallReboot();
void doSysCallWait();

#endif
