#pragma once

// 前方宣言
struct MODEL;

#include "btBulletDynamicsCommon.h"
#include "VHACD.h"
#include <vector>
#include <unordered_map>
#include <string>

// V-HACDを使って凸分解を行うクラス（キャッシュ機能付き）
class ConvexDecomposition {
public:
    // OBJモデルから凸包の集合を生成（キャッシュ機能付き）
    static btCompoundShape* CreateConvexDecomposition(
        MODEL* model,
        const std::string& cacheKey,           // キャッシュキー（モデル名など）
        unsigned int maxConvexHulls = 16,      // 最大凸包数（デフォルト16に変更）
        unsigned int resolution = 20000,        // ボクセル解像度（デフォルト20000に変更）
        double concavity = 0.001,
        double alpha = 0.05,
        double beta = 0.05
    );

    // キャッシュをクリア
    static void ClearCache();

    // キャッシュサイズ取得
    static size_t GetCacheSize() { return s_Cache.size(); }

private:
    // 凸分解の実際の計算
    static btCompoundShape* ComputeConvexDecomposition(
        MODEL* model,
        unsigned int maxConvexHulls,
        unsigned int resolution
    );

    // キャッシュ
    static std::unordered_map<std::string, btCompoundShape*> s_Cache;

    // V-HACDのコールバック
    class ProgressCallback : public VHACD::IVHACD::IUserCallback {
    public:
        void Update(const double overallProgress,
            const double stageProgress,
            const char* const stage,
            const char* const operation) override {
            // 進捗を表示
            if ((int)(overallProgress * 100) % 10 == 0) {
                printf("[V-HACD] %s: %.0f%%\n", stage, overallProgress * 100.0);
            }
        }
    };
};