#include "../../src/DcCut/dc_cut.h"
#include "../common/axis_helpers.h"
#include "../common/test_runner.h"

using namespace testutil;

bool run_dc_cut_case(const std::string &case_name, double &rmse_out)
{
    return run_case_generic(
        "DcCut", case_name,
        [&](audio::axis_stream_t &s, audio::axis_stream_t &m)
        {
            static bool pass = false;
            static float acoef = std::exp(-2.0f * float(M_PI) * dc_cut_cutoff_hz() / sample_rate());
            dc_cut(s, m, acoef, pass);
        },
        rmse_out);
}

#ifdef HLS_TB_STANDALONE // HLSプロジェクトのCFLAGSで定義

int main()
{
    try
    {
        struct CaseInfo
        {
            uint16_t index; // 使わなくていいかも
            const char *name;
        };

        // GUIで実行する場合はここを編集して
        // 内部の処理の都合、まとめて実行すると不一致が発生する。
        // 見たいテストケースを一つづつコメントアウトで確認する。ダサいが一旦これで。
        const CaseInfo cases[] = {
            {1, "dc_cut_case1"},
            //{2, "dc_cut_case2"},
            //{3, "dc_cut_case3"},
            //{4, "dc_cut_case4"},
            //{5, "dc_cut_case5"},
        };

        bool all_ok = true;
        for (auto &c : cases)
        {
            double rmse = 0.0;
            bool ok = run_dc_cut_case(c.name, rmse);
            if (!ok)
            {
                std::cerr << "[NG] " << c.name << " RMSE=" << rmse << "\n";
                all_ok = false;
            }
            else
            {
                std::cout << "[OK] " << c.name << " RMSE=" << rmse << "\n";
            }
        }

        return all_ok ? 0 : 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[EXCEPTION] " << e.what() << "\n";
        return 1;
    }
}

#endif // HLS_TB_STANDALONE

#ifdef USE_GTEST // CMakeでunit_testsターゲットで定義

#include <gtest/gtest.h>

static void CheckCase(uint16_t idx)
{
    double rmse = 0.0;
    std::string name = "dc_cut_case" + std::to_string(idx);

    bool ok = run_dc_cut_case(name, rmse);

    std::cout << "[INFO] " << name << " RMSE=" << rmse << std::endl;

    EXPECT_TRUE(ok) << "RMSE=" << rmse;
}

TEST(DcCut, Case1) { CheckCase(1); }
TEST(DcCut, Case2) { CheckCase(2); }
TEST(DcCut, Case3) { CheckCase(3); }
TEST(DcCut, Case4) { CheckCase(4); }
TEST(DcCut, Case5) { CheckCase(5); }

#endif // USE_GTEST
