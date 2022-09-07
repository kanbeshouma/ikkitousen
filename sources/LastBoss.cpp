#include"LastBoss.h"

#include "BulletManager.h"
#include"EnemyManager.h"
#include"Operators.h"
#include"DragonBreath.h"
#include"enemy_hp_gauge.h"
LastBoss::LastBoss(GraphicsPipeline& Graphics_,
    const DirectX::XMFLOAT3& EmitterPoint_,
    const EnemyParamPack& ParamPack_,
    EnemyManager* pEnemyManager_)
    :BaseEnemy(Graphics_,
        "./resources/Models/Enemy/boss_animation_sixth.fbx",
        ParamPack_, EmitterPoint_,L"./resources/Sprites/ui/minimap/minimap_lastboss.png"), mpEnemyManager(pEnemyManager_)
{
    mIsBoss = true;
    
    // ボーンを初期化
    mShipFace = mpModel->get_bone_by_name("shipface_top_joint");

    // タレットのモデルを初期化
    fRegisterFunctions();
   

    // laserを初期化
    mShipPointer.fInitialize(Graphics_.get_device().Get(),  L"");
    mRightPointer.fInitialize(Graphics_.get_device().Get(), L"");
    mLeftPointer.fInitialize(Graphics_.get_device().Get(),  L"");


    // タレットを初期化
    mpTurretLeft = std::make_unique<Turret>(Graphics_);
    mpTurretRight = std::make_unique<Turret>(Graphics_);
    // タレットのボーンを初期化
    mTurretBoneLeft = mpModel->get_bone_by_name
    ("armor_L_fire_joint");
    mTurretBoneRight= mpModel->get_bone_by_name
    ("armor_R_fire_joint");

    mpSecondGunLeft = std::make_unique<SecondGun>(Graphics_);
    mpSecondGunRight = std::make_unique<SecondGun>(Graphics_);
    mSecondGunBoneLeft = mpModel->get_bone_by_name
    ("secondarygun_L_root_joint");
    mSecondGunBoneRight = mpModel->get_bone_by_name
    ("secondarygun_R_root_joint");

    // 弾発射用の関数を取得
    mfAddBullet= BulletManager::Instance().fGetAddFunction();


    // ラッシュする敵を登録
    DirectX::XMFLOAT3 entryPos{ 0.0f,500.0f,0.0f };
    auto  enemy = new BossRushUnit(Graphics_,entryPos);
    pEnemyManager_->fAddRushBoss(enemy);
    mRushVec.emplace_back(enemy);

    auto  enemy1 = new BossRushUnit(Graphics_,entryPos);
    pEnemyManager_->fAddRushBoss(enemy1);
    mRushVec.emplace_back(enemy1);

    auto  enemy2 = new BossRushUnit(Graphics_,entryPos);
    pEnemyManager_->fAddRushBoss(enemy2);
    mRushVec.emplace_back(enemy2);

    //エフェクトを初期化
    mpAllAttackEffect = std::make_unique<Effect>(Graphics_,
        effect_manager->get_effekseer_manager(),
        "./resources/Effect/boss_wave.efk"); 

    mpBeamEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/boss_beam_big.efk");

    mpBeamBaseEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/beam_base2.efk");

    mpBeamRightEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/boss_beam_big.efk");
    mpBeamLeftEffect = std::make_unique<Effect>(Graphics_, effect_manager->get_effekseer_manager(), "./resources/Effect/boss_beam_big.efk");
    mpDieEffect = std::make_unique<Effect>(Graphics_,
        effect_manager->get_effekseer_manager(),
        "./resources/Effect/boss_death.efk");


   // ボーンを取得
   mCameraEyeBone = mpModel->get_bone_by_name("camera_joint");
   mCameraFocusBone = mpModel->get_bone_by_name("camera_focus_joint");


    // パラメーターをロード
   fLoadParam();

    // 内容に応じて初期ステータスを切り替える
   if (mBossParam.BossStateNumber == 0) // 戦艦
   {
       // 何もしない
   }
   else if (mBossParam.BossStateNumber == 1) // 人型
   {
       fChangeState(DivideState::ShipToHuman);
   }
   else if(mBossParam.BossStateNumber == 2) // ドラゴン
   {
       fChangeState(DivideState::HumanToDragon);
   }

   auto a = this;

   mVernierEffect->stop(effect_manager->get_effekseer_manager());
}

LastBoss::LastBoss(GraphicsPipeline& Graphics_)
    : BaseEnemy(Graphics_, "./resources/Models/Enemy/boss_animation_sixth.fbx")
{

    mScale = { 0.05f,0.05f,0.05f };
    mPosition = { 0.0f,20.0f,250.0f };
}

LastBoss::~LastBoss()
{
    fSaveParam();
}

void LastBoss::fUpdate(GraphicsPipeline& Graphics_, float elapsedTime_)
{
    BossHpGauge::set_hp_percent(fGetPercentHitPoint());

    elapsedTime_ = fBaseUpdate(elapsedTime_, Graphics_);
    fGuiMenu();

    // レーザーポインターを更新
    mShipPointer.fUpdate();
    mRightPointer.fUpdate();
    mLeftPointer.fUpdate();

    // 各モードから次のモードに遷移する
    switch (mCurrentMode) {
    case Mode::Ship:
        break;
    case Mode::Human:
        // 体力が特定の割合を下回ったら
        fChangeHumanToDragon();
        break;
    case Mode::Dragon:
        break;
    default:;
    }

    // タレットを更新
    mpTurretLeft->fUpdate(elapsedTime_, Graphics_);
    mpTurretRight->fUpdate(elapsedTime_, Graphics_);


    mpEnemyManager->fSetBossMode(mCurrentMode);

    // ボスにカメラを向けさせる
    DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale,
        mOrientation, mPosition);
    DirectX::XMFLOAT3 eyePosition{};
    DirectX::XMFLOAT3 focusPosition{};
    DirectX::XMFLOAT3 dummyUp{};
    // Eyeを取得
    mpModel->fech_by_bone(mAnimPara,world, mCameraEyeBone, eyePosition, dummyUp);
    mpModel->fech_by_bone(mAnimPara,world, mCameraFocusBone, focusPosition, dummyUp);
    if(mpEnemyManager)
    {
        mpEnemyManager->fSetBossEye(eyePosition);
        mpEnemyManager->fSetBossFocus(focusPosition);
    }
    else
    {
        throw std::logic_error("EnemyManager Not Find");
    }

}

void LastBoss::fUpdateAttackCapsule()
{
    throw std::logic_error("Not implemented");
}


void LastBoss::fSetStun(bool Arg_, bool IsJust_)
{
    // 船の状態はスタンしない
    if (mCurrentMode == Mode::Ship || mPosition.y >= 10.0f)
    {
        return;
    }

    if(IsJust_)
    {
        mIsStun = true;
        fChangeState(DivideState::Stun);
    }
}

void LastBoss::fRegisterFunctions()
{
    {
        InitFunc ini = [=]()->void
        {
            fShipStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipIdle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamCharge, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamShootInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamShootUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamShoot, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fShipBeamEndInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fShipBeamEndUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipBeamEnd, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fChangeShipToHumanInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fChangeShipToHumanUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::ShipToHuman, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanIdle, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanAllShotInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanAllShotUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanAllShot, 
            tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanDieStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanDieStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanDieStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanDieMiddleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanDieMiddleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanDieMiddle, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanToDragonInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanToDragonUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanToDragon, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanBlowAttackInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanBlowAttackUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanBlowAttack, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackAwayInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackAwayUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpAway, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackWaitInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackWaitUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpWait, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackTimeOverInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackTimeOverUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpOver, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpBeamShootInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpBeamShootUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpShoot,
            tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpAttackChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpAttackChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpCharge, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanSpDamageInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanSpDamageUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanSpDamage, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fMoveAwayInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fMoveAwayUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanMove, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fHumanRushInit();
        };
        UpdateFunc up = [=](float elapsedTime_,
            GraphicsPipeline& Graphics_)->void
        {
            fHumanRushUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::HumanRush, tuple));
    }


    // ドラゴン：待機
    {
        InitFunc ini = [=]()->void
        {
            fDragonIdleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonIdleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonIdle, tuple));
    }

    // ドラドンブレス前に消える
    {
        InitFunc ini = [=]()->void
        {
            fDragonFastBreathStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonFastBreathStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonHideStart, tuple));
    }
    // ドラゴン：現れる
    {
        InitFunc ini = [=]()->void
        {
            fDragonBreathAppearInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBreathAppearUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonAppear, tuple));
    }
    // ブレス溜める
    {
        InitFunc ini = [=]()->void
        {
            fDragonBreathChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBreathChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBreathCharge, tuple));
    }
    // ブレス発射
    {
        InitFunc ini = [=]()->void
        {
            fDragonBreathShotInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBreathShotUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBreathShot, tuple));
    }

    // 突進消える
    {
        InitFunc ini = [=]()->void
        {
            fDragonRushHideInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonRushHideUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonRushHide, tuple));
    }
    // 突進待機
    {
        InitFunc ini = [=]()->void
        {
            fDragonRushWaitInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonRushWaitUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonRushWait ,tuple));
    }
    // 突進現れる
    {
        InitFunc ini = [=]()->void
        {
            fDragonRushAppearInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonRushAppearUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonRushAppear, tuple));
    }

    // ビーム前に移動
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamMoveInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamMoveUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonMoveStart, tuple));
    }
    // ビーム前に移動
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBeamStart, tuple));
    }

    // ビームため
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamChargeInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamChargeUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBeamCharge, tuple));
    }
    // ビーム発射
    {
        InitFunc ini = [=]()->void
        {
            fDragonBeamShotInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonBeamShotUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonBeamShoot, tuple));
    }

    // ドラゴン：死亡
    {
        InitFunc ini = [=]()->void
        {
            fDragonDieStartInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonDieStartUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonDieStart, tuple));
    }
    {
        InitFunc ini = [=]()->void
        {
            fDragonDieMiddleInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fDragonDieMiddleUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::DragonDieEnd, tuple));
    }

    {
        InitFunc ini = [=]()->void
        {
            fStunInit();
        };
        UpdateFunc up = [=](float elapsedTime_, GraphicsPipeline& Graphics_)->void
        {
            fStunUpdate(elapsedTime_, Graphics_);
        };
        auto tuple = std::make_tuple(ini, up);
        mFunctionMap.insert(std::make_pair(DivideState::Stun, tuple));
    }


    fChangeState(DivideState::ShipStart);
}

void LastBoss::fGuiMenu()
{
#ifdef USE_IMGUI
    ImGui::Begin("LastBoss");
    ImGui::DragFloat3("Position", &mPosition.x);
    ImGui::DragFloat3("Scale", &mScale.x);
    ImGui::DragFloat4("Orientation", &mOrientation.x);

    if (ImGui::Button("Beam"))
    {
        fChangeState(DivideState::ShipBeamStart);
    }

    // 現在のModeを表記
    switch (mCurrentMode) {
    case Mode::Ship:
        ImGui::Text("Mode::Ship");
        break;
    case Mode::Human:
        ImGui::Text("Mode::Human");
        break;
    case Mode::Dragon:
        ImGui::Text("Mode::Dragon");
        break;
    case Mode::HumanToDragon:
        ImGui::Text("Mode::HumanToDragon");
        break;
    case Mode::None: break;
    case Mode::ShipToHuman: break;
    case Mode::DragonDie: break;
    default: ;
    }

    if (ImGui::TreeNode("Hp"))
    {
        ImGui::InputInt("CurrentHp", &mCurrentHitPoint);
        float p = fComputePercentHp();
        ImGui::SliderFloat("Percent",&p,0.0f,1.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("State"))
    {
        if (ImGui::Button("HumanAllShot"))
        {
            fChangeState(DivideState::HumanAllShot);
        }
        if (ImGui::Button("HumanBlow"))
        {
            fChangeState(DivideState::HumanBlowAttack);
        }
        if (ImGui::Button("HumanAway"))
        {
            fChangeState(DivideState::HumanSpAway);
        }
        if(ImGui::Button("Rush"))
        {
            fChangeState(DivideState::HumanRush);
        }
        if (ImGui::Button("DragonHide"))
        {
            fChangeState(DivideState::DragonHideStart);
        }
        if (ImGui::Button("DragonRush"))
        {
            fChangeState(DivideState::DragonRushHide);
        }
        if (ImGui::Button("DragonBeam"))
        {
            fChangeState(DivideState::DragonMoveStart);
        }
        ImGui::TreePop();
    }


    if(ImGui::TreeNode("Area"))
    {
        int areaSeed{};
        if (mPosition.x > 0.0f)
        {
            areaSeed++;
        }
        if (mPosition.z < 0.0f)
        {
            areaSeed += 2;
        }

        ImGui::DragInt("areaSeed", &areaSeed);

        ImGui::TreePop();
    }

    if(ImGui::Button("RushUnit"))
    {
        for (auto rush : mRushVec)
        {
            rush->fStartAppear({ 0.0f,0.0f,0.0f });
        }
    }

    ImGui::SliderFloat("MoveThreshold", &mMoveThreshold, 0.0f, 1.0f);
    float v = Math::Length(mPosition);
    ImGui::DragFloat("Length", &v);

    ImGui::RadioButton("FarRand", Math::Length(mPlayerPosition - mPosition) > mkDistanceToPlayer);

    // カメラ
    DirectX::XMFLOAT4X4 world = Math::calc_world_matrix(mScale,
        mOrientation, mPosition);
    DirectX::XMFLOAT3 eyePosition{};
    DirectX::XMFLOAT3 focusPosition{};
    DirectX::XMFLOAT3 dummyUp{};
    // Eyeを取得
    mpModel->fech_by_bone(mAnimPara,world, mCameraEyeBone, eyePosition, dummyUp);
    mpModel->fech_by_bone(mAnimPara,world, mCameraFocusBone, focusPosition, dummyUp);

    ImGui::DragFloat3("CameraEye", &eyePosition.x);
    ImGui::DragFloat3("CameraFocus", &focusPosition.x);

    int hp = mMaxHp;
    ImGui::DragInt("MaxHp", &hp);

    static DirectX::XMFLOAT3 pos{};
    ImGui::DragFloat3("pp", &pos.x);
    if(ImGui::Button("Effect"))
    {
        mpBeamEffect->play(effect_manager->get_effekseer_manager(), pos);
        mpBeamEffect->set_scale(effect_manager->get_effekseer_manager(), { 15.0f,15.0f,15.0f });
    }
    
    ImGui::RadioButton("IsAttack", mIsAttack);
    ImGui::Checkbox("Stun", &mIsStun);
    ImGui::DragFloat("Timer", &mTimer);

    ImGui::InputInt("AttackPower", &mAttackPower);

    ImGui::DragFloat("Dissolve", &mDissolve, 0.0f, 1.0f);

    ImGui::End();
#endif
}



float LastBoss::fComputePercentHp() const
{
    return static_cast<float>(mCurrentHitPoint) / static_cast<float>(mMaxHp);
}

void LastBoss::fChangeHumanToDragon()
{
    // 体力の割合が20%を下回ったらモードをドラゴンに変える
    if(fComputePercentHp()<=mkPercentToDragon)
    {
        fChangeState(DivideState::HumanDieStart);
        // 変形中モードに遷移（ダメージは受けない）
        mCurrentMode = Mode::HumanToDragon;
    }
}

void LastBoss::fSpawnChildUnit(GraphicsPipeline& Graphics_, int Amounts_) const
{
    // ユニットを召喚する
    std::vector<DirectX::XMFLOAT3> vec{};
    vec.reserve(Amounts_);

    // 定数
    const DirectX::XMFLOAT3 SummonCenterPosition = mPlayerPosition;

    // 一体当たりの回転角を算出する
    const float peaceOfRotation = 360.0f / static_cast<float>(Amounts_);
    for (int i = 0; i < Amounts_; ++i)
    {
        // 回転角に応じた位置を決定する
        const float rot = DirectX::XMConvertToRadians(peaceOfRotation * static_cast<float>(i));
        const DirectX::XMFLOAT3 unitPosition
        = { cosf(rot),0.0f,sinf(rot) };

        vec.emplace_back(SummonCenterPosition + (unitPosition * 60.0f));
    }

    mpEnemyManager->fReserveBossUnit(vec);

}

bool LastBoss::fLimitStageHuman(float elapsedTime_)
{
    // もしステージの端を超えたら
    if (Math::Length(mPosition) < mkLimitStage)
    {
        return false;
    }

    fChangeState(DivideState::HumanRush);
    return true;
}


