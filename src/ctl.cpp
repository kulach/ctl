#include <function.h>
#include <iostream>
#include <fft.h>
#include <chrono>

int main() {
    Vec<cplx128_t> data{16777216};
    for (size_t i = 0; i < data.size(); i++) {
        data.rdata()[i] = .001 * i;
        data.idata()[i] = .001 * i;
    } 

    FFT<double> fft(data.size());
    auto view = tview::view(data);

    auto t0 = std::chrono::system_clock::now();
    for (int i = 0; i < 10; i++) {
        fft.fft(view);
    }
    auto t1 = std::chrono::system_clock::now();
    auto dt = t1 - t0;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() << " ms\n";

}
