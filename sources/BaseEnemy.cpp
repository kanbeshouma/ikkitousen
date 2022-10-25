#define _WINSOCKAPI_  // windows.hを定義した際に、winsock.hを自動的にインクルードしない

#include"BaseEnemy.h"
#include "collision.h"
#include"resource_manager.h"
#include"Operators.h"
#include"NetWorkInformationStucture.h"
#include"Correspondence.h"

BaseEnemy::BaseEnemy(GraphicsPipeline& Graphics_,
                     const char* FileName_,
                     const EnemyParamPack& Param_,
                     const DirectX::XMFLOAT3& EntryPosition_,
					const wchar_t* IconFileName)
:mPosition(EntryPosition_),mCurrentHitPoint(Param_.MaxHp)
,mAttackInvTime(Param_.InvincibleTime),mMaxHp(Param_.MaxHp),mAttackPower(Param_.AttackPower)
,mStunTime(Param_.StunTime)
{
    mpModel = resource_manager->load_model_resource(Graphics_.get_device().Get(), FileName_);
    //アイコンの名前
    if (IconFileName)
    {
        mpIcon = std::make_unique<SpriteBatch>(Graphics_.get_device().Get(), IconFileName, 1);
    }

    mBodyCapsule.mRadius = Param_.BodyCapsuleRad;
    mAttackCapsule.mRadius = Param_.AttackCapsuleRad;

    mVernierEffect = std::make_unique<Effect>(Graphics_,effect_manager->get_effekseer_manager(), mkVernierPath);
    mCubeHalfSize = mScale.x * 2.5f;
    mDissolve = 1.0f;
    mIsStun = false;

    mBombEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), mkBombPath);
    mStunEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), mkStunPath);
    mDamageEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), mkDamagePath);

    mIsBoss = false;
}

BaseEnemy::BaseEnemy(GraphicsPipeline& Graphics_, const char* FileName_)
{
    mpModel = resource_manager->load_model_resource(Graphics_.get_device().Get(), FileName_);
}

BaseEnemy::~BaseEnemy()
{
    int a = 0;
  //  mVernierEffect->stop(effect_manager->get_effekseer_manager());
}

float BaseEnemy::fBaseUpdate(float elapsedTime_, GraphicsPipeline& Graphics_)
{
    //-----チェイン攻撃でロックオンされていたら速度を遅くする-----//
    if (mIsPlayerSearch)
    {
        elapsedTime_ *= 0.8f;
    }

    //-----位置の補間-----//
    if (mStartlerp)LerpPosition(elapsedTime_);

    //-----無敵時間の設定-----//
    mInvincibleTime -= elapsedTime_;
    mInvincibleTime = (std::max)(-1.0f, mInvincibleTime);

    //-----バーニアエフェクトの位置と回転の設定-----//
    fUpdateVernierEffectPos();

    //-----タプルに保存された更新処理を呼び出す-----//
    std::get<1>(mCurrentTuple)(elapsedTime_, Graphics_);

    //-----アニメーションの更新処理-----//
    mpModel->update_animation(mAnimPara, elapsedTime_ );

    //-----フラスタムカリングの判定-----//
    fComputeInCamera();

    //-----体力が無いときに死亡処理に移動-----//
    if (mCurrentHitPoint <= 0.0f)
    {
        fDie(Graphics_);
    }
    return elapsedTime_;
}

void BaseEnemy::fRender(GraphicsPipeline& Graphics_)
{
    if(mIsInCamera)
    {
        return;
    }

    Graphics_.set_pipeline_preset(SHADER_TYPES::PBR);
    mDissolve = (std::max)(0.0f, mDissolve);

    //-----ワールド行列を計算-----//
    const DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale, mOrientation, mPosition);

    //-----もしキャッシュデータが設定以下ならそのまま追加-----//
    if (transform.size() < SkinnedMesh::MAX_CASHE_SIZE) transform.emplace_back(world);
    //-----もしキャッシュデータが多かったら先頭データを削除して挿入-----//
    else if (transform.size() >= SkinnedMesh::MAX_CASHE_SIZE)
    {
        transform.erase(transform.begin());
        transform.emplace_back(world);
    }

    const DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
    mpModel->render(Graphics_.get_dc().Get(), mAnimPara, transform.at(0), color,mDissolve);
}

bool  BaseEnemy::fDamaged(int Damage_, float InvincibleTime_, GraphicsPipeline& Graphics_, float elapsedTime_)
{
    bool ret{ false };
    if(mInvincibleTime<=0.0f)
    {
        mDamageEffect->play(effect_manager->get_effekseer_manager(), mPosition);
        mCurrentHitPoint -= Damage_;
        mInvincibleTime = InvincibleTime_;
        ret = true;
    }
    ////HP���Ȃ��Ȃ��������S������
    //if (mCurrentHitPoint <= 0)
    //{
    //    fDie(Graphics_);
    //}
    return ret;
}

void BaseEnemy::fDie(GraphicsPipeline& Graphics_)
{
    if (mIsAlive == false) return;
    mIsAlive = false;

    mVernierEffect->stop(effect_manager->get_effekseer_manager());

    mBombEffect->play(effect_manager->get_effekseer_manager(), mPosition, 2.0f);

    if (mIsStun) mStunEffect->stop(effect_manager->get_effekseer_manager());
    // カメラシェイク
    camera_shake->reset(Graphics_);
    audio_manager->play_se(SE_INDEX::ENEMY_EXPLOSION);

    //-----死んだことを送信する-----//
    EnemySendData::EnemyDieData data;

    data.cmd[ComLocation::ComList] = CommandList::Update;
    data.cmd[ComLocation::UpdateCom] = UpdateCommand::EnemyDieCommand;

    data.object_id = object_id;

    //-----データを送信する-----//
    CorrespondenceManager::Instance().UdpSend((char*)&data, sizeof(EnemySendData::EnemyDieData));

}


void BaseEnemy::fUpdateVernierEffectPos()
{
    //--------------------<�o�[�j�A�̂̈ʒu�����肷��>--------------------//
    DirectX::XMFLOAT3 position{};
    DirectX::XMFLOAT3 up{};
    DirectX::XMFLOAT4X4 q{};


    // �{�[���̖��O����ʒu�Ə�x�N�g�����擾
    mpModel->fech_by_bone(mAnimPara,Math::calc_world_matrix(mScale, mOrientation, mPosition),mVenierBone, position, up, q);

    mVernierEffect->set_position(effect_manager->get_effekseer_manager(), position);
   //クォータニオン回転
    mVernierEffect->set_quaternion(effect_manager->get_effekseer_manager(),mOrientation);
}

void BaseEnemy::fTurnToPlayer(float elapsedTime_,float RotSpeed_)
{
    // �v���C���[�̕����ɉ�]
    constexpr DirectX::XMFLOAT3 up = { 0.001f,1.0f,0.0f };

    // �v���C���[�Ƃ̃x�N�g��
    const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(mPlayerPosition - mPosition);
    // �����̐��ʃx�N�g��
    const auto front = Math::Normalize(Math::GetFront(mOrientation));
     float dot = Math::Dot(vToPlayer, front);

     dot = acosf(dot);

    if (fabs(dot) > DirectX::XMConvertToRadians(2.0f))
    {
        float cross{ (vToPlayer.x * front.z) - (vToPlayer.z * front.x) };
        if (cross > 0)
        {
            mOrientation = Math::RotQuaternion(mOrientation, up, dot * RotSpeed_ * elapsedTime_);
        }
        else
        {
            mOrientation = Math::RotQuaternion(mOrientation, up, -dot * RotSpeed_ * elapsedTime_);
        }
    }
}

void BaseEnemy::fTurnToTarget(float elapsedTime_, float RotSpeed_,
    DirectX::XMFLOAT3 Target_)
{
    // �v���C���[�̕����ɉ�]
    constexpr DirectX::XMFLOAT3 up = { 0.001f,1.0f,0.0f };

    // �v���C���[�Ƃ̃x�N�g��
    const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(Target_ - mPosition);
    // �����̐��ʃx�N�g��
    const auto front = Math::Normalize(Math::GetFront(mOrientation));
    float dot = Math::Dot(vToPlayer, front);

    dot = acosf(dot);

    if (fabs(dot) > DirectX::XMConvertToRadians(2.0f))
    {
        float cross{ (vToPlayer.x * front.z) - (vToPlayer.z * front.x) };
        if (cross > 0)
        {
            mOrientation = Math::RotQuaternion(mOrientation, up, dot * RotSpeed_ * elapsedTime_);
        }
        else
        {
            mOrientation = Math::RotQuaternion(mOrientation, up, -dot * RotSpeed_ * elapsedTime_);
        }
    }
}

void BaseEnemy::fTurnToPlayerXYZ(float elapsedTime_, float RotSpeed_)
{
    // �v���C���[�̕����ɉ�]
    // �����Z�o
    DirectX::XMFLOAT3 axis = {};

    // �v���C���[�Ƃ̃x�N�g��
    const DirectX::XMFLOAT3 vToPlayer = Math::Normalize(mPlayerPosition - mPosition);
    // �����̐��ʃx�N�g��
    const auto front = Math::Normalize(Math::GetFront(mOrientation));
    float dot = Math::Dot(vToPlayer, front);

    dot = acosf(dot);

    if (fabs(dot) > DirectX::XMConvertToRadians(2.0f))
    {
        axis = Math::Cross(front, vToPlayer);
        float cross{ (vToPlayer.x * front.z) - (vToPlayer.z * front.x) };
        if (cross > 0)
        {
            mOrientation = Math::RotQuaternion(mOrientation, axis, dot * RotSpeed_ * elapsedTime_);
        }
        else
        {
            mOrientation = Math::RotQuaternion(mOrientation, axis, -dot * RotSpeed_ * elapsedTime_);
        }
    }
}

void BaseEnemy::fMoveFront(float elapsedTime_, float MoveSpeed_)
{
    // �O�����ɐi
    const auto velocity = Math::Normalize(Math::GetFront(mOrientation)) * MoveSpeed_;
    mPosition += (velocity * elapsedTime_);
}

void BaseEnemy::fComputeInCamera()
{
    DirectX::XMFLOAT3 maxPos{
        mPosition.x + mCubeHalfSize,
        mPosition.y + mCubeHalfSize,
        mPosition.z + mCubeHalfSize
    };
    DirectX::XMFLOAT3 minPos{
        mPosition.x - mCubeHalfSize,
        mPosition.y - mCubeHalfSize,
        mPosition.z - mCubeHalfSize
    };

    mIsInCamera = Collision::forefront_frustum_vs_cuboid(minPos, maxPos);
}

void BaseEnemy::fLimitPosition()
{
    if(mIsBoss)
    {
        return;
    }
    constexpr float length = 70.0f;
    DirectX::XMFLOAT3 v = mPosition - mPlayerPosition;
    //ボスの演出時に敵を近づけない
    if(Math::Length(v)<=length)
    {
        v = Math::Normalize(v);
        mPosition = mPlayerPosition + (v * length);
    }
}

void BaseEnemy::fSetStun(bool Arg_, bool IsJust_)
{
    mIsStun = Arg_;
}


void BaseEnemy::fSetPlayerPosition(const DirectX::XMFLOAT3& PlayerPosition_)
{
    mPlayerPosition = PlayerPosition_;
}

void BaseEnemy::fSetAttack(bool Arg_)
{
    mIsAttack = Arg_;
}

void BaseEnemy::fSetAttackOperation(bool Arg_)
{
    attack_operation = Arg_;
}

void BaseEnemy::fSetIsLockOnOfChain(bool RockOn_)
{
    mIsLockOnOfChain = RockOn_;
}

void BaseEnemy::fSetIsPlayerSearch(bool Arg_)
{
    mIsPlayerSearch = Arg_;
}

void BaseEnemy::fSetLaunchDissolve()
{
    if(mCurrentHitPoint <= 0)
    {
        mDissolve = 1.0f;
    }
}

bool BaseEnemy::fGetAttack() const
{
    return mIsAttack;
}

bool BaseEnemy::fGetAttackOperation() const
{
    return attack_operation;
}

const Capsule& BaseEnemy::fGetBodyCapsule()
{
    mBodyCapsule.mBottom = mPosition;
    const DirectX::XMFLOAT3 up = { 0.01f,1.0f,0.0f };
    mBodyCapsule.mTop = mPosition +( up * 5.0f);
    return mBodyCapsule;
}

const Capsule& BaseEnemy::fGetAttackCapsule() const
{
    return mAttackCapsule;
}

const DirectX::XMFLOAT3& BaseEnemy::fGetPosition() const
{
    return mPosition;
}

bool BaseEnemy::fGetIsAlive() const
{
    return mIsAlive;
}

void BaseEnemy::fSetIsAlive(bool arg)
{
    mIsAlive = arg;
}

bool BaseEnemy::fComputeAndGetIntoCamera() const
{
    const DirectX::XMFLOAT3 minPoint{
        mPosition.x - mCubeHalfSize,
        mPosition.y - mCubeHalfSize,
        mPosition.z - mCubeHalfSize
    };
    const DirectX::XMFLOAT3 maxPoint{
        mPosition.x + mCubeHalfSize,
        mPosition.y + mCubeHalfSize,
        mPosition.z + mCubeHalfSize
    };

    return Collision::frustum_vs_cuboid(minPoint, maxPoint);
}

int BaseEnemy::fGetAttackPower() const
{
    return mAttackPower;
}

float BaseEnemy::fGetAttackInvTime() const
{
    return mAttackInvTime;
}

float BaseEnemy::fGetLengthFromPlayer() const
{
    return Math::Length(mPlayerPosition - mPosition);
}

float BaseEnemy::fGetPercentHitPoint() const
{
    if (mCurrentHitPoint <= 0)
        return 0.0f;
    else
        return static_cast<float>(mCurrentHitPoint) / static_cast<float>(mMaxHp);
}

bool BaseEnemy::fGetStun() const
{
    return mIsStun;
}

float BaseEnemy::fGetCurrentHitPoint() const
{
    return static_cast<float>(mCurrentHitPoint);
}

void BaseEnemy::fSetCurrentHitPoint(int hp)
{
    mCurrentHitPoint = hp;
}

bool BaseEnemy::fGetAppears() const
{
    return is_appears;
}

bool BaseEnemy::fIsLockOnOfChain() const
{
    return mIsLockOnOfChain;
}

float BaseEnemy::fGetDissolve() const
{
    return mDissolve;
}

DirectX::XMFLOAT3 BaseEnemy::fGetForward() const
{
    DirectX::XMFLOAT3 forward = Math::GetFront(mOrientation);
    const DirectX::XMVECTOR Forward = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&forward));
    DirectX::XMStoreFloat3(&forward, Forward);
    return forward;
}

bool BaseEnemy::fGetIsBoss() const
{
    return mIsBoss;
}

bool BaseEnemy::fGetInnerCamera()
{
    return mIsInCamera;
}

void BaseEnemy::LerpPosition(float elapsedTime_)
{
    using namespace DirectX;
    XMVECTOR p1{ XMLoadFloat3(&mPosition) };
    XMVECTOR p2{ XMLoadFloat3(&mReceivePositiom) };
    XMVECTOR dir{ p2 - p1 };

    XMFLOAT3 l{};
    XMStoreFloat3(&l, dir);

    float length = Math::Length(l);

    if (length > AllowableLimitPosition)
    {
        mPosition = Math::lerp(mPosition, mReceivePositiom, elapsedTime_);
    }
    else
    {
        mStartlerp = false;
        mPosition = mReceivePositiom;
    }

}

void BaseEnemy::fSetReceivePosition(DirectX::XMFLOAT3 pos)
{
    using namespace DirectX;
    XMVECTOR p1{ XMLoadFloat3(&mPosition) };
    XMVECTOR p2{ XMLoadFloat3(&mReceivePositiom) };
    XMVECTOR dir{ p2 - p1 };

    XMFLOAT3 l{};
    XMStoreFloat3(&l, dir);

    float length = Math::Length(l);

    if (length > AllowableLimitPosition)
    {
        //-----補間終了位置を設定-----//
        mReceivePositiom = pos;

        //-----補間開始-----//
        mStartlerp = true;
    }
    else
    {
        mPosition = pos;
    }
}

void BaseEnemy::fChangeState(const char* Tag_)
{
    //-----Tag_と同じ名前をmap内から探す-----//
    // 見つからなかったらストップ
    if (mFunctionMap.find(Tag_) != mFunctionMap.end())
    {
        std::string str = "KEY IS NOT FIND    ";
        str += Tag_;
        OutputDebugStringA(str.c_str());
    }

    //-----見つかったらそのmaoに保存されている初期化関数と更新関数が入っているtupleを代入-----//
    mCurrentTuple = mFunctionMap.at(Tag_);

    //-----初期化関数を呼び出す-----//
    std::get<0>(mCurrentTuple)();
}
