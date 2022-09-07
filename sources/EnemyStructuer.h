#pragma once
#include<DirectXMath.h>
#include <cereal/cereal.hpp>

struct Capsule
{
    DirectX::XMFLOAT3 mTop{};
    DirectX::XMFLOAT3 mBottom{};
    float mRadius{};
};


struct EnemyParamPack
{
    int MaxHp{};
    int AttackPower{};
    float InvincibleTime{}; // 攻撃した相手の無敵時間
    float BodyCapsuleRad{};
    float AttackCapsuleRad{};
    float StunTime{}; // スタンのながさ
    template<class Archive>
    void serialize(Archive& archive, std::uint32_t const version)
    {
        if(version<=1)
        {
            archive(
            cereal::make_nvp("MaxHp", MaxHp),
            cereal::make_nvp("AttackPower", AttackPower),
            cereal::make_nvp("InvincibleTime", InvincibleTime),
            cereal::make_nvp("BodyCapsuleRad", BodyCapsuleRad),
            cereal::make_nvp("AttackCapsuleRad", AttackCapsuleRad));
        }
        else
        {
            archive(
                cereal::make_nvp("MaxHp", MaxHp),
                cereal::make_nvp("AttackPower", AttackPower),
                cereal::make_nvp("InvincibleTime", InvincibleTime),
                cereal::make_nvp("BodyCapsuleRad", BodyCapsuleRad),
                cereal::make_nvp("AttackCapsuleRad", AttackCapsuleRad),
                cereal::make_nvp("StunTime",StunTime));
        }
    }
};

CEREAL_CLASS_VERSION(EnemyParamPack, 2);

// 敵の種類
enum class EnemyType
{
    // ↓↓↓↓↓↓↓↓↓ここから下に増やす↓↓↓↓↓↓↓↓↓↓↓↓↓
    Archer, //遠距離攻撃
    Shield, //盾持ち
    Sword,  // 刀の敵 
    Spear,
    Archer_Ace, //遠距離攻撃
    Shield_Ace, //盾持ち
    Sword_Ace,  // 刀の敵 
    Spear_Ace,
    Boss,
    Tutorial_NoMove,
    Boss_Unit,
    // ↑↑↑↑↑↑↑↑↑ここから上に増やす↑↑↑↑↑↑↑↑↑↑↑↑↑
    Count, // 種類の総量
};


struct EnemySource
{
    float mSpawnTimer{};  // 出現タイミングを記録
    DirectX::XMFLOAT3 mEmitterPoint{}; // 出現位置の番号
    EnemyType mType{};          // 敵の種類

    // シリアライズ
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            cereal::make_nvp("Timer", mSpawnTimer),
            cereal::make_nvp("Emitter", mEmitterPoint),
            cereal::make_nvp("Type", mType));
    }
};
