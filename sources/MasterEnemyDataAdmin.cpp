#include "MasterEnemyDataAdmin.h"

MasterEnemyDataAdmin::~MasterEnemyDataAdmin()
{
}

void MasterEnemyDataAdmin::SetMasterData(int group, DirectX::XMFLOAT3 pos, int target_id, int ai_state)
{
    MasterDataMap::iterator it;
    //-----�}�b�v�̒��Ɉ����Ɠ����L�[�l�����邩�ǂ���������-----//
    it = master_data_map.find(group);

    //-----������Ȃ������ꍇ-----//
    if (it == master_data_map.end())
    {
        //-----�}�X�^�[�f�[�^��ݒ肵�ēo�^-----//
        MasterData data;

        data.position = pos;
        data.target_id = target_id;
        data.ai_state = ai_state;

        master_data_map.insert(std::make_pair(group, data));
    }
    //-----���������ꍇ-----//
    else
    {
        //-----�f�[�^���X�V-----//
        it->second.position = pos;
        it->second.target_id = target_id;
        it->second.ai_state = ai_state;
    }
}

std::tuple<bool, MasterEnemyDataAdmin::MasterData> MasterEnemyDataAdmin::GetMasterData(int group)
{
    MasterDataMap::iterator it;
    //-----�}�b�v�̒��Ɉ����Ɠ����L�[�l�����邩�ǂ���������-----//
    it = master_data_map.find(group);
    MasterData data{};

    //-----������Ȃ������ꍇ-----//
    if (it == master_data_map.end())
    {
        //-----false��ݒ肵�āA�Ȃ������_�~�[����Ԃ�-----//
        return std::tuple<bool, MasterData>(false,data);
    }
    //-----���������ꍇ-----//
    else
    {
        //-----true��ݒ肵�āA����Ԃ�-----//
        return std::tuple<bool, MasterData>(true,it->second);
    }

}

void MasterEnemyDataAdmin::ResetMasterData()
{
    master_data_map.clear();
}
