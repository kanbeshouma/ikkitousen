#include"LastBoss.h"
#include"Operators.h"
#include"post_effect.h"
#include"BulletManager.h"
#include "CannonballBullet.h"
#include "DragonBreath.h"
#include "EnemyManager.h"
#include"Operators.h"
#include"audio_manager.h"

LastBoss::BossParamJson LastBoss::fLoadParam()
{
    // Jsonファイルから値を取得
    std::filesystem::path path = "./resources/Data/BossParam.json";
    path.replace_extension(".json");
    if (std::filesystem::exists(path.c_str()))
    {
        std::ifstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONInputArchive o_archive(ifs);
            o_archive(mBossParam);
        }
    }

    return mBossParam;
}

void LastBoss::fResetLoadRaram()
{
    mBossParam.BossStateNumber = 0;
}

void LastBoss::fSaveParam()
{
    // Jsonファイルから値を取得
    std::filesystem::path path = "./resources/Data/BossParam.json";
    path.replace_extension(".json");
    //if (std::filesystem::exists(path.c_str()))
    {
        std::ofstream ifs;
        ifs.open(path);
        if (ifs)
        {
            cereal::JSONOutputArchive o_archive(ifs);
            o_archive(mBossParam);
        }
    }
}

//****************************************************************
// 
// 戦艦モード 
//
//****************************************************************
void LastBoss::fShipStartInit()
{
    // アニメーションを再生
    mpModel->play_animation(mAnimPara, AnimationName::ship_appearing_scene);
    mDissolve = 0.0f;
    mPosition = { 0.0f,40.0f,250.0f };
    mCurrentMode = Mode::ShipAppear;
    mTimer = 0.0f;
    mShipRoar = false;
    mSkipTimer = 0.0f;
}

void LastBoss::fShipStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    if(mTimer>14.5f&& mTimer < 18.0f)
    {
        const int t = static_cast<int>(mTimer * 30);
        if (!mIsSpawnEnemy && t % 10 == 0)
        {
            // ランダムな敵を出現させる
            auto vec = Math::GetFront(mOrientation);
            DirectX::XMFLOAT3 pos = mPosition;
            pos.y = 0.0f;
            mpEnemyManager->fCreateRandomEnemy(Graphics_, pos + (vec * 40.0f));
            mpEnemyManager->fCreateRandomEnemy(Graphics_, pos + (vec * 40.0f));
            mpEnemyManager->fCreateRandomEnemy(Graphics_, pos + (vec * 40.0f));
            mIsSpawnEnemy = true;
        }
        if (mIsSpawnEnemy && t % 10 == 9)
        {
            mIsSpawnEnemy = false;
        }
    }

    if(mTimer>15.0f&&mShipRoar==false)
    {
        audio_manager->play_se(SE_INDEX::ROAR_3);
        mShipRoar = true;
    }

    if (mpModel->end_of_animation(mAnimPara))
    {
        mCurrentMode = Mode::Ship;
        fChangeState(DivideState::ShipIdle);
    }
}

void LastBoss::fShipIdleInit()
{
    mTimer = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::ship_idle, true);
    mIsSpawnEnemy = false;
}

void LastBoss::fShipIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer += elapsedTime_;
    fTurnToPlayer(elapsedTime_, 5.0f);
   if(mTimer>30.0f)
   {
       fChangeState(DivideState::ShipBeamStart);
   }

    const int t = static_cast<int>(mTimer * 10);
   if (!mIsSpawnEnemy && t % 10 == 0)
   {
       // ランダムな敵を出現させる
       mpEnemyManager->fCreateRandomEnemy(Graphics_, mPlayerPosition);
       mIsSpawnEnemy = true;
   }
   if(mIsSpawnEnemy && t % 10 == 9)
   {
       mIsSpawnEnemy = false;
   }

}

void LastBoss::fShipAttackInit()
{

}

void LastBoss::fShipAttackUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

}

void LastBoss::fShipBeamStartInit()
{
    mpEnemyManager->fReserveDeleteEnemies();
    mMoveBegin = mPosition;
    mMoveEnd = { 0.0f,0.0f,600.0f };
    mMoveThreshold = 0.0f;
    mOrientation = { 0.0f,1.0f,0.0f,0.0f };
    mpModel->play_animation(mAnimPara, AnimationName::ship_beam_shot_start);
}

void LastBoss::fShipBeamStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mMoveThreshold += elapsedTime_ * 5.0f;
    mMoveThreshold = (std::min)(1.1f, mMoveThreshold);
    mPosition = Math::lerp(mMoveBegin, mMoveEnd, mMoveThreshold);

    // アニメーション終了と同時に遷移
    if (mpModel->end_of_animation(mAnimPara) && mMoveThreshold >= 1.0f)
    {
        effect_manager->finalize();
        effect_manager->initialize(Graphics_);
        fChangeState(DivideState::ShipBeamCharge);
    }
}

void LastBoss::fShipBeamChargeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ship_beam_charge_idle, true);
    mTimer = mkShipBeamChargeSec;

    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    mpModel->fech_by_bone(mAnimPara, world, mShipFace, mShipFacePosition, up);

    mShipPointer.fSetRadius(0.1f);
    mShipPointer.fSetColor({ 1.0f,0.0f,0.0f,1.0f });
    mShipPointer.fSetAlpha(1.0f);

    // ポインターの長さを初期化
    mPointerLength = 0.0f;

    mpBeamBaseEffect->play(effect_manager->get_effekseer_manager(), { 0.0f,0.0f,520.0f });
    mpBeamBaseEffect->set_scale(effect_manager->get_effekseer_manager(), { 15.0f,15.0f,15.0f });

    // SEを再生
    audio_manager->play_se(SE_INDEX::BOSS_CHARGE);
}

void LastBoss::fShipBeamChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // レーザーポインターを伸ばす
    mPointerLength += elapsedTime_ * 1000.0f;


    mShipPointer.fSetLengthThreshold(1.0f);
    const DirectX::XMFLOAT3 front{ Math::GetFront(mOrientation) };
    mShipPointerEnd = mShipFacePosition + (front * mPointerLength );

    // ポインターに計算された値を設定
    mShipPointer.fSetPosition(mShipFacePosition, mShipPointerEnd);

    // TODO : ここで小さめのカメラシェイク
    mTimer -= elapsedTime_;
    if(mTimer<=0.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
        // 発射
        fChangeState(DivideState::ShipBeamShoot);
    }
}

void LastBoss::fShipBeamShootInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ship_beam_shot_start);
    mTimer = mkShipBeamShootSec;

    //--------------------<レーザーポインターを消す>--------------------//
    mShipPointer.fSetAlpha(0.0f);

    //--------------------<レーザービームを発射する>--------------------//
    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    // ボーンの位置を取得
    mpModel->fech_by_bone(mAnimPara, world, mShipFace, mShipFacePosition, up);

    // ビームの長さを初期化
    mBeamLength = 0.0f;
    // ビームの攻撃を有効
    mIsAttack = true;

    // ビームの発射位置を計算
    const DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 520.0f };
    const auto normVec = Math::Normalize(mPlayerPosition - mPosition);
    float d = Math::Dot(normVec, pos - mPosition);

    mBeamEffectPosition = mPosition + (normVec * d);
    mpBeamEffect->play(effect_manager->get_effekseer_manager(), mBeamEffectPosition, 8);
    mpBeamBaseEffect->play(effect_manager->get_effekseer_manager(), mBeamEffectPosition, 8);

    // ＳＥ再生
    audio_manager->play_se(SE_INDEX::BOSS_BEAM);
}

void LastBoss::fShipBeamShootUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    const DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 520.0f };
    const auto normVec = Math::Normalize(mPlayerPosition - mPosition);
    float d = Math::Dot(normVec, pos - mPosition);
    mBeamEffectPosition = mPosition + (normVec * d);
    // エフェクトの位置を更新
    mpBeamEffect->set_position(effect_manager->get_effekseer_manager(), mBeamEffectPosition);
    mpBeamEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);

    mpBeamBaseEffect->set_position(effect_manager->get_effekseer_manager(), mBeamEffectPosition);
    mpBeamBaseEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);

    const auto world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    DirectX::XMFLOAT3 up{};
    // ボーンの位置を取得
    mpModel->fech_by_bone(mAnimPara, world, mShipFace, mShipFacePosition, up);

    // ビームの撃つ先を設定する
    const DirectX::XMFLOAT3 front = Math::GetFront(mOrientation);
    const DirectX::XMFLOAT3 beamEnd{ mShipFacePosition + (front * mBeamLength) };

    // ビームの長さを更新する
    mBeamLength += elapsedTime_ * 1000.0f;


    // 攻撃capsuleを設定
    mAttackCapsule.mBottom = mShipFacePosition;
    mAttackCapsule.mTop = beamEnd;
    mAttackCapsule.mRadius = 13.0f;


    fTurnToPlayer(elapsedTime_,6.0f);
    // TODO : ここで激しいカメラシェイク
    mTimer -= elapsedTime_;
    if(mTimer<=0.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
        fChangeState(DivideState::ShipBeamEnd);
        mIsAttack = false;
        mBeamLength = 0.0f;
    }
}

void LastBoss::fShipBeamEndInit()
{

}

void LastBoss::fShipBeamEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    effect_manager->finalize();
    effect_manager->initialize(Graphics_);
    fChangeState(DivideState::ShipToHuman);
}

void LastBoss::fChangeShipToHumanInit()
{
    audio_manager->stop_all_se();
    mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    mpModel->play_animation(mAnimPara, AnimationName::ship_to_human);
    mDissolve = 0.0f;
    mCurrentMode = Mode::ShipToHuman;
    mPosition = { 0.0f,20.0f,0.0f };
    PostEffect::boss_awakening_effect({ 0.0f,0.0f }, 0.0f, 0.25f);

    mpEnemyManager->fReserveDeleteEnemies();

    // フラグの配列を初期化
    for (int i = 0; i < ARRAYSIZE(mSeArrayShipToHuman); ++i)
    {
        mSeArrayShipToHuman[i] = false;
    }
    mTimer = 0.0f;
    mSkipTimer = 0.0f;
    if (mBossParam.mShowMovie_ShipToHuman)
    {
        mDrawSkip = true;
    }
}

void LastBoss::fChangeShipToHumanUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    bool end{ false };

    if (game_pad->get_button() & GamePad::BTN_B)
    {
        mSkipTimer += elapsedTime_ * 0.3f;
    }
    
    mTimer += elapsedTime_;
    // SEを鳴らす
    if (mTimer >= 0.9f && mSeArrayShipToHuman[0] == false) // 57
    {
        audio_manager->play_se(SE_INDEX::BOSS_SMALL_ROAR);
        mSeArrayShipToHuman[0] = true;
    }
    if (mTimer >= 2.9f && mSeArrayShipToHuman[1] == false) // 
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayShipToHuman[1] = true;
    }
    if (mTimer >= 3.5f && mSeArrayShipToHuman[2] == false) // 
    {
        audio_manager->play_se(SE_INDEX::FOOT_TRANSFORM);
        mSeArrayShipToHuman[2] = true;
    }
    if (mTimer >= 5.08f && mSeArrayShipToHuman[3] == false) // 
    {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        mSeArrayShipToHuman[3] = true;
    }
    if (mTimer >= 5.2f && mSeArrayShipToHuman[4] == false) // 
    {
        audio_manager->play_se(SE_INDEX::REVERBERATION);
        mSeArrayShipToHuman[4] = true;
    }

    if (mpModel->end_of_animation(mAnimPara)||mSkipTimer>=1.0f)
    {
        end = true;
    }
    if (end)
    {
        PostEffect::clear_post_effect();
        fChangeState(DivideState::HumanIdle);

        // 現在のモードを人型に変更する
        mCurrentMode = Mode::Human;
        mPosition = {};

        // アニメーションを見たフラグをオンにする
        mBossParam.mShowMovie_ShipToHuman = true;
        mBossParam.BossStateNumber = 1;
    }
}

void LastBoss::fHumanIdleInit()
{
    mDrawSkip = false;
    mpModel->play_animation(mAnimPara, AnimationName::human_idle, true, true, 0.3f, 1.5f);
    mAnimationSpeed = 1.5f;
    mTimer = 2.0f;
}

void LastBoss::fHumanIdleUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);

   // 条件に応じて攻撃手段を分岐させる

    mTimer -= elapsedTime_;
    if (mTimer >= 0.0f)
    {
        return;
    }

    //--------------------<アニメーションが終了したら>--------------------//

     // 体力が９割を下回ったら以下きめの必殺技
    if (!mFirstSp && fComputePercentHp() < 0.7f)
    {
        fChangeState(DivideState::HumanSpAway);
        mFirstSp = true;
        mAnimationSpeed = 1.0f;
        return;
    }

    // ステージの限界値を反映
    if(fLimitStageHuman(elapsedTime_))
    {
        return;
    }

    // プレイヤーとの距離が近かったら範囲爆発攻撃を多くする
    const std::uniform_int_distribution<int> RandTargetAdd(0, 9);
    const int randNumber = RandTargetAdd(mt);
    // 遠い
    if (Math::Length(mPlayerPosition - mPosition) > mkDistanceToPlayer*10.0f)
    {
        // さらに距離が遠かったら
        if (randNumber <= 3)
        {
            fChangeState(DivideState::HumanRush);
            mAnimationSpeed = 1.0f;
            return;
        }
    }

    if(Math::Length(mPlayerPosition-mPosition)>mkDistanceToPlayer)
    {
        if(randNumber>=8)
        {
            fChangeState(DivideState::HumanBlowAttack);
            mAnimationSpeed = 1.0f;
            return;
        }
        else  if(randNumber>=6)
        {
            fChangeState(DivideState::HumanAllShot);
            mAnimationSpeed = 1.0f;
            return;
        }
        else if(randNumber>=1)
        {
            fChangeState(DivideState::HumanRush);
            mAnimationSpeed = 1.0f;
            return;
        }
        else
        {
            fChangeState(DivideState::HumanRush);
            mAnimationSpeed = 1.0f;
            return;
        }
    }
    else
    {
        if (randNumber > 6)
        {
            fChangeState(DivideState::HumanBlowAttack);
            mAnimationSpeed = 1.0f;
            return;
        }
        else if(randNumber>2)
        {
            fChangeState(DivideState::HumanAllShot);
            mAnimationSpeed = 1.0f;
            return;
        }
        else
        {
            fChangeState(DivideState::HumanMove);
            mAnimationSpeed = 1.0f;
            return;
        }
    }
}

void LastBoss::fHumanMoveInit()
{

}

void LastBoss::fHumanMoveUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{

}

void LastBoss::fHumanAllShotInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_bullet);
    mTimer = 0.0f;
}

void LastBoss::fHumanAllShotUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{

    fTurnToPlayer(elapsedTime_, 10.0f);

    // 着弾基準点を設定
    DirectX::XMFLOAT3 position{ mPlayerPosition };

    //--------------------<銃口の向きに発射>--------------------//
    mTimer += elapsedTime_;
    if (mTimer < mkHumanAllShotEnd&& mTimer > mkHumanAllShotBegin)
    {
        // ボーンが向いている方向を計算
        DirectX::XMFLOAT3 up{};
        DirectX::XMFLOAT3 leftPosition{};
        DirectX::XMFLOAT4X4 leftRotMat{};

        mpModel->fech_by_bone(mAnimPara, Math::calc_world_matrix(mScale,
            mOrientation, mPosition), mTurretBoneLeft, leftPosition,
            up, leftRotMat);
        const DirectX::XMFLOAT3 leftFront  =
              { leftRotMat._31,leftRotMat._32, leftRotMat._33 };

        // ボーンから着弾点までのベクトル

        // ボーンとのなす角を算出
        DirectX::XMFLOAT3 v = leftPosition - position;
        v.y = 0.0f;
        v = Math::Normalize(v);
        const DirectX::XMFLOAT3 front =
            Math::Normalize(mPlayerPosition - mPosition);
        float dot = Math::Dot(v, front);

        // ターゲットの位置を基準とした乱数で着弾点を決める
        const std::uniform_real_distribution<float> RandTargetAdd(-70.0f, -30.0f);
        const std::uniform_int_distribution<int> switcher(0, 1);

        DirectX::XMFLOAT3 bulletPosition{};

        bulletPosition.x = RandTargetAdd(mt) * switcher(mt) == 0 ? 1.0f : -1.0f;
        bulletPosition.z = RandTargetAdd(mt) * switcher(mt) == 0 ? 1.0f : -1.0f;
        bulletPosition += position;

        const DirectX::XMFLOAT3 bulletVec{
          Math::Normalize(bulletPosition - leftPosition) };

        const float bulletSpeed = Math::Length(mPlayerPosition - mPosition);


        // 一定間隔で発射
        if (mShotTimer <= 0.0f)
        {
            audio_manager->play_se(SE_INDEX::CANNON);
            mfAddBullet(new CannonballBullet(Graphics_,bulletVec ,
               180.0f, leftPosition));
            mShotTimer = mkHumanAllShotDelay;
        }
        else
        {
            mShotTimer -= elapsedTime_;
        }
    }

    if(mpModel->end_of_animation(mAnimPara))
    {

        fChangeState(DivideState::HumanIdle);
    }
}


void LastBoss::fHumanBlowAttackInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_shockwave);
    mpAllAttackEffect->play(effect_manager->get_effekseer_manager(),
        mPosition, 10.0f);
    mIsAttack = true;
    mAttackCapsule.mRadius = 0.0f;
    mTimer = 0.0f;
}

void LastBoss::fHumanBlowAttackUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);

    mTimer += elapsedTime_;

    if (mTimer > mkTimerBlow)
    {
        constexpr DirectX::XMFLOAT3 capsuleLength{ 0.0f,20.0f,0.0f };
        mAttackCapsule.mRadius += elapsedTime_ * 80.0f;
        mAttackCapsule.mTop = mPosition + capsuleLength;
        mAttackCapsule.mBottom = mPosition - capsuleLength;
    }


    if(mpModel->end_of_animation(mAnimPara))
    {
        const std::uniform_int_distribution<int> RandTargetAdd(0, 9);
        if (const auto num = RandTargetAdd(mt); num < 4)
        {
            fChangeState(DivideState::HumanMove);
            mIsAttack = false;
            mAttackCapsule.mRadius = 0.0f;
        }
        else
        {
            fChangeState(DivideState::HumanIdle);
            mIsAttack = false;
            mAttackCapsule.mRadius = 0.0f;
        }
    }
}

void LastBoss::fMoveAwayInit()
{
    mMoveBegin = mPosition;
    const auto normalV = Math::Normalize(mPosition - mPlayerPosition);
    mMoveEnd = mPosition + normalV * 100.0f;
    mMoveThreshold = 0.0f;
}

void LastBoss::fMoveAwayUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mMoveThreshold += elapsedTime_ * 3.0f;
    mPosition = Math::lerp(mMoveBegin, mMoveEnd, mMoveThreshold);

    if(mMoveThreshold>=1.0f|| Math::Length(mPosition) > mkLimitStage)
    {
        fChangeState(DivideState::HumanIdle);
    }
}

void LastBoss::fHumanRushInit()
{
    // 移動目標を設定
    mMoveEnd = mPlayerPosition;
    mMoveBegin = mPosition;
    mMoveThreshold = 0.0f;
    mpModel->play_animation(mAnimPara, AnimationName::human_rush_idle);
    audio_manager->play_se(SE_INDEX::BOSS_RUSH);
}

void LastBoss::fHumanRushUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    const float limitLength = Math::Length(mMoveBegin - mMoveEnd);
    const float currentLength = Math::Length(mPosition - mMoveBegin);

    // 終了条件
    if (currentLength > limitLength * 1.2f)
    {

        fChangeState(DivideState::HumanIdle);
        return;
    }

    // プレイヤーの方向に移動するベクトルを作成する

    const DirectX::XMFLOAT3 iniVec{
        Math::Normalize(mMoveEnd - mMoveBegin) };
    const DirectX::XMFLOAT3 currentVec{
        Math::Normalize(mPlayerPosition - mMoveBegin) };

    DirectX::XMFLOAT3 endPoint{};
    {
        const float d = Math::Dot(currentVec, mMoveEnd - mMoveBegin);
        const DirectX::XMFLOAT3 end = mMoveBegin + (currentVec * d);
        endPoint=Math::lerp(mMoveBegin, end, mMoveThreshold);
    }


    mMoveThreshold += elapsedTime_ * 0.5f;


    const DirectX::XMFLOAT3 prePosition = mPosition;
    // スプライン曲線でベクトルを取得
    mPosition=Math::fBezierCurve(mMoveBegin, mMoveEnd, endPoint,mMoveThreshold);

    const DirectX::XMFLOAT3 subVec = Math::Normalize(mPosition - prePosition);

    fTurnToTarget(elapsedTime_, 10.0f, mPosition + subVec);

    if(mMoveThreshold>=1.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_RUSH);
        fChangeState(DivideState::HumanBlowAttack);
    }
}

void LastBoss::fHumanSpAttackAwayInit()
{
    mPosition.y = 6.0f;

   // 飛びのく地点をセット
    mAwayBegin = mPosition;
    mAwayLerp = 0.0f;

    mBeginOrientation = mOrientation;

    mpModel->play_animation(mAnimPara, AnimationName::human_to_ship_quick);
}

void LastBoss::fHumanSpAttackAwayUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    // 正面を向くように回転
    if(mpModel->end_of_animation(mAnimPara))
    {
        mAwayLerp += elapsedTime_ * 2.0f;
        mAwayLerp = (std::min)(1.0f, mAwayLerp);

        // 移動
        mPosition = Math::lerp(mAwayBegin, mAwayEnd, mAwayLerp);
        mOrientation = Math::lerp(mBeginOrientation, mEndOrientation, mAwayLerp);
    }

    if(mAwayLerp>=1.0f)
    {
        fChangeState(DivideState::HumanSpWait);
        fSpawnChildUnit(Graphics_, 6);
    }
}

void LastBoss::fHumanSpAttackSummonInit()
{
    throw std::logic_error("Not implemented");
}

void LastBoss::fHumanSpAttackSummonUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}


void LastBoss::fHumanSpAttackWaitInit()
{
    mTimer = 0.0f;
}

void LastBoss::fHumanSpAttackWaitUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);
    mTimer += elapsedTime_;
    if(mTimer>=mkHumanSpWaitTime)
    {
        fChangeState(DivideState::HumanSpOver);
    }

    if(mpEnemyManager->fGetEnemyCounts()<=4)
    {
        fChangeState(DivideState::HumanSpDamage);
    }
}

void LastBoss::fHumanSpAttackCancelInit()
{

}

void LastBoss::fHumanSpAttackCancelUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

}

void LastBoss::fHumanSpAttackTimeOverInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::ship_to_human_quick);
}

void LastBoss::fHumanSpAttackTimeOverUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::HumanSpCharge);
    }
}

void LastBoss::fHumanSpAttackChargeInit()
{
    mAnimationSpeed = 0.7f;
    mpModel->play_animation(mAnimPara, AnimationName::human_beam_charge, true);
    mTimer = mkSpChargeTime;

    // laserを初期化
    mRightPointer.fSetRadius(0.1f);  // 半径
    mLeftPointer.fSetRadius(0.1f);

    mRightPointer.fSetColor({1.0f,0.0f,0.0f,1.0f}); // 色
    mLeftPointer. fSetColor({1.0f,0.0f,0.0f,1.0f});

    mRightPointer.fSetLengthThreshold(1.0f); // 閾値
    mLeftPointer. fSetLengthThreshold(1.0f);

    mRightPointer.fSetAlpha(1.0f); // 不透明度
    mLeftPointer. fSetAlpha(1.0f);

    mHumanBeamTarget = mPosition;

    audio_manager->play_se(SE_INDEX::BOSS_CHARGE);
}

void LastBoss::fHumanSpAttackChargeUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;


    // ポインターをプレイヤーの位置に回転させる
    mHumanBeamTarget = Math::lerp(mHumanBeamTarget, mPlayerPosition,
        30.0f * elapsedTime_);

    mRightPointer.fSetPosition(mpTurretRight->fGetPosition(), mHumanBeamTarget);
    mLeftPointer.fSetPosition(mpTurretLeft->fGetPosition(), mHumanBeamTarget);
    fTurnToPlayer(elapsedTime_, 5.0f);



    if (mTimer <= 0.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
        fChangeState(DivideState::HumanSpShoot);
        mAnimationSpeed = 1.0f;
    }
}

void LastBoss::fHumanSpBeamShootInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_beam_shoot);
    mTimer = 0.0f;

    mRightPointer.fSetAlpha(0.0f);
    mLeftPointer.fSetAlpha(0.0f);

    mAwayBegin = mPosition;
    mAwayLerp = 0.0f;

    mpBeamRightEffect->play(effect_manager->get_effekseer_manager(), mpTurretRight->fGetPosition());
    mpBeamRightEffect->set_scale(effect_manager->get_effekseer_manager(), { 5.0f,5.0f,5.0f });
    mpBeamLeftEffect->play(effect_manager->get_effekseer_manager(), mpTurretLeft->fGetPosition());
    mpBeamLeftEffect->set_scale(effect_manager->get_effekseer_manager(), { 5.0f,5.0f,5.0f });

    mIsAttack = true;

    audio_manager->play_se(SE_INDEX::BOSS_BEAM);
}

void LastBoss::fHumanSpBeamShootUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 5.0f);

    auto fRotBeamEffect = [=](const DirectX::XMFLOAT4 Orientation_,
        DirectX::XMFLOAT3 Begin_)->DirectX::XMFLOAT4
    {

        // 回転を算出
        // 現在の上ベクトル
        DirectX::XMFLOAT3 up{ 0.001f,0.0f,1.0f };
        up = Math::Normalize(up);
        // 終点とのベクトル
        DirectX::XMFLOAT3 cylinderUp = { mHumanBeamTarget - Begin_ };
        cylinderUp = Math::Normalize(cylinderUp);

        // 外積で回転軸を算出
        auto cross = Math::Cross(up, cylinderUp);
        cross = Math::Normalize(cross);
        // 長さが０の時クラッシュ回避のため仮の値を代入
        if (Math::Length(cross) <= 0.0f)
        {
            cross = { 0.0f,1.0f,0.0f };
        }
        // 内積で回転角を算出
        auto dot = Math::Dot(up, cylinderUp);
        dot = acosf(dot);
        DirectX::XMFLOAT4 dummy{ 0.0f,0.0f,0.0f,1.0f };
        auto rotQua = Math::RotQuaternion(dummy, cross, dot);

        // 補完
        auto res = DirectX::XMQuaternionSlerp(
            DirectX::XMLoadFloat4(&Orientation_),
            DirectX::XMLoadFloat4(&rotQua),
            1.0f);

        DirectX::XMFLOAT4 ret{};
        DirectX::XMStoreFloat4(&ret, res);
        return ret;
    };

    mBeamRightOrientation = fRotBeamEffect(mBeamRightOrientation, mpTurretRight->fGetPosition());
    mBeamLeftOrientation = fRotBeamEffect(mBeamLeftOrientation, mpTurretLeft->fGetPosition());

    mpBeamLeftEffect->set_position(effect_manager->get_effekseer_manager(), mpTurretLeft->fGetPosition());
    mpBeamLeftEffect->set_quaternion(effect_manager->get_effekseer_manager(), mBeamLeftOrientation);

    mpBeamRightEffect->set_position(effect_manager->get_effekseer_manager(), mpTurretRight->fGetPosition());
    mpBeamRightEffect->set_quaternion(effect_manager->get_effekseer_manager(), mBeamRightOrientation);


    mHumanBeamTarget = Math::lerp(mHumanBeamTarget, mPlayerPosition,
        3.0f * elapsedTime_);

    mAttackCapsule.mBottom = mPosition;
    mAttackCapsule.mTop = mHumanBeamTarget;
    mAttackCapsule.mRadius = 7.0f;

    mTimer += elapsedTime_;
    if(mTimer>mkHumanSpBeamTime)
    {
        if (mIsAttack)
        {
            audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
            mpBeamRightEffect->stop(effect_manager->get_effekseer_manager());
            mpBeamLeftEffect->stop(effect_manager->get_effekseer_manager());
            mIsAttack = false;
        }
        mPosition = Math::lerp(mAwayBegin, { 0.0f,0.0f,0.0f }, mAwayLerp);
        mAwayLerp += elapsedTime_ * 2.0f;
    }
    else
    {

    }
    if (mAwayLerp > 1.0f)
    {

        fChangeState(DivideState::HumanIdle);
    }
}

void LastBoss::fHumanSpDamageInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_damage);
    mAwayBegin = mPosition;
    mAwayLerp = 0.0f;
}

void LastBoss::fHumanSpDamageUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        mPosition = Math::lerp(mAwayBegin, { 0.0f,0.0f,0.0f }, mAwayLerp);
        mAwayLerp += elapsedTime_ * 2.0f;

    }
    if (mAwayLerp > 1.0f)
    {
        // ダメージを与える
        mCurrentHitPoint -= (mMaxHp * 0.1f);
        fChangeState(DivideState::HumanIdle);
    }
}


void LastBoss::fHumanDieStartInit()
{
    mPosition.z = 0.0f;
    // 位置をリセット
    mpModel->play_animation(mAnimPara, AnimationName::human_die);
    // TODO : エフェクトの類をすべてリセットする

    // TODO : カメラをボスの方に向ける
    if (mBossParam.mShowMovie_HumanToDragon)
    {
        mDrawSkip = true;
    }
}

void LastBoss::fHumanDieStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    bool end{ false };
    if (game_pad->get_button() & GamePad::BTN_B)
    {
        end = true;
    }

    mPosition = Math::lerp(mPosition, { 0.0f,80.0f,0.0f }, 10.0f * elapsedTime_);
   if(mpModel->end_of_animation(mAnimPara))
   {
       end = true;
   }
   if (end)
   {
       effect_manager->finalize();
       effect_manager->initialize(Graphics_);
       fChangeState(DivideState::HumanDieMiddle);
   }
}

void LastBoss::fHumanDieMiddleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::human_die_idle, true);
    mTimer = mkHumanDieIdleSec;
    mRgbColorPower = 0.0f;
    mHeartTimer = 0.0f;
    mPerformThresold = 0.0f;
}

void LastBoss::fHumanDieMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    bool end{ false };
    if (game_pad->get_button() & GamePad::BTN_B)
    {
        end = true;
    }


    // RGBずらしで心臓のどくどくを演出する
    mTimer -= elapsedTime_;
    mPerformThresold += elapsedTime_ * 0.5f;
    mPerformThresold = (std::min)(0.15f, mPerformThresold);
    PostEffect::boss_awakening_effect({ 0.5f,0.5f }, mRgbColorPower, mPerformThresold);
    if (mHeartTimer <= 0.0f)
    {
        mRgbColorPower += mRgbColorSpeed * elapsedTime_;
        if (mRgbColorPower > 1.0f)
        {
            mRgbColorSpeed *= -1.0f;
            audio_manager->play_se(SE_INDEX::HEART_BEAT);
        }
        if (mRgbColorPower < 0.0f)
        {
            //  一周したら待機する
            mHeartTimer = mkWaitHeartEffect;
            mRgbColorSpeed *= -1.0f;
        }
    }
    else
    {
        mHeartTimer -= elapsedTime_;
    }

    if (mTimer <= 0.0f)
    {
        end = true;
    }
    if (end)
    {

        effect_manager->finalize();
        effect_manager->initialize(Graphics_);
        fChangeState(DivideState::HumanToDragon);
        PostEffect::clear_post_effect();
    }
}

void LastBoss::fHumanToDragonInit()
{
    audio_manager->stop_all_se();
    mpModel->play_animation(mAnimPara,AnimationName::human_to_dragon);
    mDissolve = 0.0f;
    mCurrentMode = Mode::HumanToDragon;
    mPosition = { 0.0f,40.0f,0.0f };
    for (int i = 0; i < ARRAYSIZE(mSeArrayHumanToDragon); ++i)
    {
        mSeArrayHumanToDragon[i] = false;
    }
    mAnimationSpeed = 1.0f;
    mTimer = 0.0f;
    mIsStun = false;
    mStunEffect->stop(effect_manager->get_effekseer_manager());
    mSkipTimer = 0.0f;
}

void LastBoss::fHumanToDragonUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    bool end{ false };
    if (game_pad->get_button() & GamePad::BTN_B)
    {
        mSkipTimer += elapsedTime_ * 0.3f;
    }



    mTimer += elapsedTime_;
    int i = 0;

    if (mTimer >= 9.76f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::TEARING);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 11.0f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::THROW);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 14.66f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::ROAR_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 21.63f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 21.89f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 22.10f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::BOSS_READY_FAST);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 22.63f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_2);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 23.18f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DOCKING_1);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 23.38f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::DISC);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 24.00f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::SPLINTERS);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 26.23f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::BOSS_READY_FAST);
        mSeArrayHumanToDragon[i] = true;
    }
    ++i;
    if (mTimer >= 27.28f && mSeArrayHumanToDragon[i] == false)
    {
        audio_manager->play_se(SE_INDEX::ROAR_2);
        mSeArrayHumanToDragon[i] = true;
    }

    if (mpModel->end_of_animation(mAnimPara) || mSkipTimer >= 1.0f)
    {
        end = true;
    }
    if (end)
    {
        mDrawSkip = false;
        fChangeState(DivideState::DragonHideStart);
        mCurrentMode = Mode::Dragon;
        // 体力の値をドラゴンモードの初期値にしておく
        mCurrentHitPoint = mMaxHp * mkPercentToDragon;
        mPosition = {};

        // アニメーションを見たフラグをオンにする
        mBossParam.mShowMovie_HumanToDragon = true;
        mBossParam.BossStateNumber = 2;
    }
}

void LastBoss::fDragonIdleInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_idle, true);
    mTimer = 2.0f;
    mCurrentMode = Mode::Dragon;
    mDragonBreathCount = 0;
    mAnimationSpeed = 5.0f;
}

void LastBoss::fDragonIdleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;
   if(mTimer>=0.0f)
   {
       return;
   }

    const std::uniform_int_distribution<int> RandTargetAdd(0, 9);
    const int randNumber = RandTargetAdd(mt);

    if(randNumber>8)
    {
        fChangeState(DivideState::DragonIdle);
    }
    else if (randNumber > 6)
    {
        fChangeState(DivideState::DragonRushHide);
    }
    else if(randNumber>2)
    {
        fChangeState(DivideState::DragonBeamStart);
    }
    else
    {
        fChangeState(DivideState::DragonHideStart);
    }
    mAnimationSpeed = 1.0f;
}

void LastBoss::fDragonFastBreathStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_hide);
    mTimer = mkDragonHideTime;
    mDissolve = 0.0f;
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpSecondGunLeft->fSetDissolve (mDissolve);
    mpTurretLeft->fSetDissolve(mDissolve);
    mpTurretRight->fSetDissolve(mDissolve);
}

void LastBoss::fDragonFastBreathStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    mTimer -= elapsedTime_;
    mDissolve += elapsedTime_ * 1.2f;
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpSecondGunLeft->fSetDissolve(mDissolve);
    if(mTimer<0.0f)
    {
        // ディゾルブ終了
        fChangeState(DivideState::DragonAppear);
        mDissolve = 1.0f;
        mpSecondGunRight->fSetDissolve(mDissolve);
        mpSecondGunLeft->fSetDissolve(mDissolve);
        mpTurretLeft->fSetDissolve(mDissolve);
        mpTurretRight->fSetDissolve(mDissolve);
    }
}

void LastBoss::fDragonBreathAppearInit()
{
    // 位置を移動させる

    int areaSeed{};
    // ステージを四分割して今どこにいるか判定（下図はエリア番号）

    /////////////////////////
    //         |           //
    //     0   |     1     //
    //         |           //
    //---------|-----------//
    //         |           //
    //     2   |     3     //
    //         |           //
    /////////////////////////

    if(mPosition.x>0.0f)
    {
        areaSeed++;
    }
    if(mPosition.z<0.0f)
    {
        areaSeed += 2;
    }

    std::vector<int> vec{};
    vec.reserve(3);
    for(int i=0;i<4;++i)
    {
        if (i == areaSeed) continue;;
        vec.emplace_back(i);
    }
    const std::uniform_int_distribution<int> RandTargetAdd(0, 2);

    // 目標地点のインデックスを取得
    const int areaIndex = vec.at(RandTargetAdd(mt));

    float rad = DirectX::XMConvertToRadians(45.0f);
    switch (areaIndex)
    {
    case 0:
        mPosition = { -cosf(rad),0.0f,sinf(rad) };
        break;
    case 1 :
        mPosition = { cosf(rad),0.0f,sinf(rad) };
        break;
    case 2:
        mPosition = { -cosf(rad),0.0f,-sinf(rad) };
        break;
    case 3:
        mPosition = { cosf(rad),0.0f,-sinf(rad) };
        break;
    default: ;
    }

    mPosition *= (mkLimitStage * 0.5f * 0.7f);

    mDissolve = 1.0f;

    }

void LastBoss::fDragonBreathAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    mDissolve -= elapsedTime_ * 1.3f;
    if(mDissolve<0.0f)
    {
        fChangeState(DivideState::DragonBreathCharge);
    }
}

void LastBoss::fDragonBreathChargeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_breath_ready, false, true, 0.3f, 0.5f);
}

void LastBoss::fDragonBreathChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);

    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::DragonBreathShot);
    }
}

void LastBoss::fDragonBreathShotInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_breath_start);
    mDragonBreathCount++;
    mTimer = 0.0f;
    mIsShotBreath = false;
}

void LastBoss::fDragonBreathShotUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if(mIsShotBreath==false)
    {
        auto p = mPosition;
        p.y += 30.0f;
        audio_manager->play_se(SE_INDEX::FIRE_BOLL);
        mfAddBullet(new DragonBreath(Graphics_, p, 200.0f, mPlayerPosition));
        mIsShotBreath = true;
    }

    if(mpModel->end_of_animation(mAnimPara))
    {
        // 確率でもう一度撃つ（最大三回）
        const std::uniform_int_distribution<int> RandTargetAdd(0, 9);
        const int randNumber = RandTargetAdd(mt);
        if (std::pow(mDragonBreathCount, 2) < randNumber)
        {
            fChangeState(DivideState::DragonHideStart);
        }
        else
        {
            fChangeState(DivideState::DragonIdle);
        }
    }
}

void LastBoss::fDragonRushHideInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_hide);
    mDissolve = 0.0f;
}

void LastBoss::fDragonRushHideUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve += elapsedTime_;
    mpSecondGunLeft->fSetDissolve(mDissolve);
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpTurretLeft->fSetDissolve(mDissolve);
    mpTurretRight->fSetDissolve(mDissolve);

    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::DragonRushWait);
        // ロックオンされないように遠くに移動させる
        mPosition = { 0.0f,500.0f,0.0f };
    }

}

void LastBoss::fDragonRushWaitInit()
{
    mTimer = mkDragonRushWaitTime;
}

void LastBoss::fDragonRushWaitUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{

    mTimer -= elapsedTime_;
    // 一秒ごとに出現させる
    if(mTimer<=3.5f&&mRushVec[0]->fGetIsStart()==false)
    {
        constexpr float rad = { DirectX::XMConvertToRadians(0.0f) };
        constexpr float length = 200.0f * 0.7f;
        mRushVec[0]->fStartAppear(
            {
                cosf(rad) * length,0.0f,sinf(rad) * length
            });
    }
    if (mTimer <= 2.0f && mRushVec[1]->fGetIsStart() == false)
    {
        constexpr float rad = { DirectX::XMConvertToRadians(120.0f) };
        constexpr float length = 200.0f * 0.7f;
        mRushVec[1]->fStartAppear(
            {
                cosf(rad)*length,0.0f,sinf(rad)*length
            });
    }
    if (mTimer <= 0.5f && mRushVec[2]->fGetIsStart() == false)
    {
        constexpr float rad = { DirectX::XMConvertToRadians(240.0f) };
        constexpr float length = 200.0f * 0.7f;
        mRushVec[2]->fStartAppear(
            {
                cosf(rad) * length,0.0f,sinf(rad) * length
            });
    }

    if(mTimer<-2.0f)
    {
        fChangeState(DivideState::DragonRushAppear);
    }
}

void LastBoss::fDragonRushAppearInit()
{
    mPosition = { 0.0f,0.0f,0.0f };
    mOrientation = { 0.0f,0.0f,0.0f,1.0f };
    mpModel->play_animation(mAnimPara, AnimationName::dragon_idle);
    mDissolve = 1.0f;
}

void LastBoss::fDragonRushAppearUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve -= elapsedTime_;
    mpSecondGunLeft->fSetDissolve(mDissolve);
    mpSecondGunRight->fSetDissolve(mDissolve);
    mpTurretLeft->fSetDissolve(mDissolve);
    mpTurretRight->fSetDissolve(mDissolve);
    mPosition = { 0.0f,0.0f,0.0f };
    if(mDissolve<0.0f)
    {
        fChangeState(DivideState::DragonIdle);
    }
}

void LastBoss::fDragonBeamMoveInit()
{
    mDragonMoveThreshold = 0.0f;
    mMoveBegin = mPosition;
    mpModel->play_animation(mAnimPara, AnimationName::dragon_idle, true);
}

void LastBoss::fDragonBeamMoveUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
   // 中央まで移動
    mDragonMoveThreshold += elapsedTime_;
    mPosition = Math::lerp(mMoveBegin, { 0.0f,0.0f,0.0f }, mDragonMoveThreshold);

    if(mDragonMoveThreshold >=1.0f)
    {
        fChangeState(DivideState::DragonBeamStart);
    }
}

void LastBoss::fDragonBeamStartInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_beam_ready);

}

void LastBoss::fDragonBeamStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::DragonBeamCharge);
    }
}

void LastBoss::fDragonBeamChargeInit()
{
    mpModel->play_animation(mAnimPara, AnimationName::dragon_beam_charge);
    mTimer = mkDragonBeamChargeTime;
    audio_manager->play_se(SE_INDEX::BOSS_CHARGE);
}

void LastBoss::fDragonBeamChargeUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    fTurnToPlayer(elapsedTime_, 10.0f);
    mTimer -= elapsedTime_;
    if (mTimer < 0.0f)
    {
        audio_manager->stop_se(SE_INDEX::BOSS_CHARGE);
        fChangeState(DivideState::DragonBeamShoot);
    }
}
void LastBoss::fDragonBeamShotInit()
{
    // 現在の各祖を求める

    auto front = Math::Normalize(Math::GetFront(mOrientation));
    DirectX::XMFLOAT3 worldFront = { 0.0f,0.0f,1.0f };
    float dot = Math::Dot(front, worldFront);
    mStartBeamRadian = acosf(dot);
    mAddRadian = 0.0f;
    mIsAttack = true;
    mBeamLength = 0.0f;
    mpBeamEffect->play(effect_manager->get_effekseer_manager(), mPosition);
    mpBeamEffect->set_scale(effect_manager->get_effekseer_manager(), { 10.0f,10.0f,10.0f });
    audio_manager->play_se(SE_INDEX::BOSS_BEAM);
    mAttackPower = 10;
}

void LastBoss::fDragonBeamShotUpdate(float elapsedTime_,
    GraphicsPipeline& Graphics_)
{
    // ビームを回転させる
    mpBeamEffect->set_quaternion(effect_manager->get_effekseer_manager(), mOrientation);
    DirectX::XMFLOAT3 pos = mPosition + (Math::GetFront(mOrientation) * 20.0f);
    mpBeamEffect->set_position(effect_manager->get_effekseer_manager(), pos);

    mAddRadian += DirectX::XMConvertToRadians(40.0f * elapsedTime_);
    const float radian = mStartBeamRadian + mAddRadian;
    const DirectX::XMFLOAT3 beamFront = { cosf(radian),0.0f,sinf(radian) };

    mBeamLength += elapsedTime_ * 500.0f;

    mAttackCapsule.mBottom = pos;
    mAttackCapsule.mTop =
        pos + (Math::GetFront(mOrientation) * mBeamLength);
    mAttackCapsule.mRadius = 20.0f;
    fTurnToTarget(elapsedTime_, 10.0f, mPosition + beamFront * 20.0f);


    float rotMax = 500.0f * (1.0f - static_cast<float>(mCurrentHitPoint / mMaxHp));

    if (mAddRadian >= DirectX::XMConvertToRadians(rotMax))
    {
        mIsAttack = false;
        audio_manager->stop_se(SE_INDEX::BOSS_BEAM);
        mpBeamEffect->stop(effect_manager->get_effekseer_manager());
        fChangeState(DivideState::DragonIdle);
    }
}


void LastBoss::fDragonBeamEndInit()
{
    throw std::logic_error("Not implemented");
}

void LastBoss::fDragonBeamEndUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    throw std::logic_error("Not implemented");
}

void LastBoss::fDragonDieStartInit()
{
    audio_manager->stop_all_se();
    // 位置をリセット
    mPosition = { 0.0f,40.0f,50.0f };
    mpModel->play_animation(mAnimPara, AnimationName::dragon_die);
    // TODO カメラをボスに注目させる
    mCurrentMode = Mode::DragonDie;
    mAnimationSpeed = 1.0f;
    mDissolve = 0.0f;
}

void LastBoss::fDragonDieStartUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    if(mpModel->end_of_animation(mAnimPara))
    {
        fChangeState(DivideState::DragonDieEnd);
    }
}

void LastBoss::fDragonDieMiddleInit()
{
    mDissolve = 0.0f;
}

void LastBoss::fDragonDieMiddleUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mDissolve += elapsedTime_;
    if(mDissolve>=1.0f)
    {
        mIsAlive = false;
        BaseEnemy::fDie(Graphics_);
        mCurrentMode = Mode::BossDieEnd;
        mpEnemyManager->fSetBossMode(mCurrentMode);
        mpDieEffect->play(effect_manager->get_effekseer_manager(), mPosition, 10.0f);
    }
}

void LastBoss::fStunInit()
{
    audio_manager->stop_all_se();
    mpBeamEffect->stop(effect_manager->get_effekseer_manager());
    mIsAttack = false;
    mAttackCapsule.mRadius = 0.0f;
    mStunEffect->play(effect_manager->get_effekseer_manager(), mPosition);
    mStunEffect->set_scale(effect_manager->get_effekseer_manager(), { 15.0f,15.0f,15.0f });
    mTimer = 3.0f;
    mpModel->pause_animation(mAnimPara);
}

void LastBoss::fStunUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    mTimer -= elapsedTime_;
    if (mTimer < 0.0f)
    {

        mIsStun = false;
        mpModel->progress_animation(mAnimPara);
        mStunEffect->stop(effect_manager->get_effekseer_manager());

        switch (mCurrentMode) {
            case Mode::None: break;
        case Mode::Ship:
            fChangeState(DivideState::ShipIdle);
            break;
        case Mode::ShipToHuman:
            break;
        case Mode::Human:
            fChangeState(DivideState::HumanIdle);
            break;
        case Mode::HumanToDragon: break;
        case Mode::Dragon:
            fChangeState(DivideState::DragonIdle);
            break;
        case Mode::DragonDie: break;
        default: ;
        }
    }
}


void LastBoss::fRender(GraphicsPipeline& graphics)
{

    SkinnedMesh::mesh_tuple cameraTuple = std::make_tuple("camera_mesh", 1);

    graphics.set_pipeline_preset(SHADER_TYPES::PBR);
    mDissolve = (std::max)(0.0f, mDissolve);
    const DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale, mOrientation, mPosition);
    const DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
    mpModel->render(graphics.get_dc().Get(), mAnimPara, world, color, mDissolve,0.0f,{1.0f,1.0f,1.0f,1.0f},0.8f,cameraTuple);



    //graphics.set_pipeline_preset(SHADER_TYPES::PBR);

    //--------------------<タレット描画>--------------------//
    // タレットの親の位置を取得する
    DirectX::XMFLOAT3 rightPosition{};
    DirectX::XMFLOAT3 leftPosition{};
    DirectX::XMFLOAT3 up{};

    mpModel->fech_by_bone(mAnimPara, world, mTurretBoneLeft, leftPosition,
        up);
    mpModel->fech_by_bone(mAnimPara, world, mTurretBoneRight, rightPosition,
        up);

    //--------------------<セカンドガン描画>--------------------//
    DirectX::XMFLOAT4X4 rightMat{};
    DirectX::XMFLOAT4X4 leftMat{};
    mpModel->fech_by_bone(mAnimPara, world, mSecondGunBoneLeft, leftMat);
    mpModel->fech_by_bone(mAnimPara, world, mSecondGunBoneRight, rightMat);

    mpSecondGunLeft->fRender(graphics, leftMat, leftPosition);
    mpSecondGunRight->fRender(graphics, rightMat, rightPosition);

    mpTurretLeft->fRender(graphics, leftMat, leftPosition);
    mpTurretRight->fRender(graphics, rightMat, rightPosition);

    //--------------------<ビームを描画>--------------------//
    mShipPointer.fRender(graphics);
    mRightPointer.fRender(graphics);
    mLeftPointer.fRender(graphics);

    if (mDrawSkip)
    {
        graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID, DEPTH_STENCIL::DEOFF_DWOFF);
        fonts->yu_gothic->Begin(graphics.get_dc().Get());
        fonts->yu_gothic->Draw(L"Bボタン長押しでスキップ", { 800.0f,600.0f }, { 0.7,0.7f }, { 1.0f,1.0f,1.0f,1.0f });
        fonts->yu_gothic->End(graphics.get_dc().Get());
        graphics.set_pipeline_preset(RASTERIZER_STATE::SOLID_COUNTERCLOCKWISE,DEPTH_STENCIL::DEON_DWON);
    }
}

bool LastBoss::fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsed_time)
{
    if(mCurrentMode==Mode::Ship)
    {
        return false;
    }

    bool ret{ false };
    if (mInvincibleTime <= 0.0f)
    {
        if (mIsLockOnOfChain)
        {
            Damage_ = 15;
        }
        mCurrentHitPoint -= Damage_;
        mInvincibleTime = InvincibleTime_;
        mDamageEffect->play(effect_manager->get_effekseer_manager(),
            mPosition, 10.0f);
        ret = true;
    }



    if (mCurrentHitPoint <= 0)
    {
        effect_manager->finalize();
        effect_manager->initialize(Graphics_);
        fChangeState(DivideState::DragonDieStart);
    }
    return ret;
}

void LastBoss::fDie(GraphicsPipeline& graphics)
{

}
