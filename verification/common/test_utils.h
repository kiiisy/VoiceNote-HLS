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
std::vector<float> load_csv(const std::string &path);
void               write_csv(const std::string &path, const std::vector<float> &v);

// RMSE計算
double rmse(const std::vector<float> &a, const std::vector<float> &b);

}  // namespace testutil
