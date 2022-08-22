/*! [C11] Threads 

 \note Returning from func has the same behavior as invoking thrd_exit with the value returned from func.
*/
int  thrd_create(thrd_t *thr, thrd_start_t func, void *arg){
	osThread_t *thrd = malloc(sizeof(osThread_t)); // выделяется в памяти процесса
	thrd->func = func;
	thrd->arg  = arg;
	*thr = THREAD_ID(thrd);// запустить исполнение процесса
	atomic_list_push(&thread_current->next, thrd);// \todo очередь в памяти планировщика
	return thrd_success;
}
thrd_t thrd_current(void){
	return THREAD_ID(thread_current); // 
}
int  thrd_equal(thrd_t thr0, thrd_t thr1){
	return (thr0) == (thr1);
}
_Noreturn void thrd_exit(int res){
//	if (current_thread->parent) 
//		sigqueue(current_thread->parent, SIGCHLD, res)
	svc2(SVC_EXIT, thread_current, res);
	while(1);
}
/*! блокировка треда может быть основана на мьютексе 
Тред который запросил thrd_join  ожидает освобождение ресурса, т.е семафор
Мы предполагаем, что только один тред может запросить thrd_join
 */
int  thrd_detach(thrd_t thr){
	//assert(!thrd_equal (thr, thrd_current()->parent));
	semaphore_leave(thr->sem);
	return thrd_success;
}
/*!
	
 */
int  thrd_join(thrd_t thr, int *res){
	int count = semaphore_enter(&thr->sem);
	if (count >0) {// исполнение треда завершилось
		*res = thr->result;
		free(thr);
		return thrd_success;
	}
	return osEventWait(osEventSemaphore, &thr->sem);
}
/*! 
	\return zero if the requested time has elapsed, −1 if it has been interrupted
	by a signal, or a negative value (which may also be −1) if it fails.
*/
int  thrd_sleep(const struct timespec *duration, struct timespec *remaining){
	clock_t ts; 
	if (remaining) ts = clock();
	uint32_t interval = _timestamp(duration);	// преобразует интервал в микросекунды
	int res = svc(SVC_USLEEP, interval);
	if (remaining) {
		ts = clock() - ts;
		if (interval > ts){
			_timespec(remaining, interval - ts);// преобразует штамп времени в timespec
			return -1;
		} else {
			*remaining = (struct timespec){0,0};
		}
	}
	return 0;
}
void thrd_yield(void){
	svc(SVC_YIELD);
}
