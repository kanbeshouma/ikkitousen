#pragma once
#include"BaseEnemy.h"
#include"BasePlayer.h"
#include"BossRushUnit.h"
#include "Common.h"
#include"EnemiesEditor.h"
#include"EnemyFileSystem.h"
#include "LastBoss.h"
#include"MasterEnemyDataAdmin.h"
#include"NetWorkInformationStucture.h"
#include"practical_entities.h"
#include<vector>

//****************************************************************
//
// 敵の管理クラス
//
//****************************************************************
class EnemyManager final :public PracticalEntities
{
    //****************************************************************
    //
    // 関数
    //
    //****************************************************************
public:
    EnemyManager();
    ~EnemyManager() override;

    void fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_);

    //-----リトライ時再初期化-----//
    void RestartInitialize();

    //-----通常時時に呼ぶ更新処理-----//
    void fUpdate(GraphicsPipeline & graphics_,float elapsedTime_, AddBulletFunc Func_);

    //-----マルチプレイ時のホストが呼ぶ更新処理-----//
    void fHostUpdate(GraphicsPipeline& graphics_, float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data);

    //-----マルチプレイ時にクライアント側が呼ぶ更新処理-----//
    void fClientUpdate(GraphicsPipeline & graphics_,float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data);

    //-----ダメージデータを設定する-----//
    void SetReciveDamageData(int obj_id, int damage, GraphicsPipeline& graphics_);

    //-----敵の状態データの設定-----//
    void fSetReceiveConditionData(EnemySendData::EnemyConditionData data);


    void fRender(GraphicsPipeline& graphics_);

    void fFinalize();

    bool fGetSlow()const;
private:
    //-----敵のデータを送信する-----//
    void fCheckSendEnemyData(float elapsedTime_);

    //-----敵のデータを送信-----//
    void fSendEnemyData(float elapsedTime_);

    //-----敵のダメージデータを送信-----//
    void fSendEnemyDamage(int obj_id,int damage);

    //-----受信した敵のデータを設定-----//
    void fSetReceiveEnemyData(float elapsedTime_, EnemySendData::EnemyData data, GraphicsPipeline& graphics_);

    //<ボスの再出現>//
    void BossExistence(GraphicsPipeline& graphics_);
public:
    //--------------------<当たり判定>--------------------//

    // プレイヤーの攻撃と敵の当たり判定
    int fCalcPlayerAttackVsEnemies(           // 戻り値 ： 当たった敵の数
        DirectX::XMFLOAT3 PlayerCapsulePointA_,// プレイヤーのカプセルの情報
        DirectX::XMFLOAT3 PlayerCapsulePointB_,// プレイヤーのカプセルの情報
        float PlayerCapsuleRadius_,            // プレイヤーのカプセルの情報
        int PlayerAttackPower_   ,              // プレイヤーの攻撃力
        GraphicsPipeline& Graphics_,
        float elapsedTime_,
        bool& is_shield
    );
    // 敵の攻撃とプレイヤーの当たり判定
    bool fCalcEnemiesAttackVsPlayer(
        DirectX::XMFLOAT3 PlayerCapsulePointA_,// プレイヤーのカプセルの情報
        DirectX::XMFLOAT3 PlayerCapsulePointB_,// プレイヤーのカプセルの情報
        float PlayerCapsuleRadius_,             // プレイヤーのカプセルの情報
        AddDamageFunc Func_
    );


    // 敵の攻撃とプレイヤーのカウンターの当たり判定
    bool fCalcEnemiesAttackVsPlayerCounter(
        DirectX::XMFLOAT3 PlayerCapsulePointA_,// プレイヤーのカプセルの情報
        DirectX::XMFLOAT3 PlayerCapsulePointB_,// プレイヤーのカプセルの情報
        float PlayerCapsuleRadius_);             // プレイヤーのカプセルの情報)

    // プレイヤーの回り込み攻撃と敵の当たり判定
    void fCalcPlayerStunVsEnemyBody(const DirectX::XMFLOAT3 PlayerPosition_, float Radius_);

    // スタン中の敵のうちプレイヤーにもっとも近いものを返す（いなければnull）
    BaseEnemy* fGetNearestStunEnemy();


// 敵を追加する
    void fAddRushBoss(BossRushUnit* enemy);
public:
    //--------------------<ゲッター関数>--------------------//
    [[nodiscard]]  BaseEnemy* fGetNearestEnemyPosition();
    [[nodiscard]]  BaseEnemy* fGetSecondEnemyPosition();
    [[nodiscard]] bool fGetClearWave() const;
    //--------------------<セッター関数>--------------------//
    void fSetIsPlayerChainTime(bool IsChain_);//プレイヤーがチェイン状態であるかどうかをエネミーが受け取る関数

    //-----一番近いプレイヤーの位置を設定する-----//
    void fSetPlayerPosition(std::vector<std::tuple<int, DirectX::XMFLOAT3>> Position_);

    //-----ターゲットIDからプレイヤーの位置を取得する-----//
    void fSetPlayerIdPosition(const std::vector<std::shared_ptr<BasePlayer>>& players);

    void fSetPlayerSearch(bool Arg_); // プレイヤーがチェイン攻撃中にTrue
    void fSetIsTutorial(bool Arg_);
public:
    //--------------------<ImGui>--------------------//
    void fGuiMenu(GraphicsPipeline& Graphics_, AddBulletFunc Func_);
    //--------------------<ウェーブ切り替え関数>--------------------//
    void fStartWave(int WaveIndex_);
    //--------------------<ウェーブクリアしたかどうかを判定する>--------------------//
    [[nodiscard]] bool fWaveClear() const;

    //--------------------<敵の実体を削除する関数>--------------------//
    void fDeleteEnemies();

    [[nodiscard]] std::vector<BaseEnemy*> fGetEnemies() const;
    size_t fGetEnemyCounts()const;

    //--------------------<キャッシュにモデルデータを登録>--------------------//
    void fRegisterCash(GraphicsPipeline& graphics_);
    void fDeleteCash(); // キャッシュを削除

    // ボスから呼び出す
    void fReserveBossUnit(std::vector<DirectX::XMFLOAT3> Vec_);
    void fCreateRandomMasterEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_,int grope_id);
    void fCreateRandomEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_,int grope_id,int transfer_id);


     // チュートリアルで呼び出す関数
    void fSpawnTutorial_NoAttack(float elapsedTime_, GraphicsPipeline& Graphics_);
    void fSpawnTutorial(float elapsedTime_, GraphicsPipeline& Graphics_);

    //--------------------<ボス関連の関数>--------------------//
    [[nodiscard]] LastBoss::Mode fGetBossMode()const;
    void fSetBossMode(LastBoss::Mode Mode_);
    [[nodiscard]] bool fGetIsEventCamera()const;
    [[nodiscard]] DirectX::XMFLOAT3 fGetEye()const;
    [[nodiscard]] DirectX::XMFLOAT3 fGetFocus()const;
    void fSetBossEye(DirectX::XMFLOAT3 Eye_);
    void fSetBossFocus(DirectX::XMFLOAT3 Focus_);

    bool fGetBossClear()const;

    void fReserveDeleteEnemies();

    void fLimitEnemies();

    //-----イベントを見終わったプレイヤーをカウントする
    //========================================
    //第一引数 : 見終わった数
    void EndEnventCount(int count);

    //-----イベントを終わらせる-----//
    //=======================
    void EndEvent();
private:
    //--------------------<敵と関連する処理>--------------------//

    void fSpawn(GraphicsPipeline& graphics); // 敵の生成を管理
    void fSpawn(EnemySource Source_, GraphicsPipeline& graphics_);

    //-----マルチプレイ時のクライアント側の敵の出現-----//
    void fSpawn(EnemySendData::EnemySpawnData data, GraphicsPipeline& graphics_);

    void fEnemiesUpdate(GraphicsPipeline& Graphics_,float elapsedTime_); // 敵の更新処理
    void fEnemiesRender(GraphicsPipeline& graphics_); // 敵の描画処理

    void fCreateBossUnit(GraphicsPipeline& Graphics_);

    //-----敵の出現データを送信-----//
    void fSendSpawnData(EnemySource Source_);


    //--------------------<敵をソートする>--------------------//
    void fSort(std::function<bool(const BaseEnemy* A_, const BaseEnemy* B_)> Function_);

    //--------------------<管理クラス内で完結する処理>--------------------//
    void fAllClear(); // 敵を全削除する関数

    //--------------------<敵同士の当たり判定>--------------------//
    void fCollisionEnemyVsEnemy();

    void fLoad(const char* FileName_);
    void fSave(const char* FileName_);

    //-----敵のグループのホストのデータを設定する-----//
    void SetEnemyGropeHostData();

    //-----マスターを譲渡する-----//
    //======================
    //第一引数 : 譲渡対象のグループ-----//
    void TransferMaster(int grope);

    //****************************************************************
    //
    // 変数
    //
    //****************************************************************
private:
    std::vector<BaseEnemy*> mEnemyVec;  // 敵を格納するコンテナ
    std::vector<BaseEnemy*> mRemoveVec; // 敵を格納するコンテナ
    // リソースマネージャーが情報を保持できるようにキャッシュとしてモデルデーを登録する
    std::vector<BaseEnemy*> mCashEnemyVec{};

    //-----敵のマスターデータを入れるクラス------//
    std::unique_ptr<MasterEnemyDataAdmin> master_enemy_data;

    //--------------------<ウェーブに関する変数>--------------------//
    float mWaveTimer{}; // 各ウェーブ開始からの経過時間
    int mCurrentWave{}; // 現在のウェーブ
    // 現在のウェーブのデータ
    std::vector<EnemySource> mCurrentWaveVec{};
    std::vector<EnemySource> mFirstTest{};
    float mLastTimer{};// 最後に出てくる敵の時間
    EnemyEditor mEditor{};

    bool mDebugMode{};
    bool mIsPlayerChainTime = false;
    //--------------------<プレイヤーの位置>--------------------//
    std::vector<DirectX::XMFLOAT3> mPlayerPosition{};

    int mUniqueCount{};

    // カメラシェイク
    float mCameraShakeTime{};
    const float mkOneShakeSec{ 0.15f };

    // チュートリアル
    float mTutorialTimer{};
    const float mkSeparateTutorial{ 1.0f };

    bool mIsReserveDelete{};

    // ボスがユニットを召喚
    bool mIsReserveBossUnit{};
    std::vector<DirectX::XMFLOAT3> mUnitEntryPointVec{};
    std::vector<EnemySource> mReserveVec{};
    //--------------------<ボス関連の変数>--------------------//
    LastBoss::Mode mCurrentMode{ LastBoss::Mode::None };
    //bool mIsBossEvent{};// イベント中
    DirectX::XMFLOAT3 mBossCameraEye{ 0.0f,0.0f,-80.0f };
    DirectX::XMFLOAT3 mBossCameraFocus{0.0f,0.0f,0.0f};

    bool start_boss_rush{ false };
public:
    void SetStartBossRush(bool arg) { start_boss_rush = arg; }
private:
    // チュートリアルの使用
    bool mIsTutorial{ false };

    float mDelay{};

    float mSloeTime{};

    //-----スポーン番号-----//
    int object_count{};

    //-----ミリ秒-----//
    float milliseconds{};

    //-----イベントを終了した人数-----//
    int end_event_count{ 0 };

    //<ボスの存在確認のデータ送信タイマー>//
    float check_boss_existence_timer{ 0.0f };

    //<ボスの存在確認のデータ送信フラグ>//
    bool check_boss_existence_flg{ false };

    //****************************************************************
    //
    // 定数
    //
    //****************************************************************

    // 敵同士の間隔を調整する
    const float mAdjustmentEnemies{ 1.0f };

    const char* mWaveFileNameArray[7]
    {
        "./resources/Data/Wave1_1.json",
        "./resources/Data/Wave2_1.json",
        "./resources/Data/Wave2_2.json",
        "./resources/Data/Wave3_1.json",
        "./resources/Data/Wave3_2.json",
        "./resources/Data/Wave3_3.json",
        "./resources/Data/WaveBoss.json",
    };

    // ImGui
    bool mOpenGuiMenu{};

    //-----敵の位置データを送信する時のコマンドのサイズ-----//
    const int SendEnemyDataComSize = 4;

    //-----データの送信頻度(ミリ秒)-----//
    const float EnemyDataFrequency = 300.0f;

    //-----敵がダメージを受けたときの無敵時間-----//
    const float EnemyInvincibleTime = 0.1f;

    //<ボスの出現チェックのデータ送信頻度>//
    const float CheckBossExistenceTime = 1.5f;

};
