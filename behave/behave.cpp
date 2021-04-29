#include <vector>
#include "../WebRobot.hpp"
#include "../libcommon/protocol/LxGame.pb.h"
#include "../libcommon/protocol/LxGameEnums.h"
#include "../libcommon/protocol/LxGameHelper.h"
#include "../libcommon/ThreadLog/ThreadLog.h"
#include "../libcommon/Utils/GlobalUtils.h"
using namespace std;
using namespace google::protobuf;
namespace robot{
void WebRobot::Login()
{
    LxGame::WrapPacket packet;
    LxGame::UserLoginReq req;
    req.set_account(account_);
    req.set_token(token_);
    req.set_login_type(1);
    LxGameHelper::MakeUserLoginReq(packet, req);
    SendMessage(packet);
}

void WebRobot::handleLoginRes(const LxGame::WrapPacket& packet)
{
    LxGame::UserLoginResp user_info;
    if(!user_info.ParseFromString(packet.data()) )
    {
        LOGINFO("failed to login");
        assert(false);
        return;
    }

    int32 error_code = packet.errorcode();
    UpdateRoomFlag(error_code? false: true);
    do{
        if(!error_code)
        {
            LOGINFO("login ok. %s", user_info.DebugString().data());
            user_info_ = user_info.user_info();
            UpdateStatus(STATUS_LOGINED);
            break;
        }
        LOGERROR("login error is [%d]", error_code);
    }while(0);
}
void WebRobot::EnterRoom( int32 room_index)
{
    if(room_index > 0)
    {
        LOGINFO("LogicRobot::EnterRoom");
        LxGame::WrapPacket packet;
        LxGame::FishEnterRoomReq req;
        req.set_room_index(room_index);
        LxGameHelper::MakeFishEnterRoomReq(packet, req);
        SendMessage(packet);
    }
}

void WebRobot::handleEnterRoomRes(const LxGame::WrapPacket& packet)
{
    int turret_rate_index = -1;
    int turret_rate = -1;
    LxGame::FishEnterRoomResp resp;
    if(!resp.ParseFromString(packet.data()))
    {
        LOGINFO("failed to enter room");
        return;
    }

    if(packet.errorcode())
    {
        LOGERROR("enter room packet parse error###########");
        return;
    }
    room_config_infos_.Clear();
    room_config_infos_ = resp.roominfo();

    LOGINFO("enter room ok. %s", room_config_infos_.DebugString().data());
    do{
        if(!packet.errorcode())
        {
            b_inRoom_ = true;
            // 第一个进入桌子（无论哪种测试情况都是随机一个炮台来做的）
            if(room_config_infos_.players_size() == 1)
            {
                //随机炮倍 测试1个人时所有用户炮倍随机，测试二三四(N-k)人时随机
                randomTurretRate(&turret_rate_index, &turret_rate);
                // LOGINFO("++++!!!");
                break;
            }

            //这个判断只会在测试2,3,4人时进入，作用：确保同一个房间所有用户炮倍相同（1000为K）
            //服务端同步发的是所有player包括了自己
            if(room_config_infos_.players_size() > 1)
            {
                // 使用桌子中其他人的炮倍
                for(int player_index = 0; player_index < room_config_infos_.players_size(); ++player_index)
                {
                    const FishPlayerProto& player_config = room_config_infos_.players(player_index);
                    if(player_config.user_id() != user_info_.user_id())
                    {
                        turret_rate_index = player_config.turretrateindex();
                        turret_rate = 1;
                        break;
                    }
                }
                break;

            }

            if(room_config_infos_.players_size() == 0)
            {
                LOGERROR("error ,unexpected");
                break;
            }
        }
    }while(0);

    if(turret_rate_index >= 0 && turret_rate >= 0)
    {
        // LOGINFO("TSET_RATE client_id [%d], room [%d], turret_rate_index [%d]", _client->GetClientId(), _room, turret_rate_index);
        changeTurretRate(turret_rate_index, turret_rate);
    }
}

void WebRobot::handleCreatedFish(const LxGame::WrapPacket& packet)
{
    LxGame::SyncCreateFish resp;
    if( !resp.ParseFromString(packet.data()))
    {
        LOGINFO("failed to create fish");
    }
    else
    {
        fishes_[resp.id()] = GlobalUtils::GetTickCount() + resp.lifetick();
    }
}


void WebRobot::handlecUserInfoChange(const LxGame::WrapPacket& packet)
{
    LxGame::SyncUserInfoChange msg;
    if( !msg.ParseFromString(packet.data()) )
    {
        LOGINFO("userinfochange failed");
        return;
    }
    // LOGINFO("info change [%s]", msg.DebugString().data());
    for(int32 i = 0; i < msg.fields_size(); ++i)
    {
        const std::string &field_name = msg.fields(i).field();
        const std::string &field_value =msg.fields(i).value();
        do{
            if("user_gold" == field_name)
            {
                user_info_.set_user_gold(atoi(field_value.c_str()));
                break;
            }

            if("user_diamond" == field_name)
            {
                user_info_.set_user_diamond(atoi(field_value.c_str()));
                break;
            }

            if("user_vip" == field_name)
            {
                user_info_.set_user_vip(atoi(field_value.c_str()));
                break;
            }

            // 当前装备炮台
            if("user_turret_id" == field_name)
            {
                user_info_.set_user_turret_id(atoi(field_value.c_str()));
                break;
            }
            //上一次鱼潮的等级
            if("user_last_tide" == field_name)
            {
                user_info_.set_user_last_tide(atoi(field_value.c_str()));
                break;
            }

            // 成长基金是否可以领取
            if("user_growth_fund" == field_name)
            {
                user_info_.set_user_growth_fund(atoi(field_value.c_str()));
                break;
            }

            // 累计游戏时长礼包
            if("game_time_status" == field_name)
            {
                user_info_.set_game_time_status(atoi(field_value.c_str()));
                break;
            }

            // 贫困奖励的次数
            if("poverty_num" == field_name)
            {
                user_info_.set_poverty_num(atoi(field_value.c_str()));
                break;
            }

            // 当前table结束时间
            if("fish_table_end_time" == field_name)
            {
                user_info_.set_fish_table_end_time(atoi(field_value.c_str()));
                break;
            }

            //总游戏时长
            if("total_game_time" == field_name)
            {
                user_info_.set_time_game(atoi(field_value.c_str()));
                break;
            }

        }while(0);
    }

    //delete useless info in local
    for(int i = 0; i < msg.del_turrets_size(); ++i)
    {
        auto it = turret_list_.find(msg.del_turrets(i));
        if(it != turret_list_.end())
        {
            turret_list_.erase(it);
        }
    }

    for(int i = 0; i < msg.del_swings_size(); ++i)
    {
        auto it = swings_list_.find(msg.del_swings(i));
        if(it != swings_list_.end())
        {
            swings_list_.erase(it);
        }
    }

    for(int i = 0; i < msg.del_quests_size(); ++i)
    {
        auto it = task_list_.find(msg.del_quests(i));
        if(it != task_list_.end())
        {
            task_list_.erase(it);
        }
    }

    //更新本地数据（这里暂时只更新了目前会用到的一些数据）
    for(int i = 0; i < msg.items_size(); ++i)
    {
        auto it = item_list_.find(msg.items(i).item_id());
        if(it != item_list_.end())
        {
            it->second.CopyFrom(msg.items(i));
        }
    }

    for(int i = 0; i < msg.quests_size(); ++i)
    {
        auto it = task_list_.find(msg.quests(i).quest_id());
        if(it != task_list_.end())
        {
            it->second.CopyFrom(msg.quests(i));
        }
    }

    for(int i = 0; i < msg.turrets_size(); ++i)
    {
        auto it = turret_list_.find(msg.turrets(i).turret_id());
        if(it != turret_list_.end())
        {
            it->second.CopyFrom(msg.turrets(i));
        }
    }

    for(int i = 0; i < msg.swings_size(); ++i)
    {
        auto it = swings_list_.find(msg.swings(i).swing_id());
        if(it != swings_list_.end())
        {
            it->second.CopyFrom(msg.swings(i));
        }
    }
}

void WebRobot::handleHittedFishRes(const LxGame::WrapPacket& packet)
{
    LxGame::ActHitFish resp;
    if( !resp.ParseFromString(packet.data()) )
    {
        LOGINFO("failed to create fish");
    }
    else
    {
        fishes_.erase(resp.fishid());
    }
}

void WebRobot::recharge(const std::string& product_name)
{
    LxGame::UserChargeReq msg;
    msg.set_product(product_name.c_str());
    LxGame::WrapPacket packet;
    LxGameHelper::MakeUserChargeReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::doRecharge()
{
    // 充值648鑽石
    recharge("12");
    // 获取急速技能
    LxGame::WrapPacket packet;
    LxGame::ShowMeTheMoney GmPacket;
    GmPacket.set_gold(50000);
    GmPacket.set_item_id(11401003);
    GmPacket.set_item_num(50);
    LxGameHelper::MakeShowMeTheMoney(packet, GmPacket);
    SendMessage(packet);
}

void WebRobot::handleRechargeRes(const LxGame::WrapPacket& packet)
{
    LxGame::UserChargeResp resp;
    do{
        if (!resp.ParseFromString(packet.data()))
        {
            break;
        }

        if (packet.errorcode())
        {
            LOGERROR("ERROR,errocode[%d]", packet.errorcode());
            break;
        }

        if(resp.has_product())
        {
            LOGINFO("buy product[%s] successfully", resp.product().DebugString().c_str());
        }
    }while(0);
}

void WebRobot::getPropList()
{
    LOGINFO("LogicRobot::getPropList");
    LxGame::UserItemReq msg;
    LxGame::WrapPacket packet;
    LxGameHelper::MakeUserItemReq(packet);
    SendMessage(packet);
}

void WebRobot::handlePropListlRes(const LxGame::WrapPacket& packet)
{
    LOGINFO("received cmd is[%d]", packet.cmd());
    LxGame::UserItemResp resp;
    do{
        if (!resp.ParseFromString(packet.data()))
        {
            break;
        }

        if (packet.errorcode())
        {
            LOGERROR("ERROR,errocode[%d]", packet.errorcode());
            break;
        }
        item_list_.clear();
        //store prop_list in local
        for(int i = 0; i < resp.items_size(); ++i)
        {
            LxGame::ItemPair item = resp.items(i);
            item_list_.insert(make_pair(item.item_id(), item));
            LOGINFO("prop_id is [%d]", item.item_id());
        }
    }while(0);
}

void WebRobot::useProp(int32 prop_id, int64 num)
{
    if(item_list_.find(prop_id) == item_list_.end())
    {
        LOGERROR("not find this prop, prop id is[%d]", prop_id);
        return;
    }
    LxGame::UserItemUseReq msg;
    LxGame::ItemPair* item_pair = msg.mutable_item();
    item_pair->set_item_id(prop_id);
    item_pair->set_item_num(num);
    LxGame::WrapPacket packet;
    LxGameHelper::MakeUserItemUseReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleUsePropRes(const LxGame::WrapPacket& packet)
{
    LxGame::UserItemUseResp resp;
    if (!resp.ParseFromString(packet.data()))
    {
        return;
    }
    int32 error_code = packet.errorcode();
    do{
        if(!error_code)
        {
            LOGINFO("use prop success");
            break;
        }
    }while(0);
}

// 经验豆的经验就不查表了
void WebRobot::actTurretExp()
{
    int64 turret_installed = user_info_.user_turret_id();
    // JDATA->TurretLevelPtr()->
    if(turret_list_.find(turret_installed) == turret_list_.end())
    {
        LOGERROR("turret equipped is wrong");
        assert(false);
        return;
    }

    const LxGame::TurretInfo& turret_equipped = turret_list_[turret_installed];
    int32 current_level_exp = turret_equipped.turret_level_exp();
    int32 current_level = turret_equipped.turret_level();
    int32 need_level_exp = 0;
    //使用
    if(item_list_.find(11101001) == item_list_.end())
    {
        LOGERROR("turret equipped is wrong");
        assert(false);
        return;
    }
    const LxGame::ItemPair& item_used_has  = item_list_[11101001];

    JDATA->TurretLevelPtr()->ForEach([&](tagJsonTurretLevel* data){
        if(data->_ID == (current_level/10+1)*10)
        {
            need_level_exp = data->_TotalExp - current_level_exp;
            return;
        }
    });
    int32 num_item_used  = need_level_exp/50;
    if(num_item_used > item_used_has.item_num())
    {
        //暂时就不处理了 data_read时会准备足够的量
        LOGERROR("item is not enough");
        assert(false);
        return;
    }

    //炮台升级
    std::vector <LxGame::ItemPair> comsumables;
    LxGame::ItemPair item_used;
    item_used.set_item_id(11101001);
    item_used.set_item_num(num_item_used + 1);
    comsumables.emplace_back(item_used);
    getTurretExpUp(turret_installed, comsumables);
}

// 	USER_TurretExpReq = 33555480, // 炮台升级
// USER_TurretExpResp = 33555481, // 炮台升级回包
void WebRobot::getTurretExpUp(int64 turret_id, std::vector<LxGame::ItemPair>& consumables)
{
    LOGINFO("getTurretExpUp");
    LxGame::TurretExpReq msg;
    msg.set_turret_id(turret_id);
    for(auto consumable : consumables)
    {
        LxGame::ItemPair* msg_p = msg.add_items();
        msg_p->CopyFrom(consumable);
    }

    LxGame::WrapPacket packet;
    LxGameHelper::MakeTurretExpReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleTurretExpUpRes(const LxGame::WrapPacket& packet)
{
    LxGame::TurretExpResp resp;
    int32 error_code = packet.errorcode();

    if (!resp.ParseFromString(packet.data()))
    {
        return;
    }

    if(error_code)
    {
        LOGERROR("ERROR,errocode[%d]", packet.errorcode());
    }

    do
    {
        if (!error_code)
        {
            LOGINFO("TurretExpUp successfully");
        }
    } while (0);
}

// 	USER_TurretStarExpReq = 33555482, // 炮台升星
// USER_TurretStarExpResp = 33555483, // 炮台升星回包
void WebRobot::getTurretStarUp(int64 turret_id, std::vector<LxGame::ItemPair>& consumables)
{
    LOGINFO("getTurretStarUp");
    LxGame::TurretStarExpReq msg;
    msg.set_turret_id(turret_id);
    for(auto consumable : consumables)
    {
        LxGame::ItemPair* msg_p = msg.add_items();
        msg_p->CopyFrom(consumable);
    }

    LxGame::WrapPacket packet;
    LxGameHelper::MakeTurretStarExpReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleTurretStarUpRes(const LxGame::WrapPacket& packet)
{
    LxGame::TurretStarExpResp resp;
    if (!resp.ParseFromString(packet.data()))
    {
        return;
    }

    int32 error_code = packet.errorcode();
    if (error_code)
    {
        LOGERROR("ERROR,errocode[%d]", error_code);
    }
    do
    {
        if(!error_code)
        {
            LOGINFO("TurretStarUp successfully");
        }
    } while (0);
}

// USER_TurretPlusReq = 33555484, // 炮台强化
// USER_TurretPlusResp = 33555485, // 炮台强化回包
void WebRobot::strengthenTurret(int64 turret_id)
{
    LOGINFO("strengthenTurret");
    LxGame::TurretPlusReq msg;
    msg.set_turret_id(turret_id);

    LxGame::WrapPacket packet;
    LxGameHelper::MakeTurretPlusReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleStrengthenTurretRes(const LxGame::WrapPacket& packet)
{
    LxGame::TurretPlusResp resp;
    int32 error_code = packet.errorcode();
    if (!resp.ParseFromString(packet.data()))
    {
        return;
    }

    if (error_code)
    {
        LOGERROR("ERROR,errocode[%d]", packet.errorcode());
    }
    do
    {
        if(!error_code)
        {
            LOGINFO("StrengthenTurret successfully");
        }
    } while (0);
}


// USER_TurretUseReq = 33555486, // 切换炮台
// USER_TurretUseResp = 33555487, // 切换炮台
void WebRobot::switchTurret(int64 turret_id)
{
    // LOGINFO("switchTurret");
    LxGame::TurretUseReq msg;
    msg.set_turret_id(turret_id);

    LxGame::WrapPacket packet;
    LxGameHelper::MakeTurretUseReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleSwitchTurretRes(const LxGame::WrapPacket& packet)
{
    LxGame::TurretUseResp resp;
    do
    {
        if (!resp.ParseFromString(packet.data()))
        {
            break;
        }

        if (packet.errorcode())
        {
            LOGERROR("ERROR,errocode[%d]", packet.errorcode());
            break;
        }

        LOGINFO("switch turret successfully");
    } while (0);
}


// 	USER_TurretEquipSwingReq = 33555488, // 炮台装备翅膀
// USER_TurretEquipSwingResp = 33555489, // 炮台装备翅膀回包
void WebRobot::equipSwing(int64 turret_id, int64 swings_id)
{
    LOGINFO("equipSwing");
    LxGame::TurretEquipSwingReq msg;
    msg.set_turret_id(turret_id);
    msg.set_swing_id(swings_id);

    LxGame::WrapPacket packet;
    LxGameHelper::MakeTurretEquipSwingReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleEquipSwing(const LxGame::WrapPacket& packet)
{
    LxGame::TurretEquipSwingResp resp;
    do
    {
        if (!resp.ParseFromString(packet.data()))
        {
            break;
        }

        if (!packet.errorcode())
        {
            LOGINFO("handleEquipSwing successfully");
            break;
        }

    } while (0);
}


// USER_SwingUpgradeReq = 33555490, // 翅膀进阶
// USER_SwingUpgradeResp = 33555491, // 翅膀进阶回包
void WebRobot::getSwingLevelUp(int64 swing_id, int64 item_id, const std::vector<int64>& swings_ids_consumed)
{
    LOGINFO("SwingLevelUp");
    LxGame::SwingUpgradeReq msg;
    msg.set_swing_id(swing_id);
    //消息体中item为法宝，羽之精粹不需要填，服务端自行检测
    // msg.set_item_id(item_id);
    for(auto& swing : swings_ids_consumed)
    {
        msg.add_swings(swing);
    }
    LxGame::WrapPacket packet;
    LxGameHelper::MakeSwingUpgradeReq(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleGetSwingLevelUpRes(const LxGame::WrapPacket& packet)
{
    // 这里error_code
    LxGame::SwinUpgradeResp resp;
    if (!resp.ParseFromString(packet.data()))
    {
        LOGERROR("parse error");
        assert(false);
        return;
    }

    int32 error_code = packet.errorcode();
    if (packet.errorcode())
    {
        LOGERROR("ERROR,errocode[%d]", packet.errorcode());
    }

    do
    {
        if(e_jsonErrorCodeID_SwingUpgradeRateFailed == error_code)
        {
            LOGERROR("unluckily , fail to upgrade ");

            for(std::map<int32, LxGame::SwingInfo>::iterator it = swings_list_.begin(); it != swings_list_.end(); ++it)
            {
                const LxGame::SwingInfo& swing_info = it->second;
                if(swing_info.swing_turret_id() == user_info_.user_turret_id()
                    &&swing_info.swing_phase() < _equips_task._swings_level)
                {
                    doSwingLevelUp(swing_info.swing_id());
                    break;
                }
            }
        }

        if(e_jsonErrorCodeID_SwingUpgradeSwingFailed == error_code)
        {
            LOGERROR("unluckily , fail to upgrade ");
            task_one();
            for(std::map<int32, LxGame::SwingInfo>::iterator it = swings_list_.begin(); it != swings_list_.end(); ++it)
            {
                const LxGame::SwingInfo& swing_info = it->second;
                if(swing_info.swing_turret_id() == user_info_.user_turret_id()
                    &&swing_info.swing_phase() < _equips_task._swings_level)
                {
                    doSwingLevelUp(swing_info.swing_id());
                    break;
                }
            }
            return;
        }

        for(std::map<int32, LxGame::SwingInfo>::iterator it = swings_list_.begin(); it != swings_list_.end(); ++it)
        {
            const LxGame::SwingInfo& swing_info = it->second;
            if(swing_info.swing_turret_id() == user_info_.user_turret_id()
                && swing_info.swing_phase() < _equips_task._swings_level)
            {
                doSwingLevelUp(swing_info.swing_id());
                break;
            }
        }
    } while (0);
}

void WebRobot::doSwingLevelUp(int64 swing_id)
{   
    std::map<int32, LxGame::SwingInfo>::iterator iter = swings_list_.find(swing_id);
    if(iter == swings_list_.end())
    {
        LOGERROR("swing is not exit");
        assert(false);
        return;
    }
    const LxGame::SwingInfo& swing_info = iter->second;
    int32 current_phase = swing_info.swing_phase();
    int32 consumed_item_id = 0;
    int32 consumed_swing_index = 0;
    int32 consumed_swing_num = 0;
    JDATA->SwingUpgradePtr()->ForEachWithBreak([&](tagJsonSwingUpgrade* data){
        if(data->_SwingID == swing_info.swing_index() && data->_PhaseID == current_phase)
        {
            consumed_item_id = data->_UpgradeItemID;
            consumed_swing_index = data->_UpgradeSwingID;
            consumed_swing_num = data->_UpgradeSwingNum;
            return true;
        }
        return false;
    });

    if(current_phase < _equips_task._swings_level)
    {
        std::vector<int64> swings_ids_consumed;
        int32 consumable_num = 0;
        for(auto& swing_pair : swings_list_)
        {
            LxGame::SwingInfo swing = swing_pair.second;
            int32 swing_index = swing.swing_index();
            if(swing_index == consumed_swing_index 
                && swing.swing_id() != swing_id 
                && swing.swing_turret_id() == 0
                && swing.swing_phase() == 0)
            {
                ++consumable_num;
                swings_ids_consumed.emplace_back(swing.swing_id());
            }

            if(consumable_num == consumed_swing_num)
            {
                break;
            }
        }

        // 这个判断没必要，因为目前不传入法宝，而羽之精粹 也不需要发给服务端
        if(consumed_item_id)
        {
            //使用道具羽之精粹+翅膀
            getSwingLevelUp(swing_id, consumed_item_id, swings_ids_consumed);
        }
    }
}

// FISH_InputChangeRate = 50332668, // 炮倍改变
// FISH_ActChangeRate = 50332669, // 请求炮倍改变结果
void WebRobot::changeTurretRate(int index, int rate)
{
    LxGame::WrapPacket packet;
    LxGame::InputChangeRate msg;
    msg.set_rateindex(index);
    msg.set_turretrate(rate);
    LxGameHelper::MakeInputChangeRate(packet, msg);
    SendMessage(packet);
}

void WebRobot::handleChangeTurretRate(const LxGame::WrapPacket& packet)
{
    LxGame::ActChangeRate resp;
    do{
        if (!resp.ParseFromString(packet.data()))
        {
            break;
        }

        if (!packet.errorcode())
        {
            //设置状态可以打鱼
            break;
        }
    }while(0);
}

void WebRobot::randomTurretRate(int* rate_index, int* rate)
{
    int index = -1;
    const tagJsonGamePlay* room_data = s_people_number_2_room_Config[4][room_id_];
    const std::vector<int64>& turret_rate_list = room_data->_TurretRate;

    if(!room_data)
    {
        LOGERROR("config ERROR");
        assert(false);
        return;
    }

    //炮倍都随机
    index = GlobalUtils::GetRandNumber(0, turret_rate_list.size()-1);
    if(index >= 0)
    {
        *rate_index = index;
        *rate = turret_rate_list[index];
    }
}


void WebRobot::gainTurret(int32 turret_index, int32 turret_level, int32 turret_star)
{
    LxGame::WrapPacket packet;
    LxGame::ShowMeTheMoney GmPacket;
    GmPacket.set_turret_index(turret_index);
    GmPacket.set_turret_level(turret_level);
    GmPacket.set_turret_star(turret_star);
    LxGameHelper::MakeShowMeTheMoney(packet, GmPacket);
    SendMessage(packet);
}

void WebRobot::readyTurret()
{
    for(std::map<int32, LxGame::TurretInfo>::iterator iter = turret_list_.begin(); iter != turret_list_.end(); ++iter)
    {
        const LxGame::TurretInfo& turret_t = iter->second;
        if(turret_t.turret_level() == _equips_task._turret_level && turret_t.turret_plus() == _equips_task._turret_strengthen && 6 == turret_t.turret_index())
        {
            return;
        }
    }
    int32 turret_level = _equips_task._turret_level;
    int32 turret_star = turret_level/10 == 0 ? 1 : turret_level/10;
    gainTurret(6, turret_level, turret_star);
}

void WebRobot::getItem(int32 item_id, int32 item_num)
{
    LxGame::WrapPacket packet;
    LxGame::ShowMeTheMoney GmPacket;
    GmPacket.set_item_id(item_id);
    GmPacket.set_item_num(item_num);
    LxGameHelper::MakeShowMeTheMoney(packet, GmPacket);
    SendMessage(packet);
}

void WebRobot::readyData()
{
    // 把解决进入房间的逻辑放到这里

    uint64 current_gold = user_info_.user_gold();
    const tagJsonGamePlay* data_room_ptr = s_people_number_2_room_Config[4][room_id_];
    if(!data_room_ptr)
    {
        LOGERROR("tagJsonGamePlay pointer isnull");
        assert(false);
    }

    int different_value = 0;
    if(data_room_ptr->_MaxGoldPlay == 0)
    {
        different_value = data_room_ptr->_MinGoldPlay + 10000000 - current_gold;
    }
    else
    {
        different_value = (data_room_ptr->_MaxGoldPlay - data_room_ptr->_MinGoldPlay)/2 + data_room_ptr->_MinGoldPlay - current_gold;
    }

    LxGame::WrapPacket packet;
    LxGame::ShowMeTheMoney GmPacket;
    GmPacket.set_gold(different_value);
    LxGameHelper::MakeShowMeTheMoney(packet, GmPacket);
    SendMessage(packet);

    getItem(11201005, 1000);
    getItem(11301006, 1000);
    getItem(11301001, 2000);
    getItem(11601001, 2000);
    getItem(11501011, 2000);
    //获得炮台
    readyTurret();
}


// 合成翅膀
void WebRobot::task_one()
{
    //使用所有翅膀碎片，炮台的碎片升阶时需要
    for(auto item_pair : item_list_)
    {
        const LxGame::ItemPair& item = item_pair.second;
        int32 item_id = item.item_id();
        int32 consume_index = JDATA->ItemPtr()->MiscByID(item_id);

        if(JDATA->ItemPtr()->MainTypeByID(item_id) == 202)
        {
            for(int i = 0; i < 8; i++)
            {
                useProp(item_id, JDATA->SwingDataPtr()->CombinationNumByID(consume_index));
            }
        }
    }
}


    //只做安装的任务，不做养成
void WebRobot::task_two()
{
    int64 turret_equipped = user_info_.user_turret_id();

    // if(_use_base_turret)
    // {
    //     //更换为基础炮台
    //     for(std::map<int32, LxGame::TurretInfo>::iterator it = _turret_list.begin(); it != _turret_list.end(); ++it)
    //     {
    //         const LxGame::TurretInfo& turret = it->second;
    //         if(turret.turret_index() == 1)
    //         {
    //             switchTurret(turret.turret_id());
    //             break;
    //         }
    //     }
    //     return;
    // }

    //安装赵云炮台
    if(!turret_equipped)
    {
        LOGERROR("ERROR,NO TURRET EQUIPPED");
        assert(false);
        return;
    }

    std::map<int32, LxGame::TurretInfo>::iterator it = turret_list_.find(turret_equipped);
    if(it == turret_list_.end())
    {
        LOGERROR("no such turret user can equipped");
        assert(false);
        return;
    }

    const LxGame::TurretInfo& turret_info = it->second;
    if(6 != turret_info.turret_index()
        || turret_info.turret_level() != _equips_task._turret_level
        || turret_info.turret_plus() != _equips_task._turret_strengthen)
    {
        for(std::map<int32, LxGame::TurretInfo>::iterator iter = turret_list_.begin(); iter != turret_list_.end(); ++iter)
        {
            const LxGame::TurretInfo& turret_info_t  = iter->second;
            // 安装安装最合适的赵云炮台
            if(6 == turret_info_t.turret_index()
                && turret_info_t.turret_level() == _equips_task._turret_level
                && turret_info_t.turret_plus() == _equips_task._turret_strengthen)
            {
                switchTurret(turret_info_t.turret_id());
                break;
            }
            else if(6 == turret_info_t.turret_index()
                && turret_info_t.turret_level() == _equips_task._turret_level
                && turret_info_t.turret_plus() < _equips_task._turret_strengthen)
            {
                //这个判断做容错，
                if(_equips_task._turret_strengthen != 0 && turret_info_t.turret_star() != 6)
                {
                    continue;
                }
                switchTurret(turret_info_t.turret_id());
                doTurretStrength(turret_info_t.turret_id());
                break;
            }
            else
            {
                // LOGERROR("ERROR , unexpected");
                // assert(false);
            }

        }

        return;
    }
    // 炮台属性符合要求时走下面逻辑
    int64 swing_id = turret_info.turret_swing_id();

    // 未安装翅膀
    if(!swing_id)
    {
        for(std::map<int32, LxGame::SwingInfo>::iterator it = swings_list_.begin(); it != swings_list_.end(); ++it)
        {
            const LxGame::SwingInfo& swing = it->second;
            if(swing.swing_phase() == _equips_task._swings_level
                && swing.swing_index() == 6)
            {
                equipSwing(turret_equipped, swing.swing_id());
                return;
            }
            else if(swing.swing_phase() == 0
                && swing.swing_index() == 6)
            {
                equipSwing(turret_equipped, swing.swing_id());
                return;
            }
        }
    }

    // 已安装翅膀，但不符合要求
    std::map<int32, LxGame::SwingInfo>::iterator iter= swings_list_.find(swing_id);

    if(iter == swings_list_.end())
    {
        LOGERROR("no such swing");
        assert(false);
        return;
    }

    LxGame::SwingInfo& swing_info = iter->second;
    // 检测安装翅膀是否符合要求
    if(swing_info.swing_phase() != _equips_task._swings_level
        || swing_info.swing_index() != 6)
    {
        //找符合要求的翅膀装上
        for(std::map<int32, LxGame::SwingInfo>::iterator it = swings_list_.begin(); it != swings_list_.end(); ++it)
        {
            const LxGame::SwingInfo& swing = it->second;
            if(swing.swing_phase() == _equips_task._swings_level
                && swing.swing_index() == 6)
            {
                equipSwing(turret_equipped, swing.swing_id());
                break;
            }
            else if(swing.swing_phase() == 0
                && swing.swing_index() == 6)
            {
                equipSwing(turret_equipped, swing.swing_id());
                break;
            }
        }
    }
}

void WebRobot::task_three()
{
    int64 swing_id = 0;
    int64 turret_equipped_id = user_info_.user_turret_id();

    std::map<int32, LxGame::TurretInfo>::iterator iter = turret_list_.find(turret_equipped_id);
    if(iter == turret_list_.end())
    {
        LOGERROR("turret equipped is wrong");
        assert(false);
        return;
    }
    const LxGame::TurretInfo& turret_info = iter->second;

    if(turret_info.turret_plus() < _equips_task._turret_strengthen && turret_info.turret_level() == 60 && 
    turret_info.turret_star() == 6)
    {
        doTurretStrength(turret_equipped_id);
    }


    for(std::map<int32, LxGame::SwingInfo>::iterator it = swings_list_.begin(); it != swings_list_.end(); ++it)
    {
        const LxGame::SwingInfo& swing = it->second;
        if(swing.swing_turret_id() == turret_equipped_id)
        {
            swing_id = swing.swing_id();
            break;
        }
    }

    if(!swing_id)
    {
        LOGERROR("swings equipped wrong or userino updated isn't in time");
        assert(false);
        return;
    }

    if(_equips_task._swings_level)
    {
        doSwingLevelUp(swing_id);
    }

}

void WebRobot::doTurretStrength(int64 turret_id)
{
    std::map<int32, LxGame::TurretInfo>::iterator iter = turret_list_.find(turret_id);
    if(iter == turret_list_.end())
    {
        LOGERROR("turret equipped is wrong");
        assert(false);
        return;
    }
    const LxGame::TurretInfo& turret_info = iter->second;
    if(turret_info.turret_plus() >= 10 || turret_info.turret_level() < 60 
        || turret_info.turret_star() < 6)
    {
        LOGERROR("cant strengthen");
        return;
    }

    if(turret_info.turret_plus() < _equips_task._turret_strengthen)
    {
        for(int i = 0; i < _equips_task._turret_strengthen - turret_info.turret_plus(); ++i)
        {
            strengthenTurret(turret_id);
        }
    }
}

}//end namespace robot