#include "../inc/main.h"

void *tpool_thread(void *tpool);
/***************线程池初始化*****************************/
tpool_t *tpool_init(int num_worker_threads, int max_queue_size, int do_not_block_when_full) // 是否阻塞任务满的时候
{
	int i, rtn;
	tpool_t *pool;
	size_t size;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
    pthread_attr_getstacksize(&attr, &size);
	size = size/10;
    pthread_attr_setstacksize(&attr, size);		// 设置栈大小

	if ((pool = (struct tpool *) malloc(sizeof(struct tpool))) == NULL)	// 创建线程池结构体 
	{
		lprintf(lfd, FATAL, "TPOOL: Unable to malloc thread pool!");
		return NULL;
	}
	pool->num_threads = num_worker_threads; 	// 工作线程个数
	pool->max_queue_size = max_queue_size; 		// 任务链表最大长度
	pool->do_not_block_when_full = do_not_block_when_full; 	//任务链表满时是否等待
	if ((pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * num_worker_threads)) == NULL) // 生成线程池缓存
	{
		lprintf(lfd, FATAL, "TPOOL: Unable to malloc thread info array!");
		return NULL;
	}
	// 初始化任务链表
	pool->cur_queue_size = 0;
	pool->queue_head = NULL;
	pool->queue_tail = NULL;
	pool->queue_closed = 0;
	pool->shutdown = 0;
	
	if ((rtn = pthread_mutex_init(&(pool->queue_lock), NULL)) != 0)	// 初始化互斥变量，条件变量 用于线程之间的同步
	{
		lprintf(lfd, FATAL, "TPOOL: pthread_mutex_init %s", strerror(rtn));
		return NULL;
	}
	if ((rtn = pthread_cond_init(&(pool->queue_not_empty), NULL)) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread_cond_init %s", strerror(rtn));
		return NULL;
	}
	if ((rtn = pthread_cond_init(&(pool->queue_not_full), NULL)) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread_cond_init %s", strerror(rtn));
		
		return NULL;
	}
	if ((rtn = pthread_cond_init(&(pool->queue_empty), NULL)) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread_cond_init %s", strerror(rtn));
		
		return NULL;
	}

	for (i = 0; i < num_worker_threads; i++)
	{
		if ((rtn = pthread_create(&(pool->threads[i]), &attr, tpool_thread,
				(void*) pool)) != 0)
		{ 
			lprintf(lfd, FATAL, "TPOOL: pthread_create %s", strerror(rtn));
			return NULL;
		}
	}
	return pool;
}

int tpool_add_work(tpool_t *pool,void(*routine)(void *),void *arg)
{
	int rtn;
	tpool_work_t *workp; 	// 当前工作线程
	if ((rtn = pthread_mutex_lock(&pool->queue_lock)) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread mutex lock failure!");
		return -1;
	}
	if ((pool->cur_queue_size == pool->max_queue_size) && (pool->do_not_block_when_full))	// 采取独占的形式访问任务链表
	{
		if ((rtn = pthread_mutex_unlock(&pool->queue_lock)) != 0)
		{
			lprintf(lfd, FATAL, "TPOOL: pthread mutex lock failure!");
			return -1;
		}
		return -1;
	}

	while ((pool->cur_queue_size == pool->max_queue_size) && (!(pool->shutdown || pool->queue_closed))) // 等待任务链表为新线程释放空间
	{
		if ((rtn = pthread_cond_wait(&(pool->queue_not_full),&(pool->queue_lock))) != 0)
		{
			lprintf(lfd, FATAL, "TPOOL: pthread cond wait failure!");
			return -1;
		}
	}
	if (pool->shutdown || pool->queue_closed)
	{
		if ((rtn = pthread_mutex_unlock(&pool->queue_lock)) != 0)
		{
			lprintf(lfd, FATAL, "TPOOL: pthread mutex lock failure!");
			return -1;
		}
		return -1;
	}
	if ((workp = (tpool_work_t *) malloc(sizeof(tpool_work_t))) == NULL)		// 分配工作线程结构体
	{
		lprintf(lfd, FATAL, "TPOOL: Unable to create work struct!");
		return -1;
	}
	workp->handler_routine = routine;
	workp->arg = arg;
	workp->next = NULL;
	if (pool->cur_queue_size == 0)
	{
		pool->queue_tail = pool->queue_head = workp;
		if ((rtn = pthread_cond_broadcast(&(pool->queue_not_empty))) != 0)
		{
			lprintf(lfd, FATAL, "TPOOL: pthread broadcast error!");
			return -1;
		}
	}
	else
	{
		pool->queue_tail->next = workp;
		pool->queue_tail = workp;
	}
	pool->cur_queue_size++;
	if ((rtn = pthread_mutex_unlock(&pool->queue_lock)) != 0)	// 释放对任务链表的独占
	{
		lprintf(lfd, FATAL, "TPOOL: pthread mutex lock failure!");
		return -1;
	}
	return 0;
}

int tpool_destroy(tpool_t *pool, int finish)
{
	int i, rtn;
	tpool_work_t *cur; 	// 当前工作线程
	lprintf(lfd, INFO, "TPOOL: Destroy pool begin!");
	
	if ((rtn = pthread_mutex_lock(&(pool->queue_lock))) != 0)	// 释放对任务链表的独占 
	{
		lprintf(lfd, FATAL, "TPOOL: pthread mutex lock failure!");
		return -1;
	}

	if (pool->queue_closed || pool->shutdown)
	{
		if ((rtn = pthread_mutex_unlock(&(pool->queue_lock))) != 0)
		{
			lprintf(lfd, FATAL, "TPOOL: pthread mutex lock failure!");
			return -1;
		}
		return 0;
	}

	pool->queue_closed = 1;
	if (finish)
	{
		while (pool->cur_queue_size != 0)
		{
			if ((rtn = pthread_cond_wait(&(pool->queue_empty),
					&(pool->queue_lock))) != 0)
			{
				lprintf(lfd, FATAL, "TPOOL: pthread_cond_wait %d", rtn);
				return -1;
			}
		}
	}

	pool->shutdown = 1;
	if ((rtn = pthread_mutex_unlock(&(pool->queue_lock))) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread mutex unlock failure");
		return -1;
	}

	if ((rtn = pthread_cond_broadcast(&(pool->queue_not_empty))) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread_cond_boradcast %d", rtn);
		return -1;
	}
	if ((rtn = pthread_cond_broadcast(&(pool->queue_not_full))) != 0)
	{
		lprintf(lfd, FATAL, "TPOOL: pthread_cond_boradcast %d", rtn);
		return -1;
	}
	for (i = 0; i < pool->num_threads; i++)
	{
		if ((rtn = pthread_join(pool->threads[i], NULL)) != 0)
		{
			lprintf(lfd, FATAL, "TPOOL: pthread_join %d", rtn);
			return -1;
		}
	}

	free(pool->threads);
	while (pool->queue_head != NULL)
	{
		cur = pool->queue_head->next;
		pool->queue_head = pool->queue_head->next;
		free(cur);
	}
	free(pool);
	lprintf(lfd, INFO, "TPOOL: destroy pool end!");
	return 0;
}

void *tpool_thread(void *tpool)
{
	tpool_work_t *my_work;
	tpool_t *pool = (struct tpool *) tpool;

	while(1)
	{
		pthread_mutex_lock(&(pool->queue_lock));
		
		while ((pool->cur_queue_size == 0) && (!pool->shutdown)) // 如果任务列表为0，并且线程池没有关闭，则一直等待，直到任务到来为止
		{
			pthread_cond_wait(&(pool->queue_not_empty), &(pool->queue_lock));
		}
		if (pool->shutdown)		// 线程池是否已经关闭，如果线程池关闭则线程自己主动关闭
		{
			pthread_mutex_unlock(&(pool->queue_lock));
			pthread_exit(NULL); 	// 线程退出状态为空，主线程不捕获各副线程状态
		}
		my_work = pool->queue_head;
		pool->cur_queue_size--;

		if (pool->cur_queue_size == 0)						// 将任务链表头部去掉，改任务正在处理中
			pool->queue_head = pool->queue_tail = NULL;
		else
			pool->queue_head = my_work->next;
		if ((!pool->do_not_block_when_full) && (pool->cur_queue_size== (pool->max_queue_size - 1)))	// 任务链表还没有满
			pthread_cond_broadcast(&(pool->queue_not_full));
		if (pool->cur_queue_size == 0)			// 任务链表为空
			pthread_cond_signal(&(pool->queue_empty));
		pthread_mutex_unlock(&(pool->queue_lock));
		(*(my_work->handler_routine))(my_work->arg);			// 启动线程业务处理逻辑
		free(my_work); 	// 这里释放的是链表头部的线程结构体
	}
	return (NULL);
}

