
/*
Author: Jack Mcsweeney
This program compares multiple implementations of parallelism on Win32
The performance coparisons are summations of matrices growing by a factor of 10 each iteration

The takeaway should be the tradeoff between overhead and accuracy to ensure data races dont obstruct the results
*/

#include <Windows.h>
#include <chrono>
using namespace std::chrono;

const int THREADS = 8;
HANDLE handles[THREADS];
HANDLE myLock;
CRITICAL_SECTION myCriticalSection;

long long N;
double* A;
double totalSequential, totalParallel, totalParallelWithLock, totalParallelWithCritical, totalParallelWithSubtotal;

high_resolution_clock::time_point startTime;

void StartTiming() {
	startTime = high_resolution_clock::now();
}

double StopTiming() {
	duration<double> duration = high_resolution_clock::now() - startTime;
	return duration.count();

}

DWORD WINAPI sumNaive(LPVOID lpThreadParameter) {
	int threadId = (int)lpThreadParameter;
	long long blockSize = (N + THREADS - 1) / THREADS;
	long long start = threadId * blockSize;
	long long end min(start + blockSize, N);

	for (long long i = start; i < end; i++) {
		totalParallel += A[i];
	}
	return 0;
}

DWORD WINAPI sumWithLock(LPVOID lpThreadParameter) {
	int threadId = (int)lpThreadParameter;
	long long blockSize = (N + THREADS - 1) / THREADS;
	long long start = threadId * blockSize;
	long long end min(start + blockSize, N);

	for (long long i = start; i < end; i++) {

		//get lock
		WaitForSingleObject(myLock, INFINITE);
		totalParallelWithLock += A[i];
		ReleaseMutex(myLock);

		//release lock
	}
	return 0;
}

DWORD WINAPI sumWithCriticalSection(LPVOID lpThreadParameter) {
	int threadId = (int)lpThreadParameter;
	long long blockSize = (N + THREADS - 1) / THREADS;
	long long start = threadId * blockSize;
	long long end min(start + blockSize, N);

	for (long long i = start; i < end; i++) {

		//get lock
		EnterCriticalSection(&myCriticalSection);
		totalParallelWithCritical += A[i];
		LeaveCriticalSection(&myCriticalSection);

		//release lock
	}
	return 0;
}

DWORD WINAPI sumWithSubtotal(LPVOID lpThreadParameter) {
	int threadId = (int)lpThreadParameter;
	long long blockSize = (N + THREADS - 1) / THREADS;
	long long start = threadId * blockSize;
	long long end min(start + blockSize, N);
	double threadLocalSubtotal = 0;
	for (long long i = start; i < end; i++) {
		threadLocalSubtotal += A[i];
	}

	//get lock
	EnterCriticalSection(&myCriticalSection);
	totalParallelWithSubtotal += threadLocalSubtotal;
	LeaveCriticalSection(&myCriticalSection);

	//release lock
	return 0;
}

int main(int argc, char* argv[])
{
	myLock = CreateMutexA(0, FALSE, "MyLock");
	InitializeCriticalSection(&myCriticalSection);

	for (N = 10; true; N *= 10) {
		printf("N = %lld \n", N);

		A = new double[N];

		for (long long i = 0; i < N; i++) {
			A[i] = 1.0;
		}

		StartTiming();
		totalSequential = 0;

		for (long long i = 0; i < N; i++) {

			totalSequential += A[i];
		}

		double sequentialTime = StopTiming();

		printf("Sequential total %f, %.7f seconds \n", totalSequential, sequentialTime);



		//------------------------------------------------------------------------------------

		StartTiming();

		totalParallel = 0;
		for (int t = 0; t < THREADS; t++) {
			handles[t] = CreateThread(0, 0, sumNaive, (void*)t, 0, 0);

		}
		WaitForMultipleObjects(THREADS, handles, true, INFINITE);

		double parallelTime = StopTiming();

		printf("Parallel total %f, %.7f seconds \n", totalParallel, parallelTime);


		//------------------------------------------------------------------------------------

		StartTiming();

		totalParallelWithLock = 0;
		for (int t = 0; t < THREADS; t++) {
			handles[t] = CreateThread(0, 0, sumWithLock, (void*)t, 0, 0);

		}
		WaitForMultipleObjects(THREADS, handles, true, INFINITE);

		double parallelTimeWithLock = StopTiming();

		printf("ParallelWithLock total %f, %.7f seconds \n", totalParallelWithLock, parallelTimeWithLock);

		

		//------------------------------------------------------------------------------------

		StartTiming();

		totalParallelWithCritical = 0;
		for (int t = 0; t < THREADS; t++) {
			handles[t] = CreateThread(0, 0, sumWithCriticalSection, (void*)t, 0, 0);

		}
		WaitForMultipleObjects(THREADS, handles, true, INFINITE);

		double parallelTimeWithCriticalSection = StopTiming();

		printf("ParallelWith Crit total %f, %.7f seconds \n", totalParallelWithCritical, parallelTimeWithCriticalSection);


	


	//------------------------------------------------------------------------------------

		StartTiming();

		totalParallelWithSubtotal = 0;
		for (int t = 0; t < THREADS; t++) {
			handles[t] = CreateThread(0, 0, sumWithSubtotal, (void*)t, 0, 0);

		}
		WaitForMultipleObjects(THREADS, handles, true, INFINITE);

		double parallelTimeWithSubtotal = StopTiming();

		printf("ParallelWith Crit subtotal %f, %.7f seconds \n", totalParallelWithSubtotal, parallelTimeWithSubtotal);

		delete[] A;

}
	

	
	return 0;



}

