#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"EnemyManager.h"
#include "EnemyFileSystem.h"
#include"EnemyStructuer.h"
#include"WaveManager.h"
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

#include"Correspondence.h"
#include"NetWorkInformationStucture.h"

#include <fstream>
#include <chrono>


//****************************************************************
//
// 敵の管理クラス
//
//****************************************************************

EnemyManager::EnemyManager()
{
    //-----敵のマスターデータ管理クラスの実態生成-----//
    master_enemy_data = std::make_unique<MasterEnemyDataAdmin>();
}

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

void EnemyManager::RestartInitialize()
{
    //--------------------<初期化>--------------------//
    fAllClear();
    mUniqueCount = 0;
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
        for (const auto enemy : mEnemyVec)
        {
            //-----体力が0の時に死亡処理だけ通す-----//
            if (enemy->fGetCurrentHitPoint() > 0) continue;

            enemy->fDie(graphics_);


            if (enemy->fGetIsAlive() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        return;
    }

    //-----敵のリーダーのデータを設定-----//
    SetEnemyGropeHostData();

    //--------------------<敵の更新処理>--------------------//
    fEnemiesUpdate(graphics_,elapsedTime_);

    //-----敵のデータを設定,送信-----//
    fCheckSendEnemyData(elapsedTime_);

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
                enemy->fDie(graphics_);
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }
}

void EnemyManager::fDebugUpdate(GraphicsPipeline& graphics_, float elapsedTime_, AddBulletFunc Func_)
{
    for (const auto enemy : mEnemyVec)
    {
        //-----敵の更新-----//
        enemy->fUpdate(graphics_, elapsedTime_);
    }
    // ImGuiのメニュー
    fGuiMenu(graphics_, Func_);

}

void EnemyManager::fHostUpdate(GraphicsPipeline& graphics_, float elapsedTime_, AddBulletFunc Func_, EnemyAllDataStruct& receive_data)
{
    //--------------------<管理クラス自体の更新処理>--------------------//
    mSloeTime -= elapsedTime_;

    // ウェーブ開始からの時間を更新
    mDelay -= elapsedTime_;
    if (!mDebugMode && !mIsTutorial && mDelay <= 0.0f)
    {
        //--------------------<敵がたまりすぎたら>--------------------//
        if (mEnemyVec.size() < 30)
        {
            mWaveTimer += elapsedTime_;
        }
    }

    // カメラシェイク
    if (mCameraShakeTime > 0.0f)
    {
        if (GameFile::get_instance().get_shake()) camera_shake->shake(graphics_, elapsedTime_);
    }
    mCameraShakeTime -= elapsedTime_;
    mCameraShakeTime = (std::max)(0.0f, mCameraShakeTime);

    //--------------<プレイヤーがチェイン中はエネミーの行動をすべて停止させる>-------------//
    if (mIsPlayerChainTime)
    {
        for (const auto enemy : mEnemyVec)
        {
            //-----体力が0の時に死亡処理だけ通す-----//
            if (enemy->fGetCurrentHitPoint() > 0) continue;

            enemy->fDie(graphics_);


            if (enemy->fGetIsAlive() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        return;
    }


    //-----受信した敵のデータを設定する-----//
    for (const auto& data : receive_data.enemy_die_data)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetObjectId() == data.object_id)
            {
                enemy->fDie(graphics_);
            }
        }
    }

    //----状態データを設定-----//
    for (const auto& data : receive_data.enemy_condition_data)
    {
        fSetReceiveConditionData(data);
    }

    //-----移動データを設定-----//
    for (const auto& all_data : receive_data.enemy_move_data)
    {
        for (const auto& e_data : all_data.enemy_data)
        {
            fSetReceiveEnemyData(elapsedTime_, e_data,graphics_);
        }
    }


    //-----敵のリーダーのデータを設定-----//
    SetEnemyGropeHostData();

    //--------------------<敵の更新処理>--------------------//
    fEnemiesUpdate(graphics_, elapsedTime_);

    //-----敵のデータを設定,送信-----//
    fCheckSendEnemyData(elapsedTime_);

    //--------------------<敵同士の当たり判定>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<敵のスポナー>--------------------//
    if (CorrespondenceManager::Instance().GetMultiPlay())
    {
        //-----マルチプレイ時はホストしか敵の出現処理をしない-----//
        if (CorrespondenceManager::Instance().GetHost())fSpawn(graphics_);
    }

    // ImGuiのメニュー
    fGuiMenu(graphics_, Func_);

    //--------------------<ボスが敵を召喚する>--------------------//
    fCreateBossUnit(graphics_);

    bool isCreate{};
    for (const auto& source : mReserveVec)
    {
        isCreate = true;
        fSpawn(source, graphics_);
    }
    if (isCreate)
    {
        mReserveVec.clear();
    }

    // ザコ的だけを全消しする
    if (mIsReserveDelete)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetIsBoss() == false)
            {
                enemy->fDie(graphics_);
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }

}

void EnemyManager::fClientUpdate(GraphicsPipeline& graphics_, float elapsedTime_,AddBulletFunc Func_, EnemyAllDataStruct& receive_data)
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
        for (const auto enemy : mEnemyVec)
        {
            //-----体力が0の時に死亡処理だけ通す-----//
            if (enemy->fGetCurrentHitPoint() > 0) continue;

            enemy->fDie(graphics_);


            if (enemy->fGetIsAlive() == false)
            {
                mRemoveVec.emplace_back(enemy);
            }
        }
        return;
    }

    //-----受信した敵のデータを設定する-----//
    for (const auto& data : receive_data.enemy_die_data)
    {
        for (const auto enemy : mEnemyVec)
        {
            if (enemy->fGetObjectId() == data.object_id)
            {
                enemy->fDie(graphics_);
            }
        }
    }

    //----状態データを設定-----//
    for (const auto& data : receive_data.enemy_condition_data)
    {
        fSetReceiveConditionData(data);
    }

    //-----移動データを設定-----//
    for (const auto& all_data : receive_data.enemy_move_data)
    {
        for (const auto& e_data : all_data.enemy_data)
        {
            fSetReceiveEnemyData(elapsedTime_,e_data,graphics_);
        }
    }

    //-----敵のリーダーのデータを設定-----//
    SetEnemyGropeHostData();

    //--------------------<敵の更新処理>--------------------//
    fEnemiesUpdate(graphics_,elapsedTime_);

    //--------------------<敵同士の当たり判定>--------------------//
    fCollisionEnemyVsEnemy();

    //--------------------<敵のスポナー>--------------------//
    for (auto data : receive_data.enemy_spawn_data)
    {
        fSpawn(data, graphics_);
    }

    // ImGuiのメニュー
    fGuiMenu(graphics_,Func_);


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
                enemy->fDie(graphics_);
                mRemoveVec.emplace_back(enemy);
            }
        }
        fDeleteEnemies();
        mIsReserveDelete = false;
    }
}

void EnemyManager::fCheckSendEnemyData(float elapsedTime_)
{
    if (CorrespondenceManager::Instance().GetMultiPlay() == false) return;

    //------AIの調整でのデータ送信-----//
    if (fGetIsEventCamera())
    {
        check_boss_ai_timer += elapsedTime_ * 1.0f;
    }

    //-----時間を取得-----//
    static auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();

    //-----スタートとエンドの差分を出す-----//
    auto dur = end - start;

    //-----ミリ秒に変換する----//
    milliseconds = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());


    float value{};
    if (milliseconds > EnemyDataFrequency)
    {
        auto grah_start = std::chrono::system_clock::now();
        //-----データを送る-----//
        fSendEnemyData(elapsedTime_);

        //-----タイマーを初期化-----//
        start = std::chrono::system_clock::now();
        auto grah_end = std::chrono::system_clock::now();
        auto grah_dur = grah_end - grah_start;
        value = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(grah_dur).count());
    }

}

void EnemyManager::fSendEnemyData(float elapsedTime_)
{
    using namespace EnemySendData;

    if (fGetIsEventCamera())
    {
        if (check_boss_ai_timer < CheckBossAiTime) return;
        else
        {
            check_boss_ai_timer = 0.0f;
        }
    }


    char data[512]{};
    //-----コマンドを設定する-----//
    data[ComLocation::ComList] = CommandList::Update;
    data[ComLocation::UpdateCom] = UpdateCommand::EnemiesMoveCommand;

    int data_set_count = 0;

    EnemySendData::EnemyData enemy_d;

    //-----マスターのデータを送信する-----//
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fGetMaster() == false) continue;

        //-----オブジェクト番号設定-----//
        enemy_d.enemy_data[EnemyDataArray::ObjectId] = enemy->fGetObjectId();

        //-----AIのステート設定-----//
        enemy_d.enemy_data[EnemyDataArray::AiState] = enemy->fGetEnemyAiState();

        //-----ターゲットしているプレイヤーのId-----//
        enemy_d.enemy_data[EnemyDataArray::TargetId] = enemy->fGetTargetPlayerId();

        //-----敵のタイプ-----//
        enemy_d.enemy_data[EnemyDataArray::EnemyTypeId] = static_cast<char>(enemy->GetEnemyType());

        //-----体力-----//
        enemy_d.hp = static_cast<int>(enemy->fGetCurrentHitPoint());

        //-----自分の位置を設定-----//
        enemy_d.pos.x = static_cast<int16_t>(enemy->fGetPosition().x);
        enemy_d.pos.y = static_cast<int16_t>(enemy->fGetPosition().y);
        enemy_d.pos.z = static_cast<int16_t>(enemy->fGetPosition().z);

        std::memcpy(data + SendEnemyDataComSize + (sizeof(EnemyData) * data_set_count), (char*)&enemy_d,sizeof(EnemyData));

        data_set_count++;
    }

    //-----送るデータが無いときはここで終わる-----//
    if (data_set_count <= 0) return;

    //-----データサイズを設定-----//
    data[ComLocation::Other] = data_set_count;

    int size = SendEnemyDataComSize + (sizeof(EnemyData) * data_set_count);

    CorrespondenceManager::Instance().MultiCastSend(data,size);

}

void EnemyManager::fSendEnemyDamage(int obj_id, int damage)
{
    EnemySendData::EnemyDamageData d;
    d.data[ComLocation::ComList] = CommandList::Update;
    d.data[ComLocation::UpdateCom] = UpdateCommand::EnemyDamageCommand;
    d.data[EnemySendData::EnemyDamageCmdArray::DamageComEnemyId] = obj_id;
    d.data[EnemySendData::EnemyDamageCmdArray::DamageComDamage] = damage;

    //-----ホストにダメージデータを送信-----//
    CorrespondenceManager::Instance().UdpSend(CorrespondenceManager::Instance().GetHostId(),(char*)&d, sizeof(EnemySendData::EnemyDamageData));
}

void EnemyManager::fSetReceiveEnemyData(float elapsedTime_, EnemySendData::EnemyData data, GraphicsPipeline& graphics_)
{

    for (const auto& enemy : mEnemyVec)
    {
        //-----自分のオブジェクト番号とデータの番号が違うならとばす-----//
        if (enemy->fGetObjectId() != data.enemy_data[EnemySendData::EnemyDataArray::ObjectId]) continue;

        DirectX::XMFLOAT3 pos{};
        pos.x = static_cast<int16_t>(data.pos.x);
        pos.y = static_cast<int16_t>(data.pos.y);
        pos.z = static_cast<int16_t>(data.pos.z);
        //-----自分の位置を設定-----//
        enemy->fSetReceivePosition(pos);

        //-----AIステート設定-----//
        enemy->fSetEnemyState(data.enemy_data[EnemySendData::EnemyDataArray::AiState]);

        //-----ターゲットの位置を設定-----//
        enemy->fSetTargetPlayerId(data.enemy_data[EnemySendData::EnemyDataArray::TargetId]);

        //-----体力設定------//
        enemy->fSetCurrentHitPoint(data.hp);
        break;
    }

}


void EnemyManager::fSetReceiveConditionData(EnemySendData::EnemyConditionData data)
{
    for (const auto enemy : mEnemyVec)
    {
        if (enemy->fGetObjectId() == data.data[EnemySendData::EnemyConditionArray::EnemyConditionObjectId])
        {
            switch (data.data[EnemySendData::EnemyConditionArray::EnemyCondition])
            {
                //-----スタンになっていないならスタンさせる-----//
            case EnemySendData::EnemyConditionEnum::Stun:
                enemy->fSetStun(true);
                break;
            default:
                break;
            }

        }
    }
}


void EnemyManager::SetReciveDamageData(int obj_id, int damage, GraphicsPipeline& graphics_)
{
    for (const auto& enemy : mEnemyVec)
    {
        //-----IDが違うならとばす-----//
        if (enemy->fGetObjectId() != obj_id) continue;

            //----ダメージを設定する-----//
        enemy->fDamaged(damage, 0.0f, graphics_, 0.0f);
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
        if (enemy->fGetAppears() == false) continue;
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
                if(enemy->fDamaged(PlayerAttackPower_, EnemyInvincibleTime,Graphics_,elapsedTime_))
                {
                    audio_manager->play_se(SE_INDEX::ATTACK_SWORD);

                    //攻撃を防がれたら即リターン
                    hitCounts++;
                    mSloeTime = 0.1f;

                    //-----マルチプレイの時かつホストでは無いとき-----//
                    if (CorrespondenceManager::Instance().GetMultiPlay() &&
                        CorrespondenceManager::Instance().GetHost() == false)
                    {
                        //-----ダメージのデータを設定する-----//
                        fSendEnemyDamage(enemy->fGetObjectId(), PlayerAttackPower_);
                    }
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
            //-----攻撃動作に入っていたら-----//
            if (enemy->fGetAttackOperation())
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
        //-----出現していなかったら判定しない-----//
        if (enemy->fGetAppears() == false) continue;
        const DirectX::XMFLOAT3 enemyPosition = enemy->fGetPosition();
        const float enemyRad = enemy->fGetBodyCapsule().mRadius;
        for (const auto p : mPlayerPosition)
        {
            if (Math::Length(p - enemyPosition) <= Radius_ + enemyRad)
            {
                enemy->fSetStun(true);
            }
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
        if (enemy->fGetStun() || enemy->fGetAppears() == false || enemy->fGetIsAlive() == false) continue;


        if (enemy->fComputeAndGetIntoCamera())
        {
            //// この敵からの距離を計算する
            //for(const auto enemy2:mEnemyVec)
            //{
            //    if (enemy2->fComputeAndGetIntoCamera())
            //    {
            //        if (enemy != enemy2)
            //        {
            //            //enemy2->fCalcNearestEnemy(enemy->fGetPosition());
            //        }
            //    }
            //}
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

void EnemyManager::fSetPlayerPosition(std::vector<std::tuple<int, DirectX::XMFLOAT3>> Position_)
{

    //std::tuple<object_id,position>

    //-----中に入っているデータを削除-----//
    mPlayerPosition.clear();

    //-----一番近いプレイヤーの位置-----//
    DirectX::XMFLOAT3 near_pos{};

    //-----一番近いプレイヤーの位置との距離-----//
    float near_length{ FLT_MAX };

    int id{ -1 };
    //-----計算した長さを入れる-----//
    float length{};

    for(const auto& enemy:mEnemyVec)
    {
        near_pos = {};
        near_length = FLT_MAX;
        for (const auto p : Position_)
        {
            length = Math::calc_vector_AtoB_length(enemy->fGetPosition(), std::get<1>(p));

            //-----位置を配列に保存-----//
            mPlayerPosition.emplace_back(std::get<1>(p));

            //-----一番近い距離と今の敵の位置との距離を比較して小さかったら値を更新-----//
            if (near_length > length)
            {
                near_length = length;

                //-----位置を保存-----//
                near_pos = std::get<1>(p);
                //-----idを保存------//
                id = std::get<0>(p);
            }
        }
        enemy->fSetPlayerPosition(near_pos);
        enemy->fSetTargetPlayerId(id);
    }


}


void EnemyManager::fSetPlayerIdPosition(const std::vector<std::shared_ptr<BasePlayer>>& players)
{
    for (const auto& enemy : mEnemyVec)
    {
        for (const auto player : players)
        {
            //-----ターゲットのIDと違うならとばす-----//
            if (enemy->fGetTargetPlayerId() != player->GetObjectId()) continue;

            //-----位置を設定する-----//
            enemy->fSetPlayerPosition(player->GetPosition());
            break;
        }
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

    std::map<int, EnemySource> s_data;
    // 敵をスポーンする関数
    for (const auto data : mCurrentWaveVec)
    {
        // 出現条件を満たしていたら出す
        if (data.mSpawnTimer <= mWaveTimer)
        {
            s_data.insert(std::make_pair(object_count,data));
            fSpawn(data, graphics);
            spawnCounts++;
        }
    }

    if (s_data.empty() == false)
    {
        fSendSpawnData(s_data);
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


    BaseEnemy* enemy = nullptr;

    //-----switch文の中-----//
    //実態生成
    //ID設定
    //種類設定
    //グループデータ設定

    switch (Source_.mType)
    {
    case EnemyType::Archer:
    {
        enemy = new ArcherEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Archer);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Shield:
    {
        enemy = new ShieldEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Shield);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Sword:
    {
        enemy = new SwordEnemy(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Sword);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Spear:
    {
        enemy = new SpearEnemy(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Spear);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        enemy = new ArcherEnemy_Ace(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Archer_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Shield_Ace:
    {
        enemy = new ShieldEnemy_Ace(graphics_,
            Source_.mEmitterPoint, param);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Shield_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Sword_Ace:
    {
        enemy = new SwordEnemy_Ace(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Sword_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        enemy = new SpearEnemy_Ace(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Spear_Ace);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Boss:
    {
        enemy = new LastBoss(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType), this);
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyType(EnemyType::Boss);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Count: break;
    case EnemyType::Tutorial_NoMove:
    {
        enemy = new TutorialEnemy_NoAttack(graphics_,
            Source_.mEmitterPoint,mEditor.fGetParam(Source_.mType));
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyGropeData(Source_.master, Source_.transfer_host, Source_.grope_id);
    }
    break;
    case EnemyType::Boss_Unit:
        break;
        //-----敵のタグが無いときはここでリターンする-----//
    default:
        return;
        break;
    }



    //-----敵のvectorに入れる-----//
    if (enemy != nullptr)mEnemyVec.emplace_back(enemy);

    object_count++;
}

void EnemyManager::fSpawn(EnemySendData::EnemySpawnData data, GraphicsPipeline& graphics_)
{
    //-----受信したデータから敵の種類を取得-----//
    EnemyType type = static_cast<EnemyType>(data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyType]);

    //-----番号を取得-----//
    int id = data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyId];

    // 送られてきたデータをもとに敵を出現させる
    const auto param = mEditor.fGetParam(type);

    //-----マスターかどうか-----//
    bool master = static_cast<bool>(data.cmd[EnemySendData::EnemySpawnCmdArray::Master]);

    //-----グループ番号-----//
    int grope_id = static_cast<int>(data.cmd[EnemySendData::EnemySpawnCmdArray::GropeId]);

    //-----ホスト譲渡優先度-----//
    int transfer = static_cast<int>(data.grope_data);

    DirectX::XMFLOAT3 emit_pos{};
    emit_pos.x = static_cast<float>(data.emitter_point.x);
    emit_pos.z = static_cast<float>(data.emitter_point.y);

    switch (type)
    {
    case EnemyType::Archer:
    {
        BaseEnemy* enemy = new ArcherEnemy(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Archer);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield:
    {
        BaseEnemy* enemy = new ShieldEnemy(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Shield);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword:
    {
        BaseEnemy* enemy = new SwordEnemy(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Sword);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear:
    {
        BaseEnemy* enemy = new SpearEnemy(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Spear);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Archer_Ace:
    {
        BaseEnemy* enemy = new ArcherEnemy_Ace(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Archer_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Shield_Ace:
    {
        BaseEnemy* enemy = new ShieldEnemy_Ace(graphics_,
            emit_pos, param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Shield_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Sword_Ace:
    {
        BaseEnemy* enemy = new SwordEnemy_Ace(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Sword_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Spear_Ace:
    {
        BaseEnemy* enemy = new SpearEnemy_Ace(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Spear_Ace);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Boss:
    {
        BaseEnemy* enemy = new LastBoss(graphics_,
            emit_pos,
            param, this);
        enemy->fSetObjectId(id);
        enemy->SetEnemyType(EnemyType::Boss);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
    }
    break;
    case EnemyType::Count: break;
    case EnemyType::Tutorial_NoMove:
    {
        BaseEnemy* enemy = new TutorialEnemy_NoAttack(graphics_,
            emit_pos,
            param);
        enemy->fSetObjectId(id);
        mEnemyVec.emplace_back(enemy);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
    }
    break;
    case EnemyType::Boss_Unit:
    {
        BaseEnemy* enemy = new BossUnit(graphics_,
            emit_pos,
            param,
            BulletManager::Instance().fGetAddFunction());
        enemy->fSetObjectId(id);
        enemy->SetEnemyGropeData(master, transfer, grope_id);
        mEnemyVec.emplace_back(enemy);
        std::string text = "敵ユニット生成" + std::to_string(id);
        DebugConsole::Instance().WriteDebugConsole(text,TextColor::Pink);
    }
    break;
    default:
        break;
    }

}


void EnemyManager::fSendSpawnData(std::map<int, EnemySource> spawn_data)
{
    if (CorrespondenceManager::Instance().GetMultiPlay() == false) return;

    using namespace EnemySendData;

    int data_set_count = 0;

    char data[512]{};
    //-----コマンドを設定する-----//
    data[ComLocation::ComList] = CommandList::EnemySpawnCommand;


    EnemySpawnData d;

    for (const auto s_data : spawn_data)
    {
        //-----コマンドを設定-----//
        d.cmd[EnemySpawnCmdArray::Master] = s_data.second.master;
        d.cmd[EnemySpawnCmdArray::GropeId] = s_data.second.grope_id;
        d.cmd[EnemySpawnCmdArray::EnemyId] = s_data.first;
        d.cmd[EnemySpawnCmdArray::EnemyType] = static_cast<int>(s_data.second.mType);

        //-----出現位置-----//
        d.emitter_point.x = static_cast<int16_t>(s_data.second.mEmitterPoint.x);
        d.emitter_point.y = static_cast<int16_t>(s_data.second.mEmitterPoint.z);

        //-----マスターの譲渡順-----//
        d.grope_data = s_data.second.transfer_host;

        std::memcpy(data + SendSpawnEnemyDataComSize + (sizeof(EnemySpawnData) * data_set_count), (char*)&d, sizeof(EnemySpawnData));

        data_set_count++;
    }

    //-----送るデータが無いときはここで終わる-----//
    if (data_set_count <= 0) return;

    //-----データサイズを設定-----//
    data[static_cast<int>(SendEnemySpawnData::SpawnNum)] = data_set_count;

    int size = SendSpawnEnemyDataComSize + (sizeof(EnemySpawnData) * data_set_count);


    CorrespondenceManager::Instance().TcpSendAllClient(data,size);

#if 0
    //-----コマンドを設定-----//
    data.cmd[ComLocation::ComList] = CommandList::EnemySpawnCommand;

    //-----敵のID-----//
    data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyId] = object_count;

    //-----敵のタイプ-----//
    data.cmd[EnemySendData::EnemySpawnCmdArray::EnemyType] = static_cast<int>(Source_.mType);

    //-----出現位置-----//
    data.emitter_point.x = static_cast<int16_t>(Source_.mEmitterPoint.x);
    data.emitter_point.y = static_cast<int16_t>(Source_.mEmitterPoint.y);
    data.emitter_point.z = static_cast<int16_t>(Source_.mEmitterPoint.z);

    //-----グループデータを設定-----//

    //-----マスターかどうか-----//
    data.grope_data[EnemySendData::EnemySpawnGropeArray::Master] = Source_.master;

    //-----グループの番号-----//
    data.grope_data[EnemySendData::EnemySpawnGropeArray::GropeId] = Source_.grope_id;

    //-----マスターの譲渡順-----//
    data.grope_data[EnemySendData::EnemySpawnGropeArray::Transfer] = Source_.transfer_host;


    CorrespondenceManager::Instance().TcpSendAllClient((char*)&data, sizeof(EnemySendData::EnemySpawnData));

#endif // 0

    DebugConsole::Instance().WriteDebugConsole("敵出現データ送信 : " +std::to_string(size) + "バイト" , TextColor::Pink);
}

void EnemyManager::fEnemiesUpdate(GraphicsPipeline& Graphics_,float elapsedTime_)
{
    // 更新
    for (const auto enemy : mEnemyVec)
    {
        //-----リーダーのデータを取得-----//
        if (enemy->fGetMaster() == false)
        {
            //-----構造化束縛でtupleのデータを取得-----//
            auto [check, data] = master_enemy_data->GetMasterData(enemy->fGetGropeId());
            //-----trueならリーダーが存在してデータを取得することができる-----//
            if (check) enemy->fSetMasterData(data.position, data.ai_state, data.target_id);

        }

        //-----敵の更新-----//
        enemy->fUpdate(Graphics_, elapsedTime_);

        //-----死んでいたら削除用のvectorに登録-----//
        if (enemy->fGetIsAlive() == false)
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
        //-----ボスラッシュ攻撃のモデルが来た時に攻撃のフラグがtrueじゃなかったらとばす-----//
        if (enemy->GetEnemyType() == EnemyType::BossRush && start_boss_rush == false) continue;
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

void EnemyManager::fCreateRandomMasterEnemy(GraphicsPipeline& Graphics_, DirectX::XMFLOAT3 SeedPosition_, int grope_id)
{
    if (mEnemyVec.size() > 15)
    {
        return;
    }

    // 乱数で敵のタイプを取得
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_int_distribution<int> RandTargetAdd(0, 7);
    int randNumber = RandTargetAdd(mt);

    // チュートリアルの敵なら違う敵を出す
    if (randNumber == 9 || randNumber == 8)
    {
        randNumber = 10;
    }

    EnemySource source;

    std::uniform_int_distribution<int> RandTargetAdd2(-5, 5);
    const int randPosition = RandTargetAdd2(mt);
    const int randPositionX = RandTargetAdd2(mt);
    const int randPositionY = RandTargetAdd2(mt);
    source.mEmitterPoint =
    {
        SeedPosition_.x +
        (static_cast<float>(randPosition) * static_cast<float>(randPositionX)),
        0.0f,
        SeedPosition_.z +
        static_cast<float>(randPosition) * static_cast<float>(randPositionY),
    };

    source.mType = static_cast<EnemyType>(randNumber);
    source.master = true;
    source.grope_id = grope_id;
    source.transfer_host = 0;
    mReserveVec.emplace_back(source);
}

void EnemyManager::fCreateRandomEnemy(
    GraphicsPipeline& Graphics_,
    DirectX::XMFLOAT3 SeedPosition_, int grope_id, int transfer_id)
{
    if(mEnemyVec.size()>15)
    {
        return;
    }

    // 乱数で敵のタイプを取得
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_int_distribution<int> RandTargetAdd(0, 7);
    int randNumber = RandTargetAdd(mt);

    // チュートリアルの敵なら違う敵を出す
    if (randNumber == 9 || randNumber == 8)
    {
        randNumber = 10;
    }

    EnemySource source;

    std::uniform_int_distribution<int> RandTargetAdd2(-5, 5);
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
    source.master = false;
    source.grope_id = grope_id;
    source.transfer_host = transfer_id;
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
    for (auto enemy : mEnemyVec)
    {
        enemy->StopEffec();
        // 存在していれば削除
        if (enemy)
        {
            delete enemy;
            enemy = nullptr;
        }
    }
    mEnemyVec.clear();

    //-----マスターデータの削除-----//
    master_enemy_data->ResetMasterData();
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

void EnemyManager::fSave(const char* FileName_)
{
    // Jsonファイルから値を取得
    std::filesystem::path path = FileName_;
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ofstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONOutputArchive o_archive(ifs);
            o_archive(mCurrentWaveVec);
        }
    }
}

void EnemyManager::SetEnemyGropeHostData()
{
    for (const auto& ene : mEnemyVec)
    {
        //-----リーダーじゃないならとばす-----//
        if (ene->fGetMaster() == false) continue;

        //-----リーダーがスタンしている場合はホスト権限を譲渡する-----//
        if(ene->fGetStun()) TransferMaster(ene->fGetGropeId());

        //-----リーダーのデータを設定する-----//
        master_enemy_data->SetMasterData(ene->fGetGropeId(),ene->fGetPosition(),ene->fGetTargetPlayerId(),ene->fGetEnemyAiState());
    }

}

void EnemyManager::TransferMaster(int grope)
{
    int transfer{ INT_MAX };
    BaseEnemy* e{ nullptr };

    for (const auto& enemy : mEnemyVec)
    {
        //-----マスターの場合かグループ番号が違うならとばす-----//
        if (enemy->fGetMaster() || enemy->fGetGropeId() != grope) continue;

        //-----譲渡番号が一番小さい敵を取得-----//
        if (transfer > enemy->fGetTransfer())
        {
            transfer = enemy->fGetTransfer();
            e = enemy;
        }
    }

    //-----もし値が入っているならマスターに昇格する-----//
    if (e != nullptr)e->fSetMaster(true);
    //-----値が入っていないなら譲渡先がいないのでそのデータは消す-----//
    else master_enemy_data->DeleteSpecificData(grope);
}

void EnemyManager::DebugLoadEnemyParam()
{
    mEditor.HttpLoad();
}

void EnemyManager::fGuiMenu(GraphicsPipeline& Graphics_, AddBulletFunc Func_)
{
    imgui_menu_bar("Game", "EnemyManager", mOpenGuiMenu);


#ifdef USE_IMGUI
    if (mOpenGuiMenu)
    {
        ImGui::Begin("EnemyManager");

        int size = static_cast<int>(mEnemyVec.size());

        ImGui::Text("EnemyNum%d", size);

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

    //-----敵のカウントを初期化する-----//
    object_count = 0;

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
            //-----マスターの場合譲渡する-----//
            if ((*e)->fGetMaster())
            {
                TransferMaster((*e)->fGetGropeId());
            }

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

    //enemy = new BossUnit(graphics_);
    //mCashEnemyVec.emplace_back(enemy);

}

void EnemyManager::fDeleteCash()
{

    for (auto enemy : mCashEnemyVec)
    {
        // 存在していれば削除
        if (enemy)
        {
            delete enemy;
            enemy = nullptr;
        }
    }
    mCashEnemyVec.clear();
}



void EnemyManager::fCreateBossUnit(GraphicsPipeline& Graphics_)
{
    if (mIsReserveBossUnit == false) return;

    int grope_id{ 1 };

    std::map<int, EnemySource> s_data;

    for(const auto unit:mUnitEntryPointVec)
    {
        //-----マルチプレイの時にデータを送信-----//
        if (CorrespondenceManager::Instance().GetMultiPlay())
        {
            EnemySource data;
            data.master = true;
            data.grope_id = grope_id;
            data.mEmitterPoint = unit;
            data.mType = EnemyType::Boss_Unit;

            s_data.insert(std::make_pair(object_count, data));
        }

        BaseEnemy* enemy = new BossUnit(Graphics_,
            unit,
            mEditor.fGetParam(EnemyType::Boss_Unit),
            BulletManager::Instance().fGetAddFunction());
        enemy->fSetObjectId(object_count);
        enemy->SetEnemyGropeData(true, 0, grope_id);
        mEnemyVec.emplace_back(enemy);
        grope_id++;
        object_count++;
    }

    if (s_data.empty() == false)
    {
        fSendSpawnData(s_data);
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
            std::uniform_int_distribution<int> RandTargetAdd(-5, 5);
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
            std::uniform_int_distribution<int> RandTargetAdd(-5, 5);
            const int randNumber1 = RandTargetAdd(mt);
            const int randNumber2 = RandTargetAdd(mt);
            DirectX::XMFLOAT3 pos{ randNumber1 * 10.0f,0.0f,randNumber2 * 10.0f };

            mTutorialTimer = 0.0f;

            BaseEnemy* enemy = new TutorialEnemy(Graphics_, pos, mEditor.fGetParam(EnemyType::Tutorial_NoMove));
            mEnemyVec.emplace_back(enemy);
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

void EnemyManager::EndEnventCount(int count)
{
    end_event_count += count;
    //-----接続した人数と同じならイベントを進める-----//
    if (end_event_count == CorrespondenceManager::Instance().GetConnectedPersons())
    {

        for (auto enemy : mEnemyVec)
        {
            //-----ボスじゃなかったらとばす-----//
            if (enemy->GetEnemyType() != EnemyType::Boss) continue;
            enemy->SetEndEvent(true);
            //----次に備えてカウントを減らす-----//
            end_event_count = 0;
            DebugConsole::Instance().WriteDebugConsole("イベントを終了", TextColor::Blue);
            break;
        }

    }
}

void EnemyManager::EndEvent()
{
    for (auto enemy : mEnemyVec)
    {
        //-----ボスじゃなかったらとばす-----//
        if (enemy->GetEnemyType() != EnemyType::Boss) continue;
        enemy->SetEndEvent(true);
        break;
    }
}
