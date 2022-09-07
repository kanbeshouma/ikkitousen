#include"EnemyManager.h"
#include "EnemyFileSystem.h"

#include"TestEnemy.h"
#include"NormalEnemy.h"
#include"ChaseEnemy.h"
#include"ArcherEnemy.h"
#include"ArcherEnemy_Ace.h"
#include"FrontShieldEnemy.h"
#include"ShieldEnemy_Ace.h"
#include"MiddleBoss.h"
#include"SwordEnemy.h"
#include"SpearEnemy.h"
#include"SpearEnemy_Ace.h"
#include"LastBoss.h"
#include"TutorialEnemy.h"
#include"SwordEnemyAce.h"
#include"BossUnit.h"
#include "game_icon.h"

#include"imgui_include.h"
#include "user.h"
#include"collision.h"
#include"Operators.h"


#include <fstream>

//****************************************************************
//
// 敵の管理クラス
//
//****************************************************************

EnemyManager::~EnemyManager()
{
}

void EnemyManager::fInitialize(GraphicsPipeline& graphics_, AddBulletFunc Func_)
{
    //--------------------<初期化>--------------------//
    fAllClear();
    mUniqueCount = 0;

    // キャッシュに登録
    fRegisterCash(graphics_);

    // チュートリアルかどうかを初期化
    mIsTutorial = false;
}

void EnemyManager::fUpdate(GraphicsPipeline& graphics_, float elapsedTime_,AddBulletFunc Func_)
{
    //--------------------<管理クラス自体の更新処理>--------------------//

    mSloeTime -= elapsedTime_;

    // ウェーブ開始からの時間を更新
    mDelay-=elapsedTime_;
    if (!mDebugMode && !mIsTutorial && mDelay <= 0.0f)
    {
        //--------------------<敵がたまりすぎたら>--------------------//
        if (mEnemyVec.size() < 30)
        {
            mWaveTimer += elapsedTime_;
        }
    }

    // カメラシェイク
    if(mCameraShakeTime>0.0f)
    {
        if (GameFile::get_instance().get_shake()) camera_shake->shake(graphics_, elapsedTime_);
    }
    mCameraShakeTime -= elapsedTime_;
    mCameraShakeTime = (std::max)(0.0f, mCameraShakeTime);

    //--------------<プレイヤーがチェイン中はエネミーの行動をすべて停止させる>-------------//
    if(mIsPlayerChainTime)
    {
        return;
    }

    //--------------------<敵の更新処理>--------------------//
    fEnemiesUpdate(graphics_,elapsedTime_);
    fDeleteEnemies();
    //--------------------<敵同士の当たり判定>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<敵のスポナー>--------------------//
    fSpawn(graphics_);
    // ImGuiのメニュー
    fGuiMenu(graphics_,Func_);

    //--------------------<ボスが敵を召喚する>--------------------//
    fCreateBossUnit(graphics_);

    bool isCreate{};
    for (const auto& source : mReserveVec)
    {
        isCreate = true;
        fSpawn(source, graphics_);
    }
    if(isCreate)
    {
        mReserveVec.clear();
    }

    // ザコ的だけを全消しする
    if(mIsReserveDelete)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetIsBoss() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }
}

void EnemyManager::fRender(GraphicsPipeline& graphics_)
{
    //--------------------<敵の描画処理>--------------------//
    fEnemiesRender(graphics_);
}

void EnemyManager::fFinalize()
{
    fAllClear();
    fDeleteCash();
}

bool EnemyManager::fGetSlow() const
{
    return mSloeTime > 0.0f;
}

int EnemyManager::fCalcPlayerAttackVsEnemies(DirectX::XMFLOAT3 PlayerCapsulePointA_,
                                             DirectX::XMFLOAT3 PlayerCapsulePointB_, float PlayerCapsuleRadius_, int PlayerAttackPower_,
                                             GraphicsPipeline& Graphics_,float elapsedTime_,bool& is_shield)
{
    //--------------------<プレイヤーの攻撃と敵の当たり判定>--------------------//
      // 攻撃が何体の敵に当たったか
    int  hitCounts = 0;

    for (const auto enemy : mEnemyVec)
    {
        // 当たり判定をするか確認
        if (enemy->fComputeAndGetIntoCamera())
        {
            Capsule capsule = enemy->fGetBodyCapsule();

            const bool result = Collision::capsule_vs_capsule(
                PlayerCapsulePointA_, PlayerCapsulePointB_, PlayerCapsuleRadius_,
                capsule.mTop, capsule.mBottom, capsule.mRadius);

            // 当たっていたら
            if (result)
            {
                if(enemy->fDamaged(PlayerAttackPower_, 0.1f,Graphics_,elapsedTime_))
                {
                    audio_manager->play_se(SE_INDEX::ATTACK_SWORD);

                    //攻撃を防がれたら即リターン
                    hitCounts++;
                    mSloeTime = 0.1f;
                }
                else
                {
                    if (enemy->mIsSuccesGuard)
                    {
                        is_shield = true;
                        return hitCounts;
                    }
                }
            }
        }
    }

    return hitCounts;
}

bool EnemyManager::fCalcEnemiesAttackVsPlayer(DirectX::XMFLOAT3 PlayerCapsulePointA_,
    DirectX::XMFLOAT3 PlayerCapsulePointB_, float PlayerCapsuleRadius_, AddDamageFunc Func_)
{
    //--------------------<プレイヤーと敵の攻撃の当たり判定>--------------------//

    for (const auto enemy : mEnemyVec)
    {
        // 当たり判定をするか確認
        if (enemy->fComputeAndGetIntoCamera())
        {
            if (enemy->fGetAttack())
            {
                Capsule capsule = enemy->fGetAttackCapsule();

                const bool result = Collision::capsule_vs_capsule(
                    PlayerCapsulePointA_, PlayerCapsulePointB_, PlayerCapsuleRadius_,
                    capsule.mTop, capsule.mBottom, capsule.mRadius);

                // 当たっていたら
                if (result)
                {
                    Func_(enemy->fGetAttackPower(), enemy->fGetAttackInvTime());
                    return  true;
                }
            }
        }
    }
    return false;
}

bool EnemyManager::fCalcEnemiesAttackVsPlayerCounter(DirectX::XMFLOAT3 PlayerCapsulePointA_,
    DirectX::XMFLOAT3 PlayerCapsulePointB_, float PlayerCapsuleRadius_)
{
    //--------------------<プレイヤーと敵の攻撃の当たり判定>--------------------//

    for (const auto enemy : mEnemyVec)
    {
        // 当たり判定をするか確認
        if (enemy->fComputeAndGetIntoCamera())
        {
            if (enemy->fGetAttack())
            {
                Capsule capsule = enemy->fGetAttackCapsule();

                const bool result = Collision::capsule_vs_capsule(
                    PlayerCapsulePointA_, PlayerCapsulePointB_, PlayerCapsuleRadius_,
                    capsule.mTop, capsule.mBottom, capsule.mRadius);

                // 当たっていたら
                if (result)
                {
                    return  true;
                }
            }
        }
    }
    return false;
}

void EnemyManager::fCalcPlayerStunVsEnemyBody(const DirectX::XMFLOAT3 PlayerPosition_, float Radius_)
{

    if(Radius_<=0.0f)
    {
        return;
    }
    // 総当たりでプレイヤーからスタンこうげきを受ける
    for(const auto & enemy: mEnemyVec)
    {
        const DirectX::XMFLOAT3 enemyPosition = enemy->fGetPosition();
        const float enemyRad = enemy->fGetBodyCapsule().mRadius;
        if(Math::Length(mPlayerPosition-enemyPosition) <= Radius_+enemyRad)
        {
            enemy->fSetStun(true);
        }
    }
}

BaseEnemy* EnemyManager::fGetNearestStunEnemy()
{
    BaseEnemy* pEnemy{ nullptr };
    float lengthVsPlayer{ FLT_MAX };
    for(int i=0;i<mEnemyVec.size();++i)
    {
        if(mEnemyVec[i]->fGetStun())
        {
            if(lengthVsPlayer>mEnemyVec[i]->fGetLengthFromPlayer())
            {
                lengthVsPlayer = mEnemyVec[i]->fGetLengthFromPlayer();
                pEnemy = mEnemyVec[i];
            }
        }
    }
    return pEnemy;

}

void EnemyManager::fAddRushBoss(BossRushUnit* enemy)
{
    mEnemyVec.emplace_back(enemy);
}

BaseEnemy* EnemyManager::fGetNearestEnemyPosition()
{
    auto func = [](const BaseEnemy* A_, const BaseEnemy* B_)->bool
    {
        return A_->fGetLengthFromPlayer() < B_->fGetLengthFromPlayer();
    };
    fSort(func);
    for(const auto enemy :mEnemyVec)
    {
        if (enemy->fGetStun()) continue;;


        if (enemy->fComputeAndGetIntoCamera())
        {
            // この敵からの距離を計算する
            for(const auto enemy2:mEnemyVec)
            {
                if (enemy2->fComputeAndGetIntoCamera())
                {
                    if (enemy != enemy2)
                    {
                        //enemy2->fCalcNearestEnemy(enemy->fGetPosition());
                    }
                }
            }
            return enemy;
        }
    }

    return nullptr;
}

BaseEnemy* EnemyManager::fGetSecondEnemyPosition()
{
    auto func = [](const BaseEnemy* A_, const BaseEnemy* B_)->bool
    {
        return A_->fGetLengthFromPlayer() < B_->fGetLengthFromPlayer();
    };
    fSort(func);
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fComputeAndGetIntoCamera())
        {
            // この敵からの距離を計算する
            for (const auto enemy2 : mEnemyVec)
            {
                if (enemy2->fComputeAndGetIntoCamera())
                {
                    if (enemy != enemy2)
                    {
                        //enemy2->fCalcNearestEnemy(enemy->fGetPosition());
                    }
                }
            }
            return enemy;
        }
    }

    return nullptr;
}

bool EnemyManager::fGetClearWave() const
{
    // 残りデータが０かつフィールドに敵が残っていない
    return (mCurrentWaveVec.size() <= 0 && mEnemyVec.size() <= 0);
}



void EnemyManager::fSetIsPlayerChainTime(bool IsChain_)
{
    mIsPlayerChainTime = IsChain_;
}

void EnemyManager::fSetPlayerPosition(DirectX::XMFLOAT3 Position_)
{
    mPlayerPosition = Position_;
    for(const auto& enemy:mEnemyVec)
    {
        enemy->fSetPlayerPosition(Position_);
    }
}

void EnemyManager::fSetPlayerSearch(bool Arg_)
{
    for (const auto& enemy : mEnemyVec)
    {
        enemy->fSetIsPlayerSearch(Arg_);
    }
}

void EnemyManager::fSetIsTutorial(bool Arg_)
{
    mIsTutorial = Arg_;
}

void EnemyManager::fSpawn(GraphicsPipeline& graphics)
{
    int spawnCounts = 0;

    // 敵をスポーンする関数
    for (const auto data : mCurrentWaveVec)
    {
        // 出現条件を満たしていたら出す
        if (data.mSpawnTimer <= mWaveTimer)
        {
            fSpawn(data, graphics);
            spawnCounts++;
        }
    }
    // 追加したら先頭のデータを消す
    for (int i = 0; i < spawnCounts; i++)
    {
        mCurrentWaveVec.erase(mCurrentWaveVec.begin());
    }
}

void EnemyManager::fSpawn(EnemySource Source_, GraphicsPipeline& graphics_)
{
    // 送られてきたデータをもとに敵を出現させる
    const auto param = mEditor.fGetParam(Source_.mType);
    switch (Source_.mType)
    {
    case EnemyType::Archer:
    {
        BaseEnemy* enemy = new ArcherEnemy(graphics_,
            Source_.mEmitterPoint, param);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield:
    {
        BaseEnemy* enemy = new ShieldEnemy(graphics_,
            Source_.mEmitterPoint, param);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword:
    {
        BaseEnemy* enemy = new SwordEnemy(graphics_,
            Source_.mEmitterPoint, param);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear:
    {
        BaseEnemy* enemy = new SpearEnemy(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        BaseEnemy* enemy = new ArcherEnemy_Ace(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Shield_Ace:
    {
        BaseEnemy* enemy = new ShieldEnemy_Ace(graphics_,
            Source_.mEmitterPoint, param);
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Sword_Ace:
    {
        BaseEnemy* enemy = new SwordEnemy_Ace(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        BaseEnemy* enemy = new SpearEnemy_Ace(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType));
        mEnemyVec.emplace_back(enemy);
    }
        break;
    case EnemyType::Boss:
        {
        BaseEnemy* enemy = new LastBoss(graphics_,
            Source_.mEmitterPoint,
            mEditor.fGetParam(Source_.mType),this);
        mEnemyVec.emplace_back(enemy);
        }
        break;
    case EnemyType::Count: break;
case EnemyType::Tutorial_NoMove:
{
    BaseEnemy* enemy = new TutorialEnemy_NoAttack(graphics_,
        Source_.mEmitterPoint,
        mEditor.fGetParam(Source_.mType));
    mEnemyVec.emplace_back(enemy);
}
    break;
case EnemyType::Boss_Unit:
    break;
    default:;
    }


}

void EnemyManager::fEnemiesUpdate(GraphicsPipeline& Graphics_,float elapsedTime_)
{
    // 更新
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fGetIsAlive())
        {
            enemy->fSetPlayerPosition(mPlayerPosition);
            enemy->fUpdate(Graphics_,elapsedTime_);
        }
        else
        {
            mRemoveVec.emplace_back(enemy);
            // 死んでいる敵がいたら振る時間を加算
            mCameraShakeTime += mkOneShakeSec;
        }
    }


}

void EnemyManager::fEnemiesRender(GraphicsPipeline& graphics_)
{
    for (const auto enemy : mEnemyVec)
    {
        enemy->fRender(graphics_);
    }
}

void EnemyManager::fReserveBossUnit(std::vector<DirectX::XMFLOAT3> Vec_)
{
    if(mIsReserveBossUnit==false)
    {
        mIsReserveBossUnit = true;
        mUnitEntryPointVec = Vec_;
    }
}

void EnemyManager::fCreateRandomEnemy(
    GraphicsPipeline& Graphics_,
    DirectX::XMFLOAT3 SeedPosition_)
{
    if(mEnemyVec.size()>30)
    {
        return;
    }

    // 乱数で敵のタイプを取得
    std::mt19937 mt{ std::random_device{}() };
    const std::uniform_int_distribution<int> RandTargetAdd(0, 10);
    int randNumber = RandTargetAdd(mt);

    // チュートリアルの敵なら違う敵を出す
    if (randNumber == 9 || randNumber == 8)
    {
        randNumber = 10;
    }

    EnemySource source;

    const std::uniform_int_distribution<int> RandTargetAdd2(-5, 5);
    const int randPosition = RandTargetAdd2(mt);
    const int randPositionX = RandTargetAdd2(mt);
    const int randPositionY = RandTargetAdd2(mt);
    source.mEmitterPoint =
    {
        SeedPosition_.x+
        (static_cast<float>(randPosition)* static_cast<float>(randPositionX)),
        0.0f,
        SeedPosition_.z+
        static_cast<float>(randPosition)* static_cast<float>(randPositionY),
    };

    source.mType = static_cast<EnemyType>(randNumber);
    mReserveVec.emplace_back(source);
}

void EnemyManager::fSort(std::function<bool(const BaseEnemy* A_, const BaseEnemy* B_)> Function_)
{
    // プレイヤーとの距離順に敵をソート
    std::sort(mEnemyVec.begin(), mEnemyVec.end(), Function_);
}


void EnemyManager::fAllClear()
{
    //--------------------<要素を全削除>--------------------//
    for (const auto enemy : mEnemyVec)
    {
        // 存在していれば削除
        if (enemy)
        {
            delete enemy;
        }
    }
    mEnemyVec.clear();
}

void EnemyManager::fCollisionEnemyVsEnemy()
{
    for(const auto enemy1 :mEnemyVec)
    {
        for (const auto enemy2 : mEnemyVec)
        {
            // 自分自身とは判定しない
            if (enemy1 == enemy2)
            {
                continue;
            }
             auto capsule1 = enemy1->fGetBodyCapsule();
             auto capsule2 = enemy2->fGetBodyCapsule();
            // もし半径が０以下なら計算しない
            if(capsule1.mRadius<=0.0f||capsule2.mRadius<=0.0f)
            {
                continue;
            }

            //--------------------<敵同士の距離感を調整する>--------------------//
            capsule1.mRadius *= mAdjustmentEnemies;
            capsule2.mRadius *= mAdjustmentEnemies;


            const bool result=Collision::capsule_vs_capsule(
                capsule1.mTop, capsule1.mBottom, capsule1.mRadius,
                capsule2.mTop, capsule2.mBottom, capsule2.mRadius);

            // もし当たったら
            if(result)
            {
                // 当たった敵間のベクトルを算出する
                DirectX::XMFLOAT3 vec = enemy2->fGetPosition() - enemy1->fGetPosition();
                if (Math::Length(vec) <= 0.0f)
                {
                    vec = { 0.0f,0.0f,1.0f };
                }
                // 正規化
                vec = Math::Normalize(vec);
                // 二体のめり込んでいる距離を計算する

                // 二体の半径の合計
                const float radiusAdd = capsule1.mRadius + capsule2.mRadius;
                // 二体の距離
                float length = Math::Length(
                    enemy1->fGetPosition() - enemy2->fGetPosition());
                if(length<=0.0f)
                {
                    length = 0.1f;
                }
                // めり込み距離
                const float raidLength = radiusAdd - length;
                DirectX::XMFLOAT3 res = enemy2->fGetPosition() + (vec * raidLength);

            }

        }
    }
}

void EnemyManager::fLoad(const char* FileName_)
{
    // Jsonファイルから値を取得
    std::filesystem::path path = FileName_;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(mCurrentWaveVec);
        }
    }
}

void EnemyManager::fGuiMenu(GraphicsPipeline& Graphics_, AddBulletFunc Func_)
{
    imgui_menu_bar("Game", "EnemyManager", mOpenGuiMenu);


#ifdef USE_IMGUI
    if (mOpenGuiMenu)
    {
        ImGui::Begin("EnemyManager");

        ImGui::Text("WaveNumber");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mCurrentWave).c_str());
        ImGui::Text("WaveTimer");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mWaveTimer).c_str());

        ImGui::Separator();

        ImGui::Text("EnemyValues");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mEnemyVec.size()).c_str());
        ImGui::SameLine();
        ImGui::Text("DataSize");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mCurrentWaveVec.size()).c_str());
        ImGui::Separator();

        ImGui::Separator();
        static int elem = static_cast<int>(EnemyType::Sword);
        constexpr int count = static_cast<int>(EnemyType::Count);
        const char* elems_names[count] =

        {
            "Archer","Shield","Sword","Spear","Archer_Ace",
            "Shield_Ace","Sword_Ace","Spear_Ace",
            "Boss","TutorialNoMove","BossUnit"};

        const char* elem_name = (elem >= 0 && elem < count) ? elems_names[elem] : "Unknown";
        ImGui::SliderInt("slider enum", &elem, 0, count - 1, elem_name);

        if (ImGui::Button("CreateEnemy"))
        {
            EnemySource source{};
            source.mEmitterPoint = {};
            source.mType = static_cast<EnemyType>(elem);
            fSpawn(source,Graphics_);
        }

        ImGui::InputInt("WaveNumber", &mCurrentWave);
        if (ImGui::Button("StartWave"))
        {
            fStartWave(mCurrentWave);
        }

        ImGui::Separator();

        if (ImGui::Button("AllClear"))
        {
            fAllClear();
        }

        ImGui::Text("LastTimer");
        ImGui::SameLine();
        ImGui::Text(std::to_string(mLastTimer).c_str());
        ImGui::RadioButton("ClearWave", fWaveClear());

        if (ImGui::Button("AddWaveTimer"))
        {
            mWaveTimer += 10.0f;
        }

        ImGui::Checkbox("DebugMode", &mDebugMode);

        if (ImGui::Button("Close"))
        {
            mOpenGuiMenu = false;
        }

        if(ImGui::Button("AllStun"))
        {
            for(const auto & enemy: mEnemyVec)
            {
                enemy->fSetStun(true);
            }
        }


        ImGui::InputFloat("SlowTime", &mSloeTime);

        ImGui::End();
    }
#endif
    mEditor.fGuiMenu();
}

void EnemyManager::fStartWave(int WaveIndex_)
{
    //--------------------<ウェーブを開始させる関数>--------------------//
    fAllClear();
    mWaveTimer = 0.0f;
    fLoad(mWaveFileNameArray[WaveIndex_]);

    // 何秒でこのウェーブが終わるかを初期化する
    mLastTimer = mCurrentWaveVec.back().mSpawnTimer;
    mDelay = 3.0f;
}

bool EnemyManager::fWaveClear() const
{
    // 最後の敵が出現しているかを判定する
    if (mLastTimer >= mWaveTimer)
    {
        return false;
    }
    // すべての敵が死んでいたら
    if (mEnemyVec.size() > 0)
    {
        return false;
    }
    return true;
}

void EnemyManager::fDeleteEnemies()
{
    // 削除
    for (const auto enemy : mRemoveVec)
    {
        auto e = std::find(mEnemyVec.begin(), mEnemyVec.end(), enemy);
        if (e != mEnemyVec.end())
        {
            safe_delete(*e);
            mEnemyVec.erase(e);
        }
    }
    mRemoveVec.clear();
}

std::vector<BaseEnemy*> EnemyManager::fGetEnemies() const
{
    return mEnemyVec;
}

size_t EnemyManager::fGetEnemyCounts() const
{
    return mEnemyVec.size();
}

void EnemyManager::fRegisterCash(GraphicsPipeline& graphics_)
{
    // キャッシュにモデルを登録
    BaseEnemy* enemy = new SwordEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ArcherEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ShieldEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new SpearEnemy(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ArcherEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new ShieldEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new SpearEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);

    enemy = new SwordEnemy_Ace(graphics_);
    mCashEnemyVec.emplace_back(enemy);
}

void EnemyManager::fDeleteCash()
{

    for (const auto enemy : mCashEnemyVec)
    {
        // 存在していれば削除
        if (enemy)
        {
            delete enemy;
        }
    }
    mCashEnemyVec.clear();
}



void EnemyManager::fCreateBossUnit(GraphicsPipeline& Graphics_)
{
    if (mIsReserveBossUnit == false) return;

    for(const auto unit:mUnitEntryPointVec)
    {
        BaseEnemy* enemy = new BossUnit(Graphics_,
            unit,
            mEditor.fGetParam(EnemyType::Boss_Unit),
            BulletManager::Instance().fGetAddFunction());
        mEnemyVec.emplace_back(enemy);
    }

    mIsReserveBossUnit = false;
    mUnitEntryPointVec.clear();
}

void EnemyManager::fSpawnTutorial_NoAttack(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // 敵の数が一定以下じゃないとダメ
    constexpr int maxEnemies{ 7 };

    if (mEnemyVec.size() < maxEnemies)
    {

        mTutorialTimer += elapsedTime_;
        if (mTutorialTimer > mkSeparateTutorial)
        {
            std::mt19937 mt{ std::random_device{}() };
            const std::uniform_int_distribution<int> RandTargetAdd(-5, 5);
            const int randNumber1 = RandTargetAdd(mt);
            const int randNumber2 = RandTargetAdd(mt);
            DirectX::XMFLOAT3 pos{ randNumber1 * 10.0f,0.0f,randNumber2 * 10.0f };

            mTutorialTimer = 0.0f;
            mEnemyVec.emplace_back(new TutorialEnemy_NoAttack(Graphics_, pos, mEditor.fGetParam(EnemyType::Tutorial_NoMove)));
        }
    }
}

void EnemyManager::fSpawnTutorial(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // 敵の数が一定以下じゃないとダメ
    constexpr int maxEnemies{ 7 };

    if (mEnemyVec.size() < maxEnemies)
    {

        mTutorialTimer += elapsedTime_;
        if (mTutorialTimer > mkSeparateTutorial)
        {
            std::mt19937 mt{ std::random_device{}() };
            const std::uniform_int_distribution<int> RandTargetAdd(-5, 5);
            const int randNumber1 = RandTargetAdd(mt);
            const int randNumber2 = RandTargetAdd(mt);
            DirectX::XMFLOAT3 pos{ randNumber1 * 10.0f,0.0f,randNumber2 * 10.0f };

            mTutorialTimer = 0.0f;
            mEnemyVec.emplace_back(new TutorialEnemy(Graphics_, pos, mEditor.fGetParam(EnemyType::Tutorial_NoMove)));
        }
    }
}

//****************************************************************
//
// ボス関連の関数
//
//****************************************************************
LastBoss::Mode EnemyManager::fGetBossMode() const
{
    return mCurrentMode;
}

void EnemyManager::fSetBossMode(LastBoss::Mode Mode_)
{
    mCurrentMode = Mode_;
}

bool EnemyManager::fGetIsEventCamera() const
{
    bool result{ false };
    switch (mCurrentMode) {
    case LastBoss::Mode::None:break;
    case LastBoss::Mode::Ship:break;
    case LastBoss::Mode::ShipToHuman:
        result = true;
        break;
    case LastBoss::Mode::Human: break;
    case LastBoss::Mode::HumanToDragon:
        result = true;
        break;
    case LastBoss::Mode::Dragon:break;
    case LastBoss::Mode::DragonDie:
        result = true;
        break;
    case LastBoss::Mode::BossDieEnd: break;
    case LastBoss::Mode::ShipAppear:
        result = true;
        break;
    default: ;
    }

    return result;
}

DirectX::XMFLOAT3 EnemyManager::fGetEye() const
{
    return mBossCameraEye;
}

DirectX::XMFLOAT3 EnemyManager::fGetFocus() const
{
    return mBossCameraFocus;
}

void EnemyManager::fSetBossEye(DirectX::XMFLOAT3 Eye_)
{
    mBossCameraEye = Eye_;
}

void EnemyManager::fSetBossFocus(DirectX::XMFLOAT3 Focus_)
{
    mBossCameraFocus = Focus_;
}

bool EnemyManager::fGetBossClear() const
{
    return mCurrentMode == LastBoss::Mode::BossDieEnd;
}

void EnemyManager::fReserveDeleteEnemies()
{
    mIsReserveDelete = true;
}

void EnemyManager::fLimitEnemies()
{
    for(auto enemy: mEnemyVec)
    {
        enemy->fLimitPosition();
    }
}
