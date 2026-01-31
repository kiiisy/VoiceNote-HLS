#pragma once
#include "param.h"
#include <cstddef>
#include <string>
#include <vector>

namespace testutil {

// 共通パラメータ
inline double sample_rate()
{
    return kFs;
}
inline double duration_sec()
{
    return 1.0;
}

// Dcカットパラメータ
inline double dc_cut_cutoff_hz()
{
    return kFc;
}

inline std::size_t num_samples()
{
    return static_cast<std::size_t>(sample_rate() * duration_sec());
}

// プロジェクトルートを文字列パスで返す
std::string project_root();

// tests/<Module>/golden/foo.csv
std::string golden_path(const std::string &module, const std::string &filename);

// tests/<Module>/output/foo_cpp.csv
std::string output_path(const std::string &module, const std::string &filename);

// CSV I/O
std::vector<int16_t> load_csv(const std::string &path);
void                 write_csv(const std::string &path, const std::vector<int16_t> &v);

// RMSE計算
double rmse(const std::vector<int16_t> &a, const std::vector<int16_t> &b);

// 整数の最大差分
int32_t max_abs_diff(const std::vector<int16_t> &a, const std::vector<int16_t> &b);

}  // namespace testutil
