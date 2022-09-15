#include"ClientPlayer.h"

void ClientPlayer::RegisterTransitionAnimationMap(TransEdge tuples, TransitionAnimation func)
{
    //==========std::get���牽�̒l�����o���Ă��邩�𖾊m�ɂ���==========//

    //-----�J�ڌ��̃A�j���[�V�����ԍ�-----//
    constexpr int TransRootIndex = 0;

    //-----�J�ڐ�̃A�j���[�V�����ԍ�-----//
    constexpr int TransDestinationIndex = 1;

    //==========tuples����J�ڌ��ƑJ�ڐ�̃A�j���[�V�����ԍ����擾����==========//

    //-----�J�ڌ��̃A�j���[�V�����ԍ�-----//
    const int trans_root_index = std::get<TransRootIndex>(tuples);

    //-----�J�ڐ�̃A�j���[�V�����ԍ�-----//
    const int trans_destination_index = std::get<TransDestinationIndex>(tuples);

    //-----�J�ڌ����擾-----//
    auto& child_map = transition_animations[trans_root_index];

    //-----��������J�ڐ�̊֐������ɓo�^����Ă��邩�𒲂ׂ�-----//
    if (child_map.contains(trans_destination_index))
    {
        //-----�����o�^����Ă�����I��-----//
        return;
    }

    //-----�֐���o�^����-----//
    child_map.emplace(std::make_pair(trans_destination_index, func));
}

void ClientPlayer::RegisterAnimationFunctions()
{
    RegisterTransitionAnimations([=]() {TransitionIdle(); },
        TransEdge(ActionState::ActionMove, ActionState::ActionIdle),
        TransEdge(ActionState::ActionAttack1, ActionState::ActionIdle),
        TransEdge(ActionState::ActionAttack2, ActionState::ActionIdle),
        TransEdge(ActionState::ActionAttack3, ActionState::ActionIdle),
        TransEdge(ActionState::ActionAvoidance, ActionState::ActionIdle),
        TransEdge(ActionState::ActionAwaking, ActionState::ActionIdle),
        TransEdge(ActionState::ActionCharge, ActionState::ActionIdle),
        TransEdge(ActionState::ActionDamage, ActionState::ActionIdle),
        TransEdge(ActionState::ActionInvAwaking, ActionState::ActionIdle),
        TransEdge(ActionState::ActionStageMoveEnd, ActionState::ActionIdle)
    );

    RegisterTransitionAnimations([=]() {TransitionMove(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionMove),
        TransEdge(ActionState::ActionAttack1, ActionState::ActionMove),
        TransEdge(ActionState::ActionAttack2, ActionState::ActionMove),
        TransEdge(ActionState::ActionAttack3, ActionState::ActionMove),
        TransEdge(ActionState::ActionAvoidance, ActionState::ActionMove),
        TransEdge(ActionState::ActionAwaking, ActionState::ActionMove),
        TransEdge(ActionState::ActionCharge, ActionState::ActionMove),
        TransEdge(ActionState::ActionDamage, ActionState::ActionMove),
        TransEdge(ActionState::ActionInvAwaking, ActionState::ActionMove),
        TransEdge(ActionState::ActionStageMoveEnd, ActionState::ActionMove)
        );

    RegisterTransitionAnimations([=]() {TransitionAvoidance(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionAvoidance),
        TransEdge(ActionState::ActionMove, ActionState::ActionAvoidance)
        );

    RegisterTransitionAnimations([=]() {TransitionChargeInit(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionChargeInit),
        TransEdge(ActionState::ActionMove, ActionState::ActionChargeInit)
        );

    RegisterTransitionAnimations([=]() {TransitionCharge(); },
        TransEdge(ActionState::ActionChargeInit, ActionState::ActionCharge),
        TransEdge(ActionState::ActionAttack3, ActionState::ActionCharge)
        );

    RegisterTransitionAnimations([=]() {TransitionAttackType1(); },
        TransEdge(ActionState::ActionAvoidance, ActionState::ActionAttack1),
        TransEdge(ActionState::ActionCharge, ActionState::ActionAttack1)
    );

    RegisterTransitionAnimations([=]() {TransitionAttackType2(); },
        TransEdge(ActionState::ActionAttack1, ActionState::ActionAttack2)
    );

    RegisterTransitionAnimations([=]() {TransitionAttackType3(); },
        TransEdge(ActionState::ActionAttack2, ActionState::ActionAttack3)
    );

    RegisterTransitionAnimations([=]() {TransitionDamage(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionDamage),
        TransEdge(ActionState::ActionMove, ActionState::ActionDamage),
        TransEdge(ActionState::ActionAttack1, ActionState::ActionDamage),
        TransEdge(ActionState::ActionAttack2, ActionState::ActionDamage),
        TransEdge(ActionState::ActionAttack3, ActionState::ActionDamage)
    );

    RegisterTransitionAnimations([=]() {TransitionAwaking(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionAwaking),
        TransEdge(ActionState::ActionMove, ActionState::ActionAwaking),
        TransEdge(ActionState::ActionCharge, ActionState::ActionAwaking)
    );

    RegisterTransitionAnimations([=]() {TransitionInvAwaking(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionInvAwaking),
        TransEdge(ActionState::ActionMove, ActionState::ActionInvAwaking),
        TransEdge(ActionState::ActionCharge, ActionState::ActionInvAwaking)
    );

    RegisterTransitionAnimations([=]() {TransitionStageMove(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionStageMove),
        TransEdge(ActionState::ActionMove, ActionState::ActionStageMove),
        TransEdge(ActionState::ActionChargeInit, ActionState::ActionStageMove),
        TransEdge(ActionState::ActionCharge, ActionState::ActionStageMove),
        TransEdge(ActionState::ActionAttack1, ActionState::ActionStageMove),
        TransEdge(ActionState::ActionAttack2, ActionState::ActionStageMove),
        TransEdge(ActionState::ActionAttack3, ActionState::ActionStageMove)
    );

    RegisterTransitionAnimations([=]() {TransitionStageMoveIdle(); },
        TransEdge(ActionState::ActionStageMove, ActionState::ActionStageMoveIdle)
    );

    RegisterTransitionAnimations([=]() {TransitionStageMoveEnd(); },
        TransEdge(ActionState::ActionStageMoveIdle, ActionState::ActionStageMoveEnd)
    );

    RegisterTransitionAnimations([=]() {TransitionDie(); },
        TransEdge(ActionState::ActionIdle, ActionState::ActionDie),
        TransEdge(ActionState::ActionMove, ActionState::ActionDie),
        TransEdge(ActionState::ActionAttack1, ActionState::ActionDie),
        TransEdge(ActionState::ActionAttack2, ActionState::ActionDie),
        TransEdge(ActionState::ActionAttack3, ActionState::ActionDie),
        TransEdge(ActionState::ActionAvoidance, ActionState::ActionDie),
        TransEdge(ActionState::ActionAwaking, ActionState::ActionDie),
        TransEdge(ActionState::ActionCharge, ActionState::ActionDie),
        TransEdge(ActionState::ActionDamage, ActionState::ActionDie),
        TransEdge(ActionState::ActionInvAwaking, ActionState::ActionDie),
        TransEdge(ActionState::ActionStageMoveEnd, ActionState::ActionDie)
    );

    RegisterTransitionAnimations([=]() {TransitionDying(); },
        TransEdge(ActionState::ActionDie, ActionState::ActionDying)
    );

}

void ClientPlayer::ActivationTransitionMap(int root_animation_index)
{
    //-----�J�ڊ֐����ۑ�����Ă���map����-----//
    //-----�\���������ŌĂяo��-----//
    for (auto& [key, map] : transition_animations)
    {
        //-----�ŏ���key�����̃A�j���[�V�����ԍ�����Ȃ������玟�ɂ���-----//
        if (key != root_animation_index) continue;

        //-----�v�f�̒��ɂ���map����-----//
        for (auto& [no_use, func] : map)
        {
            //-----�v�f�̒��ɂ���֐����Ă�-----//
            //-----true���A���ė�����J�ڂ���̂łʂ���-----//
            func();
        }
    }

    //-----�ǂ��ɂ��J�ڂ��Ȃ�-----//
}


void ClientPlayer::IdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionIdle);
}

void ClientPlayer::MoveUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionMove);
}

void ClientPlayer::AvoidanceUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionAvoidance);
}

void ClientPlayer::ChargeInitUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionChargeInit);
}

void ClientPlayer::ChargeUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionCharge);
}

void ClientPlayer::AttackType1Update(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionAttack1);
}

void ClientPlayer::AttackType2Update(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionAttack2);
}

void ClientPlayer::AttackType3Update(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionAttack3);
}

void ClientPlayer::DamageUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionDamage);
}

void ClientPlayer::AwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionAwaking);
}

void ClientPlayer::InvAwakingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionInvAwaking);
}

void ClientPlayer::StageMoveUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionStageMove);
}

void ClientPlayer::StageMoveIdleUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionStageMoveIdle);
}

void ClientPlayer::StageMoveEndUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionStageMoveEnd);
}

void ClientPlayer::DieUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionDie);
}

void ClientPlayer::DyingUpdate(float elapsed_time, SkyDome* sky_dome)
{
    ActivationTransitionMap(ActionState::ActionDying);
}
