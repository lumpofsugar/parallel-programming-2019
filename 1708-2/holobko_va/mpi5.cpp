#include "pch.h"
#include <omp.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cstring>

double* getRandomArray(int  sz) {
	std::random_device rd;
	std::mt19937 gen;
	gen.seed(static_cast<unsigned int>(rd()));
	double* Array = new double[sz];
	for (int i = 0; i < sz; i++) { Array[i] = (gen() % 100000) / 100.0 - 500.0; }
	return Array;
}

void CountingSortByte(double* inp, double* out, int byteNum, int size) {
	if (byteNum != 7) {
		unsigned char* mas = (unsigned char*)inp;
		int counter[256];
		int tem;
		memset(counter, 0, sizeof(int) * 256);
		for (int i = 0; i < size; i++) {
			counter[mas[8 * i + byteNum]]++;
		}
		int j = 0;
		for (; j < 256; j++) {
			if (counter[j] != 0)
				break;
		}
		tem = counter[j];
		counter[j] = 0;
		j++;
		for (; j < 256; j++) {
			int  sdvig = counter[j];
			counter[j] = tem;
			tem += sdvig;
		}
		for (int i = 0; i < size; i++) {
			out[counter[mas[8 * i + byteNum]]] = inp[i];
			counter[mas[8 * i + byteNum]]++;
		}
	}
	else {
		unsigned char* mas = (unsigned char*)inp;
		int counter[256];
		int tem = 0;
		memset(counter, 0, sizeof(int) * 256);
		for (int i = 0; i < size; i++)
			counter[mas[8 * i + byteNum]]++;
		int j = 255;
		for (; j >= 128; j--) {
			if (counter[j] != 0)
				break;
		}
		if (j != 127) {
			tem = counter[j] - 1;
			counter[j] = tem;
			j--;
			for (; j >= 128; j--) {
				int sdvig = counter[j];
				tem += sdvig;
				counter[j] = tem;
			}
		}
		tem++;
		j = 0;
		for (; j < 128; j++) {
			if (counter[j] != 0)
				break;
		}
		for (; j < 128; j++) {
			int sdvig = counter[j];
			counter[j] = tem;
			tem += sdvig;
		}
		for (int i = 0; i < size; i++) {
			out[counter[mas[8 * i + byteNum]]] = inp[i];
			if (mas[8 * i + byteNum] >= 128) {
				counter[mas[8 * i + byteNum]]--;
			}
			else {
				counter[mas[8 * i + byteNum]]++;
			}
		}
	}
}

void LSDSortDouble(double* inp, double* tmp, int size) {
	for (int i = 0; i < 7; i += 2) {
		CountingSortByte(inp, tmp, i, size);
		CountingSortByte(tmp, inp, i + 1, size);
	}
}

bool CompareArrays(double* mas, double* Mas, int size) {
	for (int i = 0; i < size; i++)
		if (mas[i] != Mas[i])
			return false;
	return true;
}

void EvenSplitter(double* mas, double* tmp, int elem, int size2) {
	for (int i = 0; i < elem; i += 2)
		tmp[i] = mas[i];
	double* mas2 = mas + elem;
	int a = 0;
	int b = 0;
	int i = 0;
	while ((a < elem) && (b < size2)) {
		if (tmp[a] <= mas2[b]) {
			mas[i] = tmp[a];
			a += 2;
		}
		else {
			mas[i] = mas2[b];
			b += 2;
		}
		i += 2;
	}
	if (a == elem) {
		for (int j = b; j < size2; j += 2, i += 2)
			mas[i] = mas2[j];
	}
	else {
		for (int j = a; j < elem; j += 2, i += 2)
			mas[i] = tmp[j];
	}
}

void OddSplitter(double* mas, double* tmp, int elem, int size2) {
	for (int i = 1; i < elem; i += 2)
		tmp[i] = mas[i];
	double* mas2 = mas + elem;
	int a = 1;
	int b = 1;
	int i = 1;
	while ((a < elem) && (b < size2)) {
		if (tmp[a] <= mas2[b]) {
			mas[i] = tmp[a];
			a += 2;
		}
		else {
			mas[i] = mas2[b];
			b += 2;
		}
		i += 2;
	}
	if (a == elem) {
		for (int j = b; j < size2; j += 2, i += 2)
			mas[i] = mas2[j];
	}
	else {
		for (int j = a; j < elem; j += 2, i += 2)
			mas[i] = tmp[j];
	}
}

void SimpleComparator(double* mas, int size) {
#pragma omp parallel for
	for (int i = 1; i < (size + 1) / 2; i++)
		if (mas[2 * i] < mas[2 * i - 1]) {
			double _tmp = mas[2 * i - 1];
			mas[2 * i - 1] = mas[2 * i];
			mas[2 * i] = _tmp;
		}
}

void LSDParallelSortDouble(double* inp, int size, int nThreads) {
	omp_set_num_threads(nThreads);
	double* out = new double[size];
	int portion = (size / nThreads);
	if (size % nThreads != 0)
		portion++;
	LSDParallelSorter(inp, out, size, portion);
	delete[] out;
}

void LSDParallelSorter(double* mas, double* tmp, int size, int portion) {
	if (size <= portion) {
		LSDSortDouble(mas, tmp, size);
	}
	else {
		int elem = size / 2 + (size / 2) % 2;
		LSDParallelSorter(mas, tmp, elem, portion);
		LSDParallelSorter(mas + elem, tmp + elem, size - elem, portion);
		EvenSplitter(mas, tmp, elem, size - elem);
		OddSplitter(mas, tmp, elem, size - elem);
		SimpleComparator(mas, size);
	}
}

double* array_double_copy(double* Array, int size) {
	double* Array_copy = new double[size];
	for (int i = 0; i < size; i++) {
		Array_copy[i] = Array[i];
	}
	return Array_copy;
}

int main(int argc, char** argv) {
	double test;
	int size = atoi(argv[1]);
	double* Array = getRandomArray(size);
	double* Array_copy = array_double_copy(Array, size);
	double* tmp = new double[size];
	LSDParallelSorter(Array, tmp, size, 2000);
	test = CompareArrays(Array, Array_copy, size);
	std::sort(Array_copy, Array_copy + size);
	if (test==true)
		std::cout << "Result_Test: True";
	else
		std::cout << "Result_Test: False";
	return 0;
}