#include "sched.h"
static void put_prev_task_mypriority(struct rq *rq, struct task_struct *p);
static int select_task_rq_mypriority(struct task_struct *p, int cpu, int sd_flag, int flags);
static void set_curr_task_mypriority(struct rq *rq);
static void task_tick_mypriority(struct rq *rq,struct task_struct *p, int oldprio);
static void switched_to_mypriority(struct rq *rq, struct task_struct *p);
void init_mypriority_rq(struct mypriority_rq *mypriority_rq);
static void update_curr_mypriority(struct rq *rq);
static void enqueue_task_mypriority(struct rq *rq, struct task_struct *p, int flags);
static void dequeue_task_mypriority(struct rq *rq, struct task_struct *p, int flags);
static void check_preempt_curr_mypriority(struct rq *rq, struct task_struct *p,int flags);
struct task_struct *pick_next_task_mypriority(struct rq *rq, struct task_struct *prev);
static void prio_changed_mypriority(struct rq *rq, struct task_struct *p, int oldprio);

#define MYPRIORITY_TIME_SLICE 4
const struct sched_class mypriority_sched_class={
	.next=&fair_sched_class,
	.enqueue_task=&enqueue_task_mypriority,
	.dequeue_task=dequeue_task_mypriority,
	.check_preempt_curr=check_preempt_curr_mypriority,
	.pick_next_task=pick_next_task_mypriority,
	.put_prev_task=put_prev_task_mypriority,
#ifdef CONFIG_SMP
	.select_task_rq=select_task_rq_mypriority,
#endif
	.set_curr_task=set_curr_task_mypriority,
	.task_tick=task_tick_mypriority,
	.prio_changed=prio_changed_mypriority,
	.switched_to=switched_to_mypriority,
	.update_curr=update_curr_mypriority,
};


void init_mypriority_rq (struct mypriority_rq *mypriority_rq)
{
	printk(KERN_INFO "***[MYPRIORITY] Mysched class is online \n");
	mypriority_rq->nr_running=0;
	INIT_LIST_HEAD(&mypriority_rq->queue);

}
static void update_curr_mypriority(struct rq *rq){
	
	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	struct list_head *queue = &mypriority_rq->queue;
	struct sched_mypriority_entity *mypriority_en = container_of(queue->next, struct sched_mypriority_entity, run_list);
	unsigned int *update_num = &mypriority_en->update_num;
	struct task_struct *curr = NULL;

	*update_num += 1;
	curr = get_current();

	if(*update_num > MYPRIORITY_TIME_SLICE){
 		list_move_tail(queue->next, queue);
		*update_num = 0;
		resched_curr(rq);
	}
	else printk("***[MYPRIORITY] update_curr_mypriority	pid=%d update_num=%d\n", curr->pid, *update_num);

}

static void enqueue_task_mypriority(struct rq *rq, struct task_struct *p, int flags) {

	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	struct list_head *queue = &mypriority_rq->queue;
	struct sched_mypriority_entity *mypriority_en = &p->mypriority;
	struct list_head *run_list = &mypriority_en->run_list;

	list_add_tail(run_list, queue);
	mypriority_rq->nr_running++;

	printk("***[MYPRIORITY] enqueue: success cpu=%d, nr_running=%d, pid=%d\n", cpu_of(rq), mypriority_rq->nr_running, p->pid);

}
static void dequeue_task_mypriority(struct rq *rq, struct task_struct *p, int flags) 
{	
	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	struct sched_mypriority_entity *mypriority_en = &p->mypriority;
	struct list_head *run_list = &mypriority_en->run_list;
	
	if(mypriority_rq->nr_running > 0){
		list_del_init(run_list);
		mypriority_rq->nr_running--;
		printk("***[MYPRIORITY] dequeue: success cpu=%d, nr_running=%d, pid=%d\n", cpu_of(rq), mypriority_rq->nr_running, p->pid);
	}
}
void check_preempt_curr_mypriority(struct rq *rq, struct task_struct *p, int flags) {
	printk("***[MYPRIORITY] check_preempt_curr_mypriority\n");
}
struct task_struct *pick_next_task_mypriority(struct rq *rq, struct task_struct *prev)
{
	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	struct list_head *queue = &mypriority_rq->queue;
	struct task_struct *next_p = NULL;
	struct sched_mypriority_entity *next_en = NULL;
		
	if(mypriority_rq->nr_running == 0){
		return NULL;
	}

	next_en = container_of(queue->next, struct sched_mypriority_entity, run_list);
	next_p = container_of(next_en, struct task_struct, mypriority);
	printk("***[MYPRIORITY] pick_next_task : cpu=%d, prev->pid=%d,next_p->pid=%d,nr_running=%d\n", cpu_of(rq), prev->pid, next_p->pid, mypriority_rq->nr_running);
	return next_p;

}
void put_prev_task_mypriority(struct rq *rq, struct task_struct *p) {
	printk(KERN_INFO "\t***[MYPRIORITY] put_prev_task: do_nothing, p->pid=%d\n",p->pid);
}
int select_task_rq_mypriority(struct task_struct *p, int cpu, int sd_flag, int flags){return task_cpu(p);}
void set_curr_task_mypriority(struct rq *rq){
	printk(KERN_INFO"***[MYPRIORITY] set_curr_task_mypriority\n");
}
void task_tick_mypriority(struct rq *rq, struct task_struct *p, int queued) {
	update_curr_mypriority(rq);
}
void prio_changed_mypriority(struct rq *rq, struct task_struct *p, int oldprio) { }
/*This routine is called when a task migrates between classes*/
void switched_to_mypriority(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}
