#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

class Binded
{
public:
    int OnRead(int n) { return n_ = n; }
private:
    int n_;
};

bool __attribute__((optimize("-O0"))) lyambda_Benchmark(int loop)
{
    auto b = new Binded();

    for (int i = 0; i < loop; ++i)
    {
        auto f = [b, i] { return b->OnRead(i); };
        int r = f();
        (void) r;
    }
    delete b;
    return true;
}

bool std_bind_Benchmark(int loop)
{
    auto b = new Binded();

    for (int i = 0; i < loop; ++i)
    {
        auto f = std::bind(&Binded::OnRead, b, std::placeholders::_1);
        int r = f(i);
        (void) r;
    }
    delete b;
    return true;
}

typedef std::function<bool(int)> BenchmarkFunctor;

void Benchmark(BenchmarkFunctor f, int loop, const std::string & name)
{
    auto start
        = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    bool rc = f(loop);

    if (rc)
    {
        auto end
            = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        auto cost = double(end - start) / loop;
        std::cout << name << " loop=" << loop << " cost=" << cost << "s op=" << double(end - start) * 1000.0 / loop
                  << "ns/op QPS=" << (loop / cost) / 1024.0 << "k (" << double(end - start) << " s)\n";
    }
    else
    {
        std::cerr << name << " failed\n";
    }
}

int main()
{
    int loop = 2000 * 1000 * 1000;
    Benchmark(&std_bind_Benchmark, loop, "   std::bind");
    Benchmark(&lyambda_Benchmark, loop, "     lyambda");
    return 0;
}
