#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/sched.h>

void jiq_timedout(unsigned long ptr)
{
    *((char *) 0x0c) = 0x00;
}

static int panic_init_module(void)
{
    struct timer_list jiq_timer;

    init_timer(&jiq_timer);	/* init the timer structure */
    jiq_timer.function = jiq_timedout;
    jiq_timer.data = 0x00;
    jiq_timer.expires = jiffies + 100;	/* one second */
    add_timer(&jiq_timer);

    return 0;
}

module_init(panic_init_module);
MODULE_LICENSE("GPL");
