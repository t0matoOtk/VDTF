#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define ITERATIONS 10000

typedef struct {
    long long user, nice, system, idle, iowait, irq, softirq;
} CpuStat;

long long timespec_diff_us(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1000000LL +
           (end->tv_nsec - start->tv_nsec) / 1000;
}

int cmp_ll(const void *a, const void *b)
{
    long long x = *(long long *)a;
    long long y = *(long long *)b;
    return (x > y) - (x < y);
}

void read_cpu_stat(CpuStat *s)
{
    FILE *f = fopen("/proc/stat", "r");
    fscanf(f, "cpu %lld %lld %lld %lld %lld %lld %lld",
           &s->user, &s->nice, &s->system, &s->idle,
           &s->iowait, &s->irq, &s->softirq);
    fclose(f);
}

double cpu_usage(CpuStat *before, CpuStat *after)
{
    long long busy_before = before->user + before->nice + before->system +
                            before->irq + before->softirq;
    long long busy_after  = after->user + after->nice + after->system +
                            after->irq + after->softirq;
    long long total_before = busy_before + before->idle + before->iowait;
    long long total_after  = busy_after  + after->idle  + after->iowait;
    long long busy_diff  = busy_after  - busy_before;
    long long total_diff = total_after - total_before;
    if (total_diff == 0) return 0.0;
    return 100.0 * busy_diff / total_diff;
}

void bench(const char *label, const char *path)
{
    char buf[64];
    struct timespec start, end;
    CpuStat cpu_before, cpu_after;
    int fd;
    long long total = 0;
    long long samples[ITERATIONS];

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("[%s] Cannot open %s\n", label, path);
        return;
    }

    printf("[%s] Running %d iterations...\n", label, ITERATIONS);
    read_cpu_stat(&cpu_before);

    for (int i = 0; i < ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        pread(fd, buf, sizeof(buf), 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        samples[i] = timespec_diff_us(&start, &end);
        total += samples[i];
    }

    read_cpu_stat(&cpu_after);
    close(fd);

    qsort(samples, ITERATIONS, sizeof(long long), cmp_ll);

    long long avg  = total / ITERATIONS;
    long long p50  = samples[ITERATIONS * 50  / 100];
    long long p90  = samples[ITERATIONS * 90  / 100];
    long long p95  = samples[ITERATIONS * 95  / 100];
    long long p99  = samples[ITERATIONS * 99  / 100];
    long long p999 = samples[ITERATIONS * 999 / 1000];

    printf("[%s] avg=%lld us  min=%lld us  max=%lld us\n",
           label, avg, samples[0], samples[ITERATIONS-1]);
    printf("[%s] p50=%lld us  p90=%lld us  p95=%lld us  p99=%lld us  p99.9=%lld us\n",
           label, p50, p90, p95, p99, p999);
    printf("[%s] throughput=%.1f reads/sec\n", label, 1000000.0 / avg);
    printf("[%s] cpu_usage=%.1f%%\n", label, cpu_usage(&cpu_before, &cpu_after));
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: bench <kernel|simple>\n");
        return 1;
    }

    if (strcmp(argv[1], "kernel") == 0) {
        printf("=== Kernel Driver Benchmark ===\n\n");
        bench("I2C kernel (IIO)",
              "/sys/bus/iio/devices/iio:device0/in_temp_input");
        printf("\n");
        bench("SPI kernel (IIO)",
              "/sys/bus/iio/devices/iio:device1/in_temp_input");
    } else if (strcmp(argv[1], "simple") == 0) {
        printf("=== Simple Driver Benchmark ===\n\n");
        bench("I2C simple (sysfs)",
              "/sys/bus/i2c/devices/1-0076/temperature");
        printf("\n");
        bench("SPI simple (sysfs)",
              "/sys/bus/spi/devices/spi0.0/temperature");
    } else {
        printf("Usage: bench <kernel|simple>\n");
        return 1;
    }

    printf("\n=== Done ===\n");
    return 0;
}
