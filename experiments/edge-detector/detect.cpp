// OrcPluto off-target CS16 energy and DFT detector. NOT a live radio driver or modulation classifier.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 5) {
    std::cerr << "usage: edge_detect <iq_cs16le_file> <window_samples> <power_threshold_counts2> <events.ndjson>\n";
    return 2;
  }
  const std::string input = argv[1], output = argv[4];
  if (input == output) { std::cerr << "input and output must differ\n"; return 2; }
  unsigned long n_arg = 0;
  double threshold = 0;
  try { n_arg = std::stoul(argv[2]); threshold = std::stod(argv[3]); }
  catch (...) { std::cerr << "invalid numeric argument\n"; return 2; }
  if (n_arg < 16 || n_arg > 4096 || !(threshold > 0.0) || !std::isfinite(threshold)) {
    std::cerr << "invalid window/threshold (window 16..4096, threshold >0)\n"; return 2;
  }
  const size_t n = static_cast<size_t>(n_arg);
  std::ifstream src(input, std::ios::binary);
  if (!src) { std::cerr << "cannot read input\n"; return 3; }
  std::ofstream dst(output, std::ios::trunc);
  if (!dst) { std::cerr << "cannot write output\n"; return 3; }
  std::vector<uint8_t> raw(4*n);
  std::vector<double> re(n), im(n);
  uint64_t first=0, events=0;
  const double pi = std::acos(-1.0);
  while (src.read(reinterpret_cast<char*>(raw.data()), static_cast<std::streamsize>(raw.size()))) {
    double sum_power=0.0;
    for (size_t i=0;i<n;i++) {
      const auto at=[&](size_t off)->int16_t { return static_cast<int16_t>(uint16_t(raw[off]) | (uint16_t(raw[off+1])<<8)); };
      re[i]=at(i*4); im[i]=at(i*4+2);
      sum_power+=re[i]*re[i]+im[i]*im[i];
    }
    const double mean_power=sum_power/double(n);
    // Educational O(N^2) DFT. Analyze a bounded window; not a production high-rate FFT.
    double peak=0.0;
    size_t peak_bin=0;
    if (mean_power > threshold) {
      for (size_t k=0;k<n;k++) {
        double xr=0.0, xi=0.0;
        for(size_t j=0;j<n;j++) {
          const double angle=-2.0*pi*double(k)*double(j)/double(n);
          const double c=std::cos(angle), s=std::sin(angle);
          xr+=re[j]*c-im[j]*s;
          xi+=re[j]*s+im[j]*c;
        }
        const double power=(xr*xr+xi*xi)/(double(n)*double(n));
        if(power>peak){peak=power;peak_bin=k;}
      }
      dst << std::setprecision(12)
          << "{\"schema\":\"orcp.internal.measurement.v0\",\"source\":\"offline_cs16le_replay\","
          << "\"source_epoch\":\"offline_file_0001\",\"sample_index_domain\":\"file_relative_not_device_time\",\"first_sample_index\":" << first
          << ",\"sample_count\":" << n << ",\"rx_channel\":0,"
          << "\"measured\":{\"mean_iq_power_counts2\":" << mean_power
          << ",\"peak_dft_power_counts2\":" << peak << ",\"peak_dft_bin\":" << peak_bin
          << "},\"classification\":{\"status\":\"unknown\",\"confidence\":null},"
          << "\"signal_present\":true,\"iq_gaps_known\":false}\n";
      ++events;
    }
    first+=n;
  }
  if (src.gcount() != 0) {
    std::cerr << "error: truncated final IQ window (not silently discarded)\n";
    return 4;
  }
  if (!src.eof() || !dst.good()) { std::cerr << "I/O error\n"; return 5; }
  std::cout << "windows=" << (first/n) << " events=" << events << " samples=" << first << "\n";
  return 0;
}
