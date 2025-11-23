#include "test_utils.h"

#include <cmath>
#include <fstream>
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

std::vector<float> load_csv(const std::string &path)
{
    std::vector<float> v;
    std::ifstream      ifs(path.c_str());
    if (!ifs) {
        throw std::runtime_error("Failed to open CSV: " + path);
    }

    float x;
    while (ifs >> x) {
        v.push_back(x);
    }

    return v;
}

void write_csv(const std::string &path, const std::vector<float> &v)
{
    // HLS 環境だとディレクトリ作成が面倒なので、
    // ひとまず「パスが存在している前提」でファイルだけ開く実装にしておく。
    std::ofstream ofs(path.c_str());
    if (!ofs) {
        throw std::runtime_error("Failed to write CSV: " + path);
    }

    ofs.setf(std::ios::fixed);
    ofs.precision(10);

    for (float x : v) {
        ofs << x << "\n";
    }
}

double rmse(const std::vector<float> &a, const std::vector<float> &b)
{
    if (a.size() != b.size()) {
        throw std::runtime_error("rmse: size mismatch");
    }
    if (a.empty()) {
        return 0.0;
    }

    double mse = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        mse += d * d;
    }

    mse /= static_cast<double>(a.size());

    return std::sqrt(mse);
}

}  // namespace testutil
