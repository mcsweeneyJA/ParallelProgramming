// SequentialSummationWithLocks.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <chrono>
using namespace std::chrono;

const int THREADS = 4;
HANDLE handles[THREADS];

long long N;
double* A;
double totalSequential, totalParallel, totalParallelWithLock;

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

int main(int argc, char* argv[])
{
	N = 10;

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

	return 0;



}

