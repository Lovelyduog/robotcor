#pragma once
#include <string>
#include <map>
#include <set>
#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "RobotBase.hpp"
#include "libcommon/protocol/LxGame.pb.h"
#include "libcommon/protocol/pbjson.h"
#include "libjsons/JsonConfig.h"
namespace conn{
    class WebConnect;
}
class MessagerParser;
using namespace std;
using namespace google::protobuf;

namespace robot{
enum STATUS{
    STATUS_UNLOGINED,
    STATUS_LOGINED,
    STATUS_READY_DATA,
    STATUS_READY_DATA_DONE,
    STATUS_EQUIP_DONE,
    STATUS_EQUIP_GROWTH,
    STATUS_PLAY,
    STATUS_PLAY_NOT
};

typedef std::map<int, const tagJsonGamePlay *> DATA_ROOMS;

class Equips
{
public:
    Equips(){_turret_level = 0; _turret_strengthen = 0; _swings_level = 0;}
    Equips(int turret_level, int turret_strengthen, int swings_level):
    _turret_level(turret_level),_turret_strengthen(turret_strengthen),
    _swings_level(swings_level){};

    Equips(const Equips& other)
    {
    _turret_level = other._turret_level;
    _turret_level = other._turret_strengthen;
    _turret_level = other._swings_level;
    }

    Equips& operator=(const Equips& src)
    {
    if(this == &src)
    {
        return *this;
    }

    _turret_level = src._turret_level;
    _turret_strengthen = src._turret_strengthen;
    _swings_level = src._swings_level;
    return *this;
    }
    bool isInitted(){return 0 != (_turret_level & _turret_strengthen & _swings_level);}

public:
    int _turret_level;
    int _turret_strengthen;
    int _swings_level;
};
class WebRobot : public RobotBase
{
public:
    WebRobot(boost::asio::io_context& ioc,const std::string& host, const std::string& port, const std::string& target);
    ~WebRobot();
    void Init();
    void Run();
    void StartHeartBeat();
    void SendMessage(const LxGame::WrapPacket& packet);
    void UpdateRoomFlag(bool status);
    void SetAccount(const std::string& account){account_ = account;};
    void SetToken(const std::string& token){ token_ = token;};
    void UpdateStatus(STATUS status);
    void SetRoom(int32 room){room_id_ = room;};
public:
    //universal
    void Login();
    void handleLoginRes(const LxGame::WrapPacket& packet);
    void handleEnterRoomRes(const LxGame::WrapPacket& packet);
    void EnterRoom(int32 room_index);
    void handleCreatedFish(const LxGame::WrapPacket& packet);
    void handlecUserInfoChange(const LxGame::WrapPacket& packet);
    void handleHittedFishRes(const LxGame::WrapPacket& packet);
    void recharge(const std::string& product_name);
    void doRecharge();
    void handleRechargeRes(const LxGame::WrapPacket& packet);
    void getPropList();
    void handlePropListlRes(const LxGame::WrapPacket& packet);
    void useProp(int32 prop_id, int64 num);
    void handleUsePropRes(const LxGame::WrapPacket& packet);
    //dont complete for the time being
    // void castSkill(int32 skill_id);
    // void castSkillRes(const LxGame::WrapPacket& packet);
public:
    // growth function
    void actTurretExp();
    void getTurretExpUp(int64 turret_id, std::vector<LxGame::ItemPair>& consumables);
    virtual void handleTurretExpUpRes(const LxGame::WrapPacket& packet);
    void getTurretStarUp(int64 turret_id, std::vector<LxGame::ItemPair>& consumables);
    void handleTurretStarUpRes(const LxGame::WrapPacket& packet);
    void strengthenTurret(int64 turret_id);
    void handleStrengthenTurretRes(const LxGame::WrapPacket& packet);
    void actSwitchTurret();
    void switchTurret(int64 turret_id);
    void handleSwitchTurretRes(const LxGame::WrapPacket& packet);
    void equipSwing(int64 turret_id, int64 swings_id);
    void handleEquipSwing(const LxGame::WrapPacket& packet);
    void getSwingLevelUp(int64 swing_id, int64 item_id, const std::vector<int64>& swings_ids_consumed);
    void handleGetSwingLevelUpRes(const LxGame::WrapPacket& packet);
    void doSwingLevelUp(int64 swing_id);
    void changeTurretRate(int index, int rate);
    void handleChangeTurretRate(const LxGame::WrapPacket& packet);
    void doTurretStrength(int64 turret_id);
public:
    void gainTurret(int32 turret_index, int32 turret_level, int32 turret_star);
    void getItem(int32 item_id, int32 item_num);
    void randomTurretRate(int* rate_index, int* rate);
    void readyTurret();
    void readyData();
    void LoadDataInLocal();
    void task_one();
    void task_two();
    void task_three();
private:
    MessagerParser* parser_;
    conn::WebConnect* connecter_;
    boost::asio::io_context& ioc_;
    boost::asio::deadline_timer heart_beat_timer_;
    boost::asio::deadline_timer shot_fish_timer_;
    LxGame::UserInfo user_info_;
    std::map<int32, LxGame::ItemPair> item_list_;
    std::map<int32, LxGame::TurretInfo> turret_list_;
    std::map<int32, LxGame::SwingInfo> swings_list_;
    std::map<int32, LxGame::QuestInfo> task_list_;
    std::map<int32, LxGame::FishRoomProto> room_list_;
    std::map<int32, LxGame::SurpriseInfo> surprises_;
    LxGame::FishRoomProto room_config_infos_;
    std::map<uint32, uint64> fishes_;
    std::string account_;
    std::string token_;
    bool b_inRoom_;
    std::size_t bullet_id_;
    STATUS status_;
    int32 room_id_;
    int32 client_id_;
    static std::map<int , DATA_ROOMS> s_people_number_2_room_Config;
    Equips _equips_task;
    static std::map<int, Equips> s_equips_Config;
    std::set<std::size_t> m_bullets;
};

}//end namespace robot