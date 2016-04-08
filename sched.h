#ifndef SCHED_H
#define SCHED_H


jcb* pick_work();
void reschedule();
void reschedule_from_pthread();
void do_reschedule(void* esp);
void do_reschedule_reset_current();
void swicth_free_current(void* p_esp);
void free_current();


#endif