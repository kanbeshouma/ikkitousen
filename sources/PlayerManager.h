#pragma once
#include<memory>
#include<vector>

#include"BasePlayer.h"

//===================================//
////----------プレイヤーを管理するクラス----------//
//===================================//
class PlayerManager
{
public:
    PlayerManager();
    ~PlayerManager();
public:
    ////----------更新関数----------//
    void Update(float elapsed_time, GraphicsPipeline& graphics, SkyDome* sky_dome, std::vector<BaseEnemy*> enemies);

    ////-----描画関数-----//
    void Render(GraphicsPipeline& graphics, float elapsed_time);
public:
    ////-----プレイヤー登録関数-----//
    void RegisterPlayer(BasePlayer* player);

    ////-----ステージ移動開始-----//
    void TransitionStageMove();

    ////-----ステージ移動終了-----//
    void TransitionStageMoveEnd();

    ////-----ジャスト回避したかどうか-----//
    bool GetIsJustAvoidance();

    ////-----プレイヤーの位置取得-----//
    //=========================
    //戻り値 : 登録されているプレイヤーの位置の配列
    std::vector<DirectX::XMFLOAT3> GetPosition();


private:
    ////-----プレイヤーを入れる変数-----//
    std::vector<std::shared_ptr<BasePlayer>> players;
private:
    //-----この端末で操作することができるプレイヤーのオブジェクト番号-----//
    int private_object_id{ -1 };
};