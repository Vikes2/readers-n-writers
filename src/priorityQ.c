#include "priorityQ.h"

int Post(priosem_t *sem) {
	//monitor zwalniający wątek z kolejki
	if (Lock(&(sem->mutex)) == -1) {
		return -1;
	}

	sem->value++; // Zmniejsza liczbę wątków zablokowanych w semaphorze (ile na minusie tyle w kolejce)

	// Jeżeli jest jeszcze jakis czekający wątek
	if (sem->value <= 0 && IsThreadWaiting(sem)) {
		int prio = GetHighestWaitingPriority(sem); //pobierany jest najwyższy priorytet kolejki

		//Zmieniany jest stan kolejki
		sem->prio_waiting[prio]--; 
		sem->prio_released[prio]++;

		//wysyłany jest sygnał dla innych wątków
		if (Cond_broadcast(&(sem->cv), &(sem->mutex)) == 1) {
			return -1;
		}
	}
	//wyjście z monitora
	if (Unlock(&(sem->mutex)) == -1) {
		return -1;
	}
}

int Wait(priosem_t *sem, int prio) {
	//monitor dołączający wątek do kolejki
	if (Lock(&(sem->mutex)) == -1) {
		return -1;
	}

	sem->value--; //zwiększona liczba zablokowanych wątków (dodano do kolejki)

	if (sem->value < 0) { // Jeżeli są inne wątki czekające
		sem->prio_waiting[prio]++; // zwiększenie statusu kolejki podanego priorytetu
		while (sem->prio_released[prio] < 0) { //usypiany jest wątek do momentu  wypuszczenia wszystkich wątków na tym preiorytecie
			if (Cond_wait(&(sem->cv), &(sem->mutex)) == -1) {
				return -1;
			}
		}

		sem->prio_released[prio]--; //zwolnienie 
	}
	
	//wyjście z monitora
	if (Unlock(&(sem->mutex)) == -1) {
		return -1;
	}
}

int Lock(pthread_mutex_t *mutex) {
	if (pthread_mutex_lock(mutex) == -1) {
		return -1;
	}

	return 0;
}

int Unlock(pthread_mutex_t *mutex) {
	if (pthread_mutex_unlock(mutex) == -1) {
		return -1;
	}

	return 0;
}

int IsThreadWaiting(priosem_t *sem) {
	for (int i = 0; i < sem->threadsCount; i++) {
		if (sem->prio_waiting[i] > 0) {
			return 1;
		}
	}

	return 0;
}

int GetHighestWaitingPriority(priosem_t *sem) {
	for (int i = sem->threadsCount - 1; i >= 0; i--) {
		if (sem->prio_waiting[i] > 0) {
			return i;
		}
	}

	return -1;
}

int Cond_broadcast(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	if (pthread_cond_broadcast(cond) == -1) {
		return -1;
	}

	if (Unlock(mutex) == -1) {
		return -1;
	}

	return 0;
}

int Cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	if (pthread_cond_wait(cond, mutex) == -1) {
		return -1;
	}

	return 0;
}