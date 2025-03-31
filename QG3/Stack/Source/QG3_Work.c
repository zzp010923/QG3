#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#define Maxsize 200000   // 支持最大20万数据量
#define DATA_FILE "test_data.txt"

typedef struct Data {
    int Array[Maxsize+1]; // Array[0]作为哨兵或闲置
    int length;
} Data;

// ================= 排序算法实现 =================
void swap(Data *L, int i, int j) {
    int temp = L->Array[i];
    L->Array[i] = L->Array[j];
    L->Array[j] = temp;
}

// 插入排序
void InsertSort(Data *L) {
    int i, j;
    for (i = 2; i <= L->length; i++) {
        if (L->Array[i] < L->Array[i-1]) {
            L->Array[0] = L->Array[i];
            for (j = i-1; L->Array[j] >= L->Array[0]; j--) {
                L->Array[j+1] = L->Array[j];
            }
            L->Array[j+1] = L->Array[0];
        }
    }
}

// 归并排序辅助函数
void Merge(int SR[], int TR[], int low, int mid, int high) {
    int i = low, j = mid + 1, k = low;
    while (i <= mid && j <= high) {
        if (SR[i] <= SR[j]) TR[k++] = SR[i++];
        else TR[k++] = SR[j++];
    }
    while (i <= mid) TR[k++] = SR[i++];
    while (j <= high) TR[k++] = SR[j++];
}

void MergeApart(int SR[], int TR1[], int low, int high) {
    if (low >= high) return;
    int mid = (low + high) / 2;
    int* TR2 = malloc((high + 1) * sizeof(int));
    MergeApart(SR, TR2, low, mid);
    MergeApart(SR, TR2, mid + 1, high);
    Merge(TR2, TR1, low, mid, high);
    free(TR2);
}

void MergeSort(Data *L) {
    MergeApart(L->Array, L->Array, 1, L->length);
}

// 快速排序
int Partition(Data *L, int low, int high) {
    int mid = low + (high - low)/2;
    if (L->Array[low] > L->Array[mid]) swap(L, low, mid);
    if (L->Array[low] > L->Array[high]) swap(L, low, high);
    if (L->Array[mid] > L->Array[high]) swap(L, mid, high);
    swap(L, mid, low);

    int pivotkey = L->Array[low];
    while (low < high) {
        while (low < high && L->Array[high] >= pivotkey) high--;
        L->Array[low] = L->Array[high];
        while (low < high && L->Array[low] <= pivotkey) low++;
        L->Array[high] = L->Array[low];
    }
    L->Array[low] = pivotkey;
    return low;
}

void QuickSort(Data *L, int low, int high) {
    if (low < high) {
        int pivot = Partition(L, low, high);
        QuickSort(L, low, pivot-1);
        QuickSort(L, pivot+1, high);
    }
}

// 计数排序
void CountSort(Data *L) {
    if (L->length <= 0) return;

    int max = L->Array[1];
    for (int i = 1; i <= L->length; i++) {
        if (L->Array[i] > max) max = L->Array[i];
    }

    int* count = calloc(max+1, sizeof(int));
    if (!count) return;

    for (int i = 1; i <= L->length; i++) {
        count[L->Array[i]]++;
    }

    int index = 1;
    for (int val = 0; val <= max; val++) {
        while (count[val]-- > 0) {
            L->Array[index++] = val;
        }
    }
    free(count);
}

// 基数排序
void RadixCountSort(Data *L) {
    if (L == NULL || L->length <= 0) return;

    int max = L->Array[1];
    for (int i = 1; i <= L->length; i++) {
        if (L->Array[i] > max) max = L->Array[i];
    }

    int *output = malloc((L->length + 1) * sizeof(int));
    if (!output) return;

    int exp;
    int count[10];
    for (exp = 1; max/exp > 0; exp *= 10) {
        memset(count, 0, sizeof(count));

        for (int i = 1; i <= L->length; i++) {
            int digit = (L->Array[i]/exp) % 10;
            count[digit]++;
        }

        for (int i = 1; i < 10; i++) {
            count[i] += count[i-1];
        }

        for (int i = L->length; i >= 1; i--) {
            int digit = (L->Array[i]/exp) % 10;
            output[count[digit]--] = L->Array[i];
        }

        for (int i = 1; i <= L->length; i++) {
            L->Array[i] = output[i];
        }
    }
    free(output);
}

// ================= 数据生成与测试 =================
// 生成测试数据并保存到文件
void generate_data(int size, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create file");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        fprintf(fp, "%d\n", rand() % 10000); // 生成0~9999的随机数
    }

    fclose(fp);
    printf("Generated %s with %d data\n", filename, size);
}

// 从文件加载数据到结构体
void load_data(Data* L, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    L->length = 0;
    int value;
    while (fscanf(fp, "%d", &value) != EOF && L->length < Maxsize) {
        L->Array[++L->length] = value; // 从Array[1]开始存储
    }

    fclose(fp);
}

// 获取当前时间（秒）
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// 测试大数据量性能
void test_large_scale() {
    const char* files[] = {"data_1e4.txt", "data_5e4.txt", "data_2e5.txt"};
    const int sizes[] = {10000, 50000, 200000};

    for (int i = 0; i < 3; i++) {
        Data data;
        load_data(&data, files[i]);
        printf("\n=== Testing %d elements ===\n", sizes[i]);

        // 测试插入排序（大数据时非常慢！）
        Data data_copy;
        memcpy(&data_copy, &data, sizeof(Data));
        double start = get_time();
        InsertSort(&data_copy);
        printf("InsertSort:\t%.3f s\n", get_time() - start);

        // 测试归并排序
        memcpy(&data_copy, &data, sizeof(Data));
        start = get_time();
        MergeSort(&data_copy);
        printf("MergeSort:\t%.3f s\n", get_time() - start);

        // 测试快速排序
        memcpy(&data_copy, &data, sizeof(Data));
        start = get_time();
        QuickSort(&data_copy, 1, data_copy.length);
        printf("QuickSort:\t%.3f s\n", get_time() - start);

        // 测试计数排序
        memcpy(&data_copy, &data, sizeof(Data));
        start = get_time();
        CountSort(&data_copy);
        printf("CountSort:\t%.3f s\n", get_time() - start);

        // 测试基数排序
        memcpy(&data_copy, &data, sizeof(Data));
        start = get_time();
        RadixCountSort(&data_copy);
        printf("RadixSort:\t%.3f s\n", get_time() - start);
    }
}

// 测试小数据多次排序
void test_small_repeated() {
    Data origin, temp;
    load_data(&origin, "small_data.txt");
    const int iterations = 100000;
    printf("\n=== Testing 100 elements * 100,000 times ===\n");

    // 测试插入排序
    double start = get_time();
    for (int i = 0; i < iterations; i++) {
        memcpy(&temp, &origin, sizeof(Data));
        InsertSort(&temp);
    }
    printf("InsertSort:\t%.3f s\n", get_time() - start);

    // 测试快速排序
    start = get_time();
    for (int i = 0; i < iterations; i++) {
        memcpy(&temp, &origin, sizeof(Data));
        QuickSort(&temp, 1, temp.length);
    }
    printf("QuickSort:\t%.3f s\n", get_time() - start);
}

int main() {
    // 生成测试数据
    //generate_data(10000, "data_1e4.txt");
    generate_data(50000, "data_5e4.txt");
   //generate_data(200000, "data_2e5.txt");
    generate_data(100, "small_data.txt");

    // 执行性能测试
    //test_large_scale();
    test_small_repeated();

    return 0;
}