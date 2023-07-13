#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

void compute_raw(int *arr, int *bin, int arr_size);
void compute_parallel1(int *arr, int *bin, int arr_size);
void compute_parallel2(int *arr, int *bin, int arr_size);
void compute_parallel3(int *arr, int *bin, int arr_size);


void compute_raw(int *arr, int *bin, int arr_size)
{
    for(int i = 0; i < arr_size; i++)
    {
        ++bin[arr[i]];
    }
}

void compute_parallel1(int *arr, int *bin, int arr_size)
{
    #pragma omp parallel for num_threads(40)
    for(int i = 0; i < arr_size; i++)
    {
        int c = arr[i];
        #pragma omp atomic
        ++bin[c];
    }
}

// 简单分块，跳跃取数操作
void compute_parallel2(int *arr, int *bin, int arr_size)
{
    #pragma omp parallel for num_threads(40)
    for (int i = 0; i < 40; i++)
    {   
        int temp_bin[256] = {};
        for (int j = i; j < arr_size; j += 40)
        {
            int c = arr[j];
            ++temp_bin[c];
        }

        for (int k = 0; k < 256; k++)
        {
            int a = temp_bin[k];
            #pragma omp atomic
            bin[k] += a;
        }
    }
}

// 简单分块，连续取数操作
void compute_parallel3(int *arr, int *bin, int arr_size)
{
    int op_size = (arr_size + 39) / 40; 
    #pragma omp parallel for num_threads(40)
    for (int i = 0; i < 40; i++)
    {   
        int l = op_size * i, r = op_size * (i + 1);
        if (r > arr_size) r = arr_size;

        int temp_bin[256] = {};
        for (int j = l; j < r; j++)
        {   
            int c = arr[j];
            ++temp_bin[c];
        }

        for (int k = 0; k < 256; k++)
        {
            int a = temp_bin[k];
            #pragma omp atomic
            bin[k] += a;
        }
    }
}





int main()
{
    const int arr_size = 50000000;
    int* arr = new int[arr_size];
    int* bin = new int[256]();

    // 生成随机数据
    srand(time(0));
    for(int i = 0; i < arr_size; i++)
    {
        arr[i] = rand() % 256;
    }

    // 测试 compute_raw 函数
    auto start = std::chrono::steady_clock::now();
    compute_raw(arr, bin, arr_size);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    auto elapsed_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "compute_raw elapsed time: " << elapsed_nanoseconds.count() << "ns\n";


    // 测试 compute_parallel1 函数
    start = std::chrono::steady_clock::now();
    compute_parallel1(arr, bin, arr_size);
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
    elapsed_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "compute_parallel1 elapsed time: " << elapsed_nanoseconds.count() << "ns\n";


    // 测试 compute_parallel2 函数
    start = std::chrono::steady_clock::now();
    compute_parallel2(arr, bin, arr_size);
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
    elapsed_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "compute_parallel2 elapsed time: " << elapsed_nanoseconds.count() << "ns\n";

    // 测试 compute_parallel2 函数
    start = std::chrono::steady_clock::now();
    compute_parallel3(arr, bin, arr_size);
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
    elapsed_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "compute_parallel3 elapsed time: " << elapsed_nanoseconds.count() << "ns\n";

    // 释放动态分配的内存
    delete[] arr;
    delete[] bin;
    return 0;
}
