#ifndef HW_H
#define HW_H

/*
 * Address of memory bottom
 */
#define HEAP_START 0x50000

#define IRQ_MODE 0x12
#define SVC_MODE 0x13
#define SYS_MODE 0x1F

#define ENABLE_IRQ()				\
  asm("\tpush {r0}\n\t"				\
      "mrs r0,cpsr\n\t"				\
      "bic r0,r0,#0x80\n\t"			\
      "msr cpsr_c,r0\n\t"			\
      "pop {r0}"				\
      :						\
      :						\
      : "r0");

#define DISABLE_IRQ()				\
  asm("\tpush {r0}\n\t"				\
      "mrs r0,cpsr\n\t"				\
      "orr r0,r0,#0x80\n\t"			\
      "msr cpsr_c,r0\n\t"			\
      "pop {r0}"				\
      :						\
      :						\
      : "r0");

void enable_timer_irq();
void disable_timer_irq();
void set_tick_and_enable_timer();

void led_off();
void led_on();

void init_hw();

#endif
