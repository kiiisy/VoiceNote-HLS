#include "test_utils.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "../../../../../../"
#endif

namespace testutil {

// シンプルに文字列でパス管理
std::string project_root()
{
    return std::string(PROJECT_SOURCE_DIR);
}

static std::string join_path(const std::string &a, const std::string &b)
{
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/' || a.back() == '\\') {
        return a + b;
    }

    return a + "/" + b;
}

std::string golden_path(const std::string &module, const std::string &filename)
{
    std::string p = project_root();
    p             = join_path(p, "verification");
    p             = join_path(p, module);
    p             = join_path(p, "golden");
    p             = join_path(p, filename);

    return p;
}

std::string output_path(const std::string &module, const std::string &filename)
{
    std::string p = project_root();
    p             = join_path(p, "verification");
    p             = join_path(p, module);
    p             = join_path(p, "output");
    p             = join_path(p, filename);

    return p;
}

std::vector<int16_t> load_csv(const std::string &path)
{
    std::vector<int16_t> v;
    std::ifstream        ifs(path.c_str());
    if (!ifs) {
        throw std::runtime_error("Failed to open CSV: " + path);
    }

    int16_t x;
    while (ifs >> x) {
        v.push_back(x);
    }

    return v;
}

void write_csv(const std::string &path, const std::vector<int16_t> &v)
{
    // HLS 環境だとディレクトリ作成が面倒なので、
    // ひとまず「パスが存在している前提」でファイルだけ開く実装にしておく。
    std::ofstream ofs(path.c_str());
    if (!ofs) {
        throw std::runtime_error("Failed to write CSV: " + path);
    }

    ofs.setf(std::ios::fixed);
    ofs.precision(10);

    for (int16_t x : v) {
        ofs << x << "\n";
    }
}

double rmse(const std::vector<int16_t> &a, const std::vector<int16_t> &b)
{
    if (a.size() != b.size()) {
        throw std::runtime_error("rmse: size mismatch");
    }
    if (a.empty()) {
        return 0.0;
    }

    double mse = 0.0;

    for (std::size_t i = 0; i < a.size(); ++i) {
        // Q15 → float [-1,1)
        double af = static_cast<double>(a[i]) / 32768.0;
        double bf = static_cast<double>(b[i]) / 32768.0;

        double d = af - bf;
        mse += d * d;
    }

    mse /= static_cast<double>(a.size());

    return std::sqrt(mse);
}

int32_t max_abs_diff(const std::vector<int16_t> &a, const std::vector<int16_t> &b)
{
    if (a.size() != b.size()) {
        throw std::runtime_error("max_abs_diff: size mismatch");
    }

    int32_t maxd = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        int32_t d = std::abs(int(a[i]) - int(b[i]));
        if (d > maxd) {
            //std::cout << a[i] << b[i] << std::endl;
            maxd = d;
        }
    }

    return maxd;
}

}  // namespace testutil
