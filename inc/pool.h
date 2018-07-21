#ifndef POOL_H_
#define POOL_H_

/*********** pool.h *************/
typedef struct tpool_work{
	void (*handler_routine)();
    void *arg;
	struct tpool_work *next;
}tpool_work_t;

typedef struct tpool{
	int num_threads;
	int max_queue_size;
	int do_not_block_when_full;
	pthread_t *threads;
	int cur_queue_size;
	tpool_work_t *queue_head;
	tpool_work_t *queue_tail;
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_not_full;
	pthread_cond_t queue_not_empty;
	pthread_cond_t queue_empty;
	int queue_closed;
	int shutdown;
}tpool_t;

extern tpool_t *tpool_init(int num_worker_threads, int max_queue_size, int do_not_block_when_full);
extern int tpool_add_work(tpool_t *pool,void(*routine)(void *),void *arg);
extern int tpool_destroy(tpool_t *pool, int finish);

#endif
