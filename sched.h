#ifndef SCHED_H
#define SCHED_H

#include "thread.h"

jcb *work_steal(int victim);

jcb *pick_work();

jcb *try_pick_work();

void reschedule();

void reschedule_from_pthread();

void do_reschedule(void *esp);

void do_reschedule_reset_current();

void swicth_free_current(void *p_esp);

void free_current();

void switch_context_to_new(void *esp);

void switch_context(void *esp);

void fstate_save();

void fstate_restore();

void gregor_srand(unsigned long seed);

#endif