#pragma once


//******************************************************************************
//
//
//      ユーザー（ユーティリティー）
//
//
//******************************************************************************

//------< インクルード >----------------------------------------------------------
#include <algorithm>
#include <sstream>
#include <bitset>
#include <assert.h>
#include <DirectXMath.h>
#include<vector>
#include "imgui_include.h"
//------< inline function >-----------------------------------------------------
namespace Math
{
    //--------------------------------------------------------------
    //  実数値のイコール判定
    //--------------------------------------------------------------
    inline bool equal_check(float value1, float value2, float ep = FLT_EPSILON)
    {
        return (value1 >= value2 - ep) && (value1 < value2 + ep);
    }
    inline bool equal_check(const DirectX::XMFLOAT2& value1, const DirectX::XMFLOAT2& value2, float ep = FLT_EPSILON)
    {
        return ((value1.x >= value2.x - ep) && (value1.x < value2.x + ep)) && ((value1.y >= value2.y - ep) && (value1.y < value2.y + ep));
    }
    //--------------------------------------------------------------
    //  値を範囲内に収める関数
    //--------------------------------------------------------------
    //    引数：const float& v  入力する数値
    //        ：const float& lo 最小値
    //        ：const float& hi 最大値
    //  戻り値：const float&    範囲内に収まった数値
    //--------------------------------------------------------------
    inline const float& clamp(const float& v, const float& lo, const float& hi)
    {
        assert(hi >= lo);
        return (std::max)((std::min)(v, hi), lo);
    }
    inline const int& clamp(const int& v, const int& lo, const int& hi)
    {
        assert(hi >= lo);
        return (std::max)((std::min)(v, hi), lo);
    }
    //--------------------------------------------------------------
    //  値をラップアラウンド（範囲を繰り返す）させる
    //--------------------------------------------------------------
    //  const int v  範囲を繰り返させたい値
    //  const int lo 下限値
    //  const int hi 上限値（loより大きい値でなければならない）
    //--------------------------------------------------------------
    //  戻り値：int    vをloからhiまでの範囲でラップアラウンドさせた数値
    //--------------------------------------------------------------
    inline int wrap(const int v, const int lo, const int hi)
    {
        assert(hi > lo);
        const int n = (v - lo) % (hi - lo); // 負数の剰余はc++11から使用可になった
        return n >= 0 ? (n + lo) : (n + hi);
    }
    // float版
    inline float wrap(const float v, const float lo, const float hi)
    {
        assert(hi > lo);
        const float n = std::fmod(v - lo, hi - lo);
        return n >= 0 ? (n + lo) : (n + hi);
    }
    //--------------------------------------------------------------
    //  任意のフレーム後にtrueを返す
    //--------------------------------------------------------------
    inline bool stop_watch(const int frame, int& timer)
    {
        assert(frame > 0);
        assert(frame >= timer);

        if (timer < frame) ++timer;
        return timer >= frame;
    }
    //--------------------------------------------------------------
    //  3Dモデルの座標系を変更する
    //--------------------------------------------------------------
    enum COORDINATE_SYSTEM
    {
        RHS_YUP, LHS_YUP, RHS_ZUP, LHS_ZUP,
    };
    inline const DirectX::XMFLOAT4X4 conversion_coordinate_system(
        COORDINATE_SYSTEM coordinate_system, float scale_factor = 1.0f)
    {
        const DirectX::XMFLOAT4X4 coordinate_system_transforms[]{
            {-1,0,0,0,   0,1,0,0,   0,0,1,0,   0,0,0,1},    // 0:RHS Y-UP
            { 1,0,0,0,   0,1,0,0,   0,0,1,0,   0,0,0,1},	// 1:LHS Y-UP
            {-1,0,0,0,   0,0,-1,0,  0,1,0,0,   0,0,0,1},	// 2:RHS Z-UP
            { 1,0,0,0,   0,0,1,0,   0,1,0,0,   0,0,0,1},	// 3:LHS Z-UP
        };
        // To change the units from centimeters to meters, set 'scale_factor' to 0.01.
        DirectX::XMFLOAT4X4 mat;
        DirectX::XMStoreFloat4x4(&mat, DirectX::XMMATRIX{ DirectX::XMLoadFloat4x4(&coordinate_system_transforms[coordinate_system])
            * DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) });
        return mat;
    }
    //--------------------------------------------------------------
    //  ワールド行列を計算する(オイラー)
    //--------------------------------------------------------------
    // 戻り値：ワールド行列
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& scale,
        const DirectX::XMFLOAT3& rotate, const DirectX::XMFLOAT3& trans)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(COORDINATE_SYSTEM::RHS_YUP, 1.0f)) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(trans.x, trans.y, trans.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);
        return world;
    }
    //--------------------------------------------------------------
    // ワールド行列を計算する(オイラーの親子関係)
    //--------------------------------------------------------------
    // 戻り値：親子関係を考慮したワールド行列
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& parent_scale,
        const DirectX::XMFLOAT3& parent_rotate, const DirectX::XMFLOAT3& parent_trans,
        const DirectX::XMFLOAT3& child_scale, const DirectX::XMFLOAT3& child_rotate,
        const DirectX::XMFLOAT3& child_trans)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(COORDINATE_SYSTEM::RHS_YUP, 1.0f)) };
        DirectX::XMMATRIX P_S{ DirectX::XMMatrixScaling(parent_scale.x, parent_scale.y, parent_scale.z) };
        DirectX::XMMATRIX P_R{ DirectX::XMMatrixRotationRollPitchYaw(parent_rotate.x, parent_rotate.y, parent_rotate.z) };
        DirectX::XMMATRIX P_T{ DirectX::XMMatrixTranslation(parent_trans.x, parent_trans.y, parent_trans.z) };

        DirectX::XMMATRIX C_S{ DirectX::XMMatrixScaling(child_scale.x, child_scale.y, child_scale.z) };
        DirectX::XMMATRIX C_R{ DirectX::XMMatrixRotationRollPitchYaw(child_rotate.x, child_rotate.y, child_rotate.z) };
        DirectX::XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_trans.x, child_trans.y, child_trans.z) };

        DirectX::XMMATRIX P_W = P_S * P_R * P_T;
        DirectX::XMMATRIX C_W = C_S * C_R * C_T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * C_W * P_W);

        return world;
    }
    //--------------------------------------------------------------
    //  ワールド行列を計算する(クォータニオン)
    //--------------------------------------------------------------
    // 戻り値：ワールド行列
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& scale,
        const DirectX::XMFLOAT4& orien, const DirectX::XMFLOAT3& trans)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(COORDINATE_SYSTEM::RHS_YUP, 1.0f)) };
        DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
        DirectX::XMMATRIX R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&orien)) };
        DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(trans.x, trans.y, trans.z) };
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * S * R * T);

        return world;
    }
    //--------------------------------------------------------------
    //  ワールド行列を計算する(クォータニオンの親子関係)
    //--------------------------------------------------------------
    // 戻り値：親子関係を考慮したワールド行列
    //--------------------------------------------------------------
    inline auto calc_world_matrix(const DirectX::XMFLOAT3& parent_scale,
        const DirectX::XMFLOAT4& parent_orien, const DirectX::XMFLOAT3& parent_trans,
        const DirectX::XMFLOAT3& child_scale, const DirectX::XMFLOAT4& child_orien,
        const DirectX::XMFLOAT3& child_trans)
    {
        DirectX::XMMATRIX C{ DirectX::XMLoadFloat4x4(&conversion_coordinate_system(COORDINATE_SYSTEM::RHS_YUP, 1.0f)) };
        DirectX::XMMATRIX P_S{ DirectX::XMMatrixScaling(parent_scale.x, parent_scale.y, parent_scale.z) };
        DirectX::XMMATRIX P_R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&parent_orien)) };
        DirectX::XMMATRIX P_T{ DirectX::XMMatrixTranslation(parent_trans.x, parent_trans.y, parent_trans.z) };

        DirectX::XMMATRIX C_S{ DirectX::XMMatrixScaling(child_scale.x, child_scale.y, child_scale.z) };
        DirectX::XMMATRIX C_R{ DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&child_orien)) };
        DirectX::XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_trans.x, child_trans.y, child_trans.z) };

        DirectX::XMMATRIX P_W = P_S * P_R * P_T;
        DirectX::XMMATRIX C_W = C_S * C_R * C_T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, C * C_W * P_W);

        return world;
    }
    //--------------------------------------------------------------
    //  親子関係を考慮したpositionを算出
    //--------------------------------------------------------------
    inline auto calc_world_position(const DirectX::XMFLOAT3& parent_pos, const DirectX::XMFLOAT3& child_pos)
    {
        using namespace DirectX;
        XMMATRIX P_T{ DirectX::XMMatrixTranslation(parent_pos.x, parent_pos.y, parent_pos.z) };
        XMMATRIX C_T{ DirectX::XMMatrixTranslation(child_pos.x, child_pos.y, child_pos.z) };
        XMFLOAT4X4 m4x4 = {};
        XMStoreFloat4x4(&m4x4, C_T * P_T);

        return XMFLOAT3(m4x4._41, m4x4._42, m4x4._43);
    }
    //--------------------------------------------------------------
    // 点(x, y)が点(cx, cy)を中心に角(angle)で回転した時の座標を算出
    //--------------------------------------------------------------
    inline void rotate(float& x, float& y, float cx, float cy, float angle)
    {
        x -= cx;
        y -= cy;

        float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
        float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
        float tx{ x }, ty{ y };
        x = cos * tx + -sin * ty;
        y = sin * tx + cos * ty;

        x += cx;
        y += cy;
    }
    //-------------------------------------------------------
    //  線形補完(float3)
    //-------------------------------------------------------
    //  戻り値：補完された値
    //-------------------------------------------------------
    inline const DirectX::XMFLOAT3 lerp(const DirectX::XMFLOAT3& start,
        const DirectX::XMFLOAT3& end, float lerp_rate)
    {
        using namespace DirectX;
        XMVECTOR start_vec = XMLoadFloat3(&start);
        XMVECTOR end_vec = XMLoadFloat3(&end);
        XMFLOAT3 lerp;
        XMStoreFloat3(&lerp, XMVectorLerp(start_vec, end_vec, lerp_rate));
        return lerp;
    }
    //-------------------------------------------------------
    //  線形補完(float2)
    //-------------------------------------------------------
    //  戻り値：補完された値
    //-------------------------------------------------------
    inline const DirectX::XMFLOAT2 lerp(const DirectX::XMFLOAT2& start,
        const DirectX::XMFLOAT2& end, float lerp_rate)
    {
        using namespace DirectX;
        XMVECTOR start_vec = XMLoadFloat2(&start);
        XMVECTOR end_vec = XMLoadFloat2(&end);
        XMFLOAT2 lerp;
        XMStoreFloat2(&lerp, XMVectorLerp(start_vec, end_vec, lerp_rate));
        return lerp;
    }
    //-------------------------------------------------------
    //  線形補完(float)
    //-------------------------------------------------------
    //  戻り値：保管された値
    //-------------------------------------------------------
    inline float lerp(const float start, const float end, float lerp_rate)
    {
        using namespace DirectX;
        XMVECTOR start_vec = XMLoadFloat(&start);
        XMVECTOR end_vec = XMLoadFloat(&end);
        float lerp;
        XMStoreFloat(&lerp, XMVectorLerp(start_vec, end_vec, lerp_rate));
        return lerp;
    }

    inline DirectX::XMFLOAT4 lerp(const DirectX::XMFLOAT4& begin,
        const DirectX::XMFLOAT4& end,float thread)
    {
        auto res = DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&begin),
            DirectX::XMLoadFloat4(&end), thread);
        DirectX::XMFLOAT4 s{};
        DirectX::XMStoreFloat4(&s, res);
        return s;
    }

    //-------------------------------------------------------
    //  二点からベクトルを算出する
    //-------------------------------------------------------
    //  戻り値：aからbに向かうベクトル
    //-------------------------------------------------------
    inline const DirectX::XMVECTOR calc_vector_AtoB(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR a_vec = DirectX::XMLoadFloat3(&a);
        XMVECTOR b_vec = DirectX::XMLoadFloat3(&b);
        XMVECTOR vec = b_vec - a_vec;
        return vec;
    }
    //-------------------------------------------------------
    //  二点からベクトルを算出する
    //-------------------------------------------------------
    //  戻り値：aからbに向かうベクトル
    //-------------------------------------------------------
    inline const DirectX::XMVECTOR calc_vector_AtoB(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        using namespace DirectX;
        XMVECTOR a_vec = DirectX::XMLoadFloat2(&a);
        XMVECTOR b_vec = DirectX::XMLoadFloat2(&b);
        XMVECTOR vec = b_vec - a_vec;
        return vec;
    }
    //--------------------------------------------------------------
    //  二点からベクトルを算出する(正規化)
    //--------------------------------------------------------------
    //  戻り値：aからbに向かう正規化されたベクトル
    //--------------------------------------------------------------
    inline const DirectX::XMVECTOR calc_vector_AtoB_normalize(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return DirectX::XMVector3Normalize(calc_vector_AtoB(a, b));
    }
    //--------------------------------------------------------------
    //  二点からベクトルを算出する(正規化)
    //--------------------------------------------------------------
    //  戻り値：aからbに向かう正規化されたベクトル
    //--------------------------------------------------------------
    inline const DirectX::XMVECTOR calc_vector_AtoB_normalize(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        return DirectX::XMVector2Normalize(calc_vector_AtoB(a, b));
    }
    //--------------------------------------------------------------
    //  二点からベクトルを算出し、その長さを計算する(2乗)
    //--------------------------------------------------------------
    //  戻り値：aからbに向かうベクトルの長さ
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length_sq(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR length_sq_vec = XMVector3LengthSq(calc_vector_AtoB(a, b));
        float length_sq;
        XMStoreFloat(&length_sq, length_sq_vec);
        return length_sq;
    }
    //--------------------------------------------------------------
    //  二点からベクトルを算出し、その長さを計算する(2乗)
    //--------------------------------------------------------------
    //  戻り値：aからbに向かうベクトルの長さ
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length_sq(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        using namespace DirectX;
        XMVECTOR length_sq_vec = XMVector2LengthSq(calc_vector_AtoB(a, b));
        float length_sq;
        XMStoreFloat(&length_sq, length_sq_vec);
        return length_sq;
    }
    //--------------------------------------------------------------
    //  二点からベクトルを算出し、その長さを計算する
    //--------------------------------------------------------------
    //  戻り値：aからbに向かうベクトルの長さ
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        using namespace DirectX;
        XMVECTOR length_vec = XMVector3Length(calc_vector_AtoB(a, b));
        float length;
        XMStoreFloat(&length, length_vec);
        return length;
    }
    //--------------------------------------------------------------
    //  二点からベクトルを算出し、その長さを計算する
    //--------------------------------------------------------------
    //  戻り値：aからbに向かうベクトルの長さ
    //--------------------------------------------------------------
    inline float calc_vector_AtoB_length(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        using namespace DirectX;
        XMVECTOR length_vec = XMVector2Length(calc_vector_AtoB(a, b));
        float length;
        XMStoreFloat(&length, length_vec);
        return length;
    }
    //--------------------------------------------------------------
    //  指定方向に指定距離進んだ点を算出する
    //--------------------------------------------------------------
    //  戻り値：startからdirection方向にlength分進んだ地点
    //--------------------------------------------------------------
    inline const DirectX::XMFLOAT3 calc_designated_point(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& direction, float length)
    {
        return { start.x + direction.x * length, start.y + direction.y * length, start.z + direction.z * length };
    }
    //--------------------------------------------------------------
    //  指定方向に指定距離進んだ点を算出する
    //--------------------------------------------------------------
    //  戻り値：startからdirection方向にlength分進んだ地点
    //--------------------------------------------------------------
    inline const DirectX::XMFLOAT2& calc_designated_point(const DirectX::XMFLOAT2& start,
        const DirectX::XMFLOAT2& direction, float length)
    {
        return { start.x + direction.x * length, start.y + direction.y * length };
    }
    //--------------------------------------------------------------
    //  線分 start end と点p0の最近点を算出する
    //--------------------------------------------------------------
    //  戻り値：線分 start end と点p0の最近点
    //--------------------------------------------------------------
    inline const DirectX::XMFLOAT3& calc_closest_point(const DirectX::XMFLOAT3& start,
        const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT3& p0)
    {
        using namespace DirectX;
        XMVECTOR v0_vec = XMLoadFloat3(&end) - XMLoadFloat3(&start);
        XMVECTOR v1_vec = XMLoadFloat3(&p0) - XMLoadFloat3(&start);
        XMVECTOR projection_length_vec = XMVector3Dot(v1_vec, XMVector3Normalize(v0_vec));
        float projection_length;
        XMStoreFloat(&projection_length, projection_length_vec);
        XMFLOAT3 v0_norm;
        XMStoreFloat3(&v0_norm, XMVector3Normalize(v0_vec));
        return calc_designated_point(start, v0_norm, projection_length);
    }

    //--------------------------------------------------------------
    //  ベクトルを正規化する
    //--------------------------------------------------------------
    //  戻り値：長さが１のベクトル
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT3 Normalize(DirectX::XMFLOAT3 V_)
    {
        auto V = DirectX::XMLoadFloat3(&V_);
        V = DirectX::XMVector3Normalize(V);
        DirectX::XMFLOAT3 ret{};
        DirectX::XMStoreFloat3(&ret, V);
        return ret;
    }

    //--------------------------------------------------------------
    //  外積
    //--------------------------------------------------------------
    //
    //  引数：ベクトル、ベクトル、正規化するかどうか
    //
    //--------------------------------------------------------------
    //  戻り値：ベクトル
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT3 Cross(DirectX::XMFLOAT3 A_, DirectX::XMFLOAT3 B_, bool IsNormalize_ = true)
    {
        const auto VA = DirectX::XMLoadFloat3(&A_);
        const auto VB = DirectX::XMLoadFloat3(&B_);
        auto Cross = DirectX::XMVector3Cross(VA, VB);
        if (IsNormalize_)
        {
            Cross = DirectX::XMVector3Normalize(Cross);
        }
        DirectX::XMFLOAT3 cross{};
        DirectX::XMStoreFloat3(&cross, Cross);
        return cross;
    }

    //--------------------------------------------------------------
    //  内積
    //--------------------------------------------------------------
    //
    //  引数：ベクトル、ベクトル
    //
    //--------------------------------------------------------------
    //  戻り値：角度（ラジアン）
    //--------------------------------------------------------------
    inline float Dot(const DirectX::XMFLOAT3 A, const DirectX::XMFLOAT3 B)
    {
        const DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&A);
        const DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&B);
        const DirectX::XMVECTOR Ans = DirectX::XMVector3Dot(V0, V1);
        float ans{};
        DirectX::XMStoreFloat(&ans, Ans);
        return ans;
    }
    //--------------------------------------------------------------
    //  `ベクトルの長さを計算する
    //--------------------------------------------------------------
    //
    //  引数：ベクトル
    //
    //--------------------------------------------------------------
    //  戻り値：長さ
    //--------------------------------------------------------------
    inline float Length(DirectX::XMFLOAT3 V_)
    {
        auto VL = DirectX::XMLoadFloat3(&V_);
        VL = DirectX::XMVector3Length(VL);
        float ret;
        DirectX::XMStoreFloat(&ret, VL);
        return ret;
    }
    //--------------------------------------------------------------
    //  クォータニオン回転
    //--------------------------------------------------------------
    //
    //  引数：現在の回転姿勢、軸、回転角
    //
    //--------------------------------------------------------------
    //  戻り値：回転後の姿勢
    //--------------------------------------------------------------
    inline DirectX::XMFLOAT4 RotQuaternion(DirectX::XMFLOAT4 Orientation_,DirectX::XMFLOAT3 Axis_,float Radian_)
    {

        if (fabs(Radian_) > 1e-8f)
        {
            // 変換
            const auto Axis = DirectX::XMLoadFloat3(&Axis_);
            auto oriV = DirectX::XMLoadFloat4(&Orientation_);

            // 回転クォータニオンを算出
            const auto rotQua = DirectX::XMQuaternionRotationAxis(Axis, Radian_);
            oriV = DirectX::XMQuaternionMultiply(oriV, rotQua);

            DirectX::XMFLOAT4 ret{};
            DirectX::XMStoreFloat4(&ret, oriV);
            return ret;
        }
        return Orientation_;
    }


    inline DirectX::XMFLOAT3 HermiteFloat3(std::vector<DirectX::XMFLOAT3>& controllPoints, float ratio)
    {
        using namespace DirectX;
        const size_t size = controllPoints.size();

        if (size == 0)assert("Not ControllPoint");
        if (size == 1)return controllPoints.at(0);

        //-----< 今どこの区間にいるか >-----//
        std::vector<float> sectionLength;
        float length = 0.0f;
        //各区間の長さを求める
        for (int i = 0; i < size - 1; i++)
        {
            const DirectX::XMFLOAT3 v = {
        controllPoints.at(i + 1).x - controllPoints.at(i).x,
        controllPoints.at(i + 1).y - controllPoints.at(i).y,
        controllPoints.at(i + 1).z - controllPoints.at(i).z };
            const float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

            sectionLength.emplace_back(l);
            length += l;
        }
        std::vector<float> sections;
        //各区間の割合を求める
        for (int i = 0; i < size - 1; i++)
        {
            sections.emplace_back(sectionLength.at(i) / length);
        }

        int sectionNum = 0;
        float r = 0.0f;
        for (sectionNum; sectionNum < sections.size(); sectionNum++)
        {
            r += sections.at(sectionNum);
            if ((ratio - r) <= 0.00001f)
            {
                break;
            }
        }

        XMFLOAT3 dummy = {
            2.0f * controllPoints.at(0).x - controllPoints.at(1).x,
            2.0f * controllPoints.at(0).y - controllPoints.at(1).y,
            2.0f * controllPoints.at(0).z - controllPoints.at(1).z,
        };
        const auto it = controllPoints.begin();
        controllPoints.insert(it, dummy);
        dummy = {
        2.0f * controllPoints.at(size - 1).x - controllPoints.at(size - 2).x,
        2.0f * controllPoints.at(size - 1).y - controllPoints.at(size - 2).y,
        2.0f * controllPoints.at(size - 1).z - controllPoints.at(size - 2).z,
        };
        controllPoints.emplace_back(dummy);

        sectionNum++;
        float sectionRatio = (ratio - (r - sections.at(sectionNum - 1))) / sections.at(sectionNum - 1);

        XMVECTOR Out;

        const float power = 1.0f; // Usually power is 0.5f
        XMVECTOR P0 = XMLoadFloat3(&controllPoints.at(sectionNum - 1));
        XMVECTOR P1 = XMLoadFloat3(&controllPoints.at(sectionNum + 0));
        XMVECTOR P2 = XMLoadFloat3(&controllPoints.at(sectionNum + 1));
        XMVECTOR P3 = XMLoadFloat3(&controllPoints.at(sectionNum + 2));
        XMVECTOR V0 = XMVectorScale(XMVectorSubtract(P2, P0), power);
        XMVECTOR V1 = XMVectorScale(XMVectorSubtract(P3, P1), power);

        Out = powf(sectionRatio, 3.0f) * (2.0f * P1 - 2.0f * P2 + V0 + V1);
        Out += powf(sectionRatio, 2.0f) * (-3.0f * P1 + 3.0f * P2 - 2.0f * V0 - V1);
        Out += sectionRatio * V0 + P1;


        XMFLOAT3 out{};
        XMStoreFloat3(&out, Out);

        return out;
    }


    inline DirectX::XMFLOAT3 GetUp(const DirectX::XMFLOAT4& Orientation_)
    {
        using namespace DirectX;
        XMFLOAT3 up{};
        const XMVECTOR orientation_vec{ XMLoadFloat4(&Orientation_) };
        const DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);
        const DirectX::XMVECTOR up_vec = { m4x4._21, m4x4._22, m4x4._23 };
        XMStoreFloat3(&up, up_vec);
        return up;
    }
    inline DirectX::XMFLOAT3 GetFront (const DirectX::XMFLOAT4& Orientation_)
    {
        using namespace DirectX;
        XMFLOAT3 up{};
        const XMVECTOR orientation_vec{ XMLoadFloat4(&Orientation_) };
        const DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(orientation_vec);
        DirectX::XMFLOAT4X4 m4x4 = {};
        DirectX::XMStoreFloat4x4(&m4x4, m);
        const DirectX::XMVECTOR up_vec = { m4x4._31, m4x4._32, m4x4._33 };
        XMStoreFloat3(&up, up_vec);
        return up;
    }

    inline float Saturate(float Resource_,float Min_=0.0f,float Max_=1.0f)
    {
        _ASSERT_EXPR(Max_ > Min_, "最大値が最小値よりも小さい");
        return  (std::max)((std::min)(Resource_, Max_), Min_);
    }

    inline DirectX::XMFLOAT3 fBezierCurve(
        const DirectX::XMFLOAT3& Point0,
        const DirectX::XMFLOAT3& Point1,
        const DirectX::XMFLOAT3& Point2,
        float Threshold
    )
    {
        const DirectX::XMFLOAT3 p0 = Math::lerp(Point0, Point1, Threshold);
        const DirectX::XMFLOAT3 p1 = Math::lerp(Point1, Point2, Threshold);

        return Math::lerp(p0, p1, Threshold);
    }
}

//--------------------------------------------------------------
//  strBit16    整数を2進数（16bit）のstringに変換する
//--------------------------------------------------------------
//  　引数：const int n     変換する整数
//  戻り値：std::string     数値を2進数に変換したもの(string)
//--------------------------------------------------------------
inline std::string strBit16(const int n)
{
    std::stringstream ss;
    ss << static_cast<std::bitset<16>>(n);
    return ss.str();
}

// imgui Menu
inline void imgui_menu_bar(std::string menu_label, std::string menu_item_label, bool& selected)
{
#ifdef USE_IMGUI
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(menu_label.c_str()))
        {
            ImGui::MenuItem(menu_item_label.c_str(), NULL, &selected);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
#endif // USE_IMGUI
}

template <typename T>
inline void safe_delete(T*& p)
{
    if (p != nullptr)
    {
        delete(p);
        (p) = nullptr;
    }
}

template<typename T>
inline void safe_delete_array(T*& p)
{
    if (p != nullptr)
    {
        delete[](p);
        (p) = nullptr;
    }
}

template <typename T>
inline void safe_release(T*& p)
{
    if (p != nullptr)
    {
        (p)->Release();
        (p) = nullptr;
    }
}


class TimerComponent final
{
public:
    void StartTimer(float LimitTimer_)
    {
        mStackTimer = 0.0f;
        mLimitTime = LimitTimer_;
    }
    void fUpdate(float elapsedTime_)
    {
        mStackTimer += elapsedTime_;
    }
    [[nodiscard]] bool fGetOver()const
    {
        return mStackTimer > mLimitTime;
    }
private:
    float mStackTimer{};
    float mLimitTime{};
};
