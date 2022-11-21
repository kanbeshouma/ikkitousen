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
    float InvincibleTime{}; // �U����������̖��G����
    float BodyCapsuleRad{};
    float AttackCapsuleRad{};
    float StunTime{}; // �X�^���̂Ȃ���
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

// �G�̎��
enum class EnemyType
{
    // �������������������������牺�ɑ��₷��������������������������
    Archer, //0 �������U��
    Shield, //1 ������
    Sword,  //2 ���̓G
    Spear,//3
    Archer_Ace, //4 �������U��
    Shield_Ace, //5 ������
    Sword_Ace,  //6 ���̓G
    Spear_Ace,//7
    Boss,//8
    Tutorial_NoMove,//9
    Boss_Unit,//10
    // ���������������������������ɑ��₷��������������������������
    Count, // ��ނ̑���
};


//-----�ǂ̃^�C�v�̓G�̃f�[�^�𑗐M���邩-----//
enum SendEnemyType
{
    Sword,
    Archer,
    Spear,
    Shield,
    Boss,
    BossRush,
};



struct EnemySource
{
    // �o���^�C�~���O���L�^
    float mSpawnTimer{};
    //�o���ʒu�̔ԍ�
    DirectX::XMFLOAT3 mEmitterPoint{};
    // �G�̎��
    EnemyType mType{};
    //-----�G�̃O���[�v�̒��ł̃��[�_�[���ǂ���-----//
    bool master{};
    //-----�z�X�g���n��-----//
    int transfer_host{};
    //-----�O���[�v�ԍ�-----//
    int grope_id{};

    // �V���A���C�Y
    template<class Archive>
    void serialize(Archive& archive, std::uint32_t const version)
    {
        switch (version)
        {
        case 1:
        {
            archive(
                cereal::make_nvp("Timer", mSpawnTimer),
                cereal::make_nvp("Emitter", mEmitterPoint),
                cereal::make_nvp("Type", mType));
            break;
        }
        case 2:
        {
            archive(
                cereal::make_nvp("Timer", mSpawnTimer),
                cereal::make_nvp("Emitter", mEmitterPoint),
                cereal::make_nvp("Type", mType),
                cereal::make_nvp("Host", master),
                cereal::make_nvp("TransferHost", transfer_host),
                cereal::make_nvp("GropeId", grope_id));

            break;
        }
        default:
            break;
        }
    }
};

CEREAL_CLASS_VERSION(EnemySource, 2);
