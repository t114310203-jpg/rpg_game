#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>

using namespace std;

// 定義角色類別 Character
class Character {
protected:
    static const int EXP_LV;  // 定義每個等級所需的經驗值常數
    string name;              // 角色名稱
    int hp;                  // 角色當前生命值
    int maxHP;               // 角色最大生命值
    int level;               // 角色等級
    int exp;                 // 角色經驗值
    int power;               // 角色力量屬性
    int knowledge;           // 角色知識屬性
    int luck;                // 角色幸運屬性
    vector<string> skills;   // 角色技能列表

    // 等級提升方法，增加各項屬性
    void levelUp(int hInc, int pInc, int kInc, int lInc);

public:
    // 建構子
    Character(string n, int lv, int h, int po, int kn, int lu);
    
    // 顯示角色資訊
    virtual void print();
    
    // 擊敗怪物獲得經驗值 (純虛擬函數，需由子類別實作)
    virtual void beatMonster(int exp) = 0;
    
    // 設定角色生命值 (純虛擬函數)
    virtual void setHP(int) = 0;
    
    // 取得角色生命值 (純虛擬函數)
    virtual int getHP() = 0;
    
    // 取得角色攻擊力 (純虛擬函數)
    virtual int getAttack() = 0;
    
    // 使用技能攻擊，並返回傷害值
    virtual int useSkill(vector<Character*>& team);

    // 取得角色名稱
    string getName();

    // 選擇技能
    void chooseSkills(vector<string> skillOptions);
};

// 定義經驗值常數
const int Character::EXP_LV = 100;

// 建構子實作
Character::Character(string n, int lv, int h, int po, int kn, int lu) {
    name = n;
    level = lv;
    exp = pow(lv - 1, 2) * EXP_LV; // 經驗值計算公式
    hp = h;
    maxHP = h;
    power = po;
    knowledge = kn;
    luck = lu;
}

// 取得角色名稱
string Character::getName() {
    return name;
}

// 使用技能攻擊
int Character::useSkill(vector<Character*>& team) {
    if (skills.empty()) {
        cout << "沒有可用的技能！\n";
        return 0;
    }

    int skillChoice;
    int damage = 0;
    int maxHpIncrease = 0;
    string chosenSkill;

    // 列出所有可用技能
    cout << "可用技能：\n";
    for (size_t i = 0; i < skills.size(); ++i) {
        cout << i + 1 << ". " << skills[i] << "\n";
    }

    // 讓玩家選擇技能
    while (true) {
        cout << "選擇技能 (輸入對應數字): ";
        cin >> skillChoice;
        if (skillChoice >= 1 && skillChoice <= (int)skills.size()) {
            chosenSkill = skills[skillChoice - 1];
            break;
        }
        cout << "輸入無效，請重新選擇技能。\n";
    }

    // 根據選擇的技能計算傷害
    cout << name << " 使用了技能：" << chosenSkill << "！\n";

    if (chosenSkill == "狂怒") {
        damage = power * 2;
    } else if (chosenSkill == "重擊") {
        damage = power + 30;
    } else if (chosenSkill == "盾擊") {
        damage = power / 2 + 15;
        cout << name << " 使用盾擊，降低敵方攻擊力 5 點！\n";
    } else if (chosenSkill == "火球術") {
        damage = knowledge * 2;
    } else if (chosenSkill == "冰霜箭") {
        damage = knowledge * 1.5 + 10;
    } else if (chosenSkill == "閃電鏈") {
        damage = knowledge * 1.8;
    } else if (chosenSkill == "背刺") {
        damage = power * 2.5;
    } else if (chosenSkill == "毒刃") {
        damage = power + 20;
    } else if (chosenSkill == "爪擊") {
        damage = power + 10;
    } else {
        cout << "無效的技能選擇。\n";
        return 0;
    }

    // 顯示造成的傷害
    cout << name << " 造成 " << damage << " 點傷害！\n";
    return damage;
}

void Character::levelUp(int hInc, int pInc, int kInc, int lInc) {
    // 等級提升，經驗值計算並扣除
    level++;
    exp -= pow(level - 1, 2) * EXP_LV;

    // 屬性增加
    hp += hInc;
    maxHP += hInc;
    power += pInc;
    knowledge += kInc;
    luck += lInc;

    // 顯示等級提升訊息
    cout << name << " 升級了！生命值+" << hInc << " 力量+" << pInc << " 智力+" << kInc << " 幸運+" << lInc << "\n";
}

void Character::print() {
    // 顯示角色的基本資訊
    cout << name << ": 等級 " << level << " (經驗值: " << exp << "/" << pow(level, 2) * EXP_LV
         << "), 生命值: " << hp << "/" << maxHP << " 攻擊力: " << power
         << " 智力: " << knowledge << " 幸運: " << luck << "\n";
}

class Warrior : public Character {
private:
    // 靜態常數用於屬性成長值
    static const int HP_LV;
    static const int PO_LV;
    static const int KN_LV;
    static const int LU_LV;

public:
    // 戰士建構子，初始化基礎屬性與技能
    Warrior(string n, int lv = 1) : Character(n, lv, lv * HP_LV, lv * PO_LV, lv * KN_LV, lv * LU_LV) {
        skills.push_back("重擊");
        skills.push_back("盾擊");
        skills.push_back("狂怒");
    }

    // 顯示角色資訊 (覆寫)
    void print() { cout << "戰士 "; Character::print(); }
    
    // 取得與設定生命值
    int getHP() { return hp; }
    void setHP(int hp) { this->hp = hp; }

    // 取得攻擊力
    int getAttack() { return power; }

    // 擊敗怪物並獲得經驗值
    void beatMonster(int exp) {
        this->exp += exp;
        while (this->exp >= pow(this->level, 2) * EXP_LV)
            this->levelUp(HP_LV, PO_LV, KN_LV, LU_LV);
    }

    // 使用技能
    int useSkill(vector<Character*>& team);
};

int Warrior::useSkill(vector<Character*>& team) {
    return Character::useSkill(team);
}

// 初始化戰士屬性成長值
const int Warrior::HP_LV = 100;
const int Warrior::PO_LV = 10;
const int Warrior::KN_LV = 5;
const int Warrior::LU_LV = 5;

class Wizard : public Character {
private:
    static const int HP_LV;
    static const int PO_LV;
    static const int KN_LV;
    static const int LU_LV;

public:
    // 法師建構子，初始化基礎屬性與技能
    Wizard(string n, int lv = 1) : Character(n, lv, lv * HP_LV, lv * PO_LV, lv * KN_LV, lv * LU_LV) {
        skills.push_back("火球術");
        skills.push_back("冰霜箭");
        skills.push_back("閃電鏈");
    }

    // 顯示角色資訊 (覆寫)
    void print() { cout << "法師 "; Character::print(); }
    
    // 取得與設定生命值
    int getHP() { return hp; }
    void setHP(int hp) { this->hp = hp; }

    // 取得攻擊力
    int getAttack() { return knowledge; }

    // 擊敗怪物並獲得經驗值
    void beatMonster(int exp) {
        this->exp += exp;
        while (this->exp >= pow(this->level, 2) * EXP_LV)
            this->levelUp(HP_LV, PO_LV, KN_LV, LU_LV);
    }

    int useSkill(vector<Character*>& team);
};

int Wizard::useSkill(vector<Character*>& team) {
    return Character::useSkill(team);
}

// 初始化法師屬性成長值
const int Wizard::HP_LV = 80;
const int Wizard::PO_LV = 4;
const int Wizard::KN_LV = 15;
const int Wizard::LU_LV = 7;

class Rogue : public Character {
private:
    static const int HP_LV;
    static const int PO_LV;
    static const int KN_LV;
    static const int LU_LV;

public:
    // 盜賊建構子，初始化基礎屬性與技能
    Rogue(string n, int lv = 1) : Character(n, lv, lv * HP_LV, lv * PO_LV, lv * KN_LV, lv * LU_LV) {
        skills.push_back("背刺");
        skills.push_back("毒刃");
        skills.push_back("爪擊");
    }

    // 顯示角色資訊 (覆寫)
    void print() { cout << "盜賊 "; Character::print(); }
    
    // 取得與設定生命值
    int getHP() { return hp; }
    void setHP(int hp) { this->hp = hp; }

    // 取得攻擊力
    int getAttack() { return luck; }

    // 擊敗怪物並獲得經驗值
    void beatMonster(int exp) {
        this->exp += exp;
        while (this->exp >= pow(this->level, 2) * EXP_LV)
            this->levelUp(HP_LV, PO_LV, KN_LV, LU_LV);
    }

    int useSkill(vector<Character*>& team);
};

int Rogue::useSkill(vector<Character*>& team) {
    return Character::useSkill(team);
}

// 初始化盜賊屬性成長值
const int Rogue::HP_LV = 90;
const int Rogue::PO_LV = 7;
const int Rogue::KN_LV = 7;
const int Rogue::LU_LV = 15;

// 定義怪物類型的列舉型別，用於標示怪物的分類
enum MonsterType { NORMAL, ELITE, BOSS };

// 定義怪物類別
class Monster {
public:
    string name;      // 怪物名稱
    int hp;          // 生命值
    int attack;      // 攻擊力
    MonsterType type; // 怪物類型

    // 建構子，用於初始化怪物
    Monster(string n, int h, int a, MonsterType t) : name(n), hp(h), attack(a), type(t) {}

    // 顯示怪物資訊
    void print() {
        cout << name << " 生命值: " << hp << " 攻擊力: " << attack << " 類型: ";
        switch (type) {
            case NORMAL: cout << "普通怪物\n"; break;
            case ELITE: cout << "菁英怪\n"; break;
            case BOSS: cout << "BOSS\n"; break;
        }
    }

    // 取得怪物生命值
    int getHP() { return hp; }
    // 設定怪物生命值
    void setHP(int newHP) { hp = newHP; }
    // 取得怪物攻擊力
    int getAttack() { return attack; }
};

// 生成怪物的函數，根據角色隊伍的攻擊力調整怪物強度
Monster generateMonster(const vector<Character*>& team) {
    int totalLevel = 0;
    for (size_t i = 0; i < team.size(); ++i) {
        totalLevel += team[i]->getAttack();
    }

    int baseHP = 120;
    int baseAttack = 12;

    // 計算隊伍的平均攻擊力，用於決定怪物的強度
    int averageLevel = totalLevel / team.size();
    int hp = baseHP + (averageLevel * 6);
    int attack = baseAttack + (averageLevel * 2);

    // 可隨機產生的怪物名稱
    vector<string> monsterNames;
    monsterNames.push_back("哥布林");
    monsterNames.push_back("狼人");
    monsterNames.push_back("食人魔");

    int index = rand() % monsterNames.size();
    MonsterType type = NORMAL;

    // 根據隊伍平均等級，隨機決定怪物等級
    if (averageLevel >= 10 && rand() % 3 == 0) {
        type = ELITE;
        hp += 80;
        attack += 5;
    } else if (averageLevel >= 20 && rand() % 5 == 0) {
        type = BOSS;
        hp += 150;
        attack += 10;
    }

    // 顯示產生的怪物資訊
    cout << "產生的怪物: " << monsterNames[index]
         << " 血量: " << hp
         << ", 攻擊力: " << attack
         << ", 類型: " << (type == BOSS ? "BOSS" : (type == ELITE ? "菁英怪" : "普通怪物")) << "\n";

    return Monster(monsterNames[index], hp, attack, type);
}

// 戰鬥系統，處理角色與怪物的戰鬥流程
void battle(vector<Character*>& team, Monster* monster) {
    cout << "戰鬥開始！\n";

    while (true) {
        // 角色輪流攻擊
        for (size_t i = 0; i < team.size(); ++i) {
            if (team[i]->getHP() > 0) {
                int choice;
                cout << team[i]->getName() << " 回合：\n1. 普通攻擊\n2. 使用技能\n選擇: ";
                cin >> choice;

                int damage = (choice == 2) ? team[i]->useSkill(team) : team[i]->getAttack();
                monster->setHP(monster->getHP() - damage);

                cout << team[i]->getName() << " 對 " << monster->name << " 造成 " << damage << " 點傷害！\n";

                if (monster->getHP() <= 0) {
                    cout << monster->name << " 被擊敗了！戰鬥勝利！\n";
                    int expDrop = (monster->type == BOSS) ? 200 : 50;
                    for (size_t j = 0; j < team.size(); ++j) {
                        if (team[j]->getHP() > 0) {
                            team[j]->beatMonster(expDrop);
                        }
                    }
                    return;
                }
            }
        }

        // 檢查存活角色數
        int aliveCount = 0;
        for (size_t i = 0; i < team.size(); ++i) {
            if (team[i]->getHP() > 0) {
                aliveCount++;
            }
        }

        // 怪物攻擊所有角色
        if (aliveCount > 0) {
            int damagePerCharacter = monster->getAttack() / aliveCount;
            cout << monster->name << " 開始攻擊所有存活的角色，每人受到 " << damagePerCharacter << " 點傷害！\n";

            if (monster->type == BOSS) {
                int bossSpecialDamage = monster->getAttack() * 1.5;
                for (size_t i = 0; i < team.size(); ++i) {
                    if (team[i]->getHP() > 0) {
                        team[i]->setHP(team[i]->getHP() - bossSpecialDamage);
                        cout << monster->name << " 使用特殊攻擊對 " << team[i]->getName() << " 造成 " << bossSpecialDamage << " 點傷害！\n";
                    }
                }
            }

            for (size_t i = 0; i < team.size(); ++i) {
                if (team[i]->getHP() > 0) {
                    team[i]->setHP(team[i]->getHP() - damagePerCharacter);
                    if (team[i]->getHP() <= 0) {
                        team[i]->setHP(0);
                        cout << team[i]->getName() << " 已陣亡！\n";
                    } else {
                        cout << team[i]->getName() << " 受到 " << damagePerCharacter << " 點傷害，剩餘生命值: " << team[i]->getHP() << "\n";
                    }
                }
            }
        }
    }
}




int main() {
    srand(time(0));  // 初始化隨機數生成器，以時間作為種子

    // 定義各職業技能列表
    vector<string> warriorSkills;
    warriorSkills.push_back("重擊");
    warriorSkills.push_back("盾守");
    warriorSkills.push_back("狂怒");

    vector<string> wizardSkills;
    wizardSkills.push_back("火球術");
    wizardSkills.push_back("冰霜箭");
    wizardSkills.push_back("閃電鏈");

    vector<string> rogueSkills;
    rogueSkills.push_back("背刺");
    rogueSkills.push_back("毒刃");
    rogueSkills.push_back("潛行");

    vector<Character*> team;  // 儲存角色的隊伍容器

    // 創建三個角色
    for (int i = 0; i < 3; ++i) {
        string name;
        int choice;

        cout << "請輸入角色" << (i + 1) << "的名字: ";
        cin >> name;

        // 讓使用者選擇角色職業
        cout << "選擇職業:\n1. 戰士\n2. 法師\n3. 盜賊\n選擇: ";
        cin >> choice;

        // 根據選擇建立對應的角色物件
        switch (choice) {
            case 1:
                team.push_back(new Warrior(name));
                break;
            case 2:
                team.push_back(new Wizard(name));
                break;
            case 3:
                team.push_back(new Rogue(name));
                break;
            default:
                cout << "無效的選擇！預設為戰士。\n";
                team.push_back(new Warrior(name));
        }
    }

    cout << "角色已創建！" << endl;

    bool playing = true;  // 控制遊戲主循環的布林值
    while (playing) {
        // 生成怪物並顯示其資訊
        Monster monster = generateMonster(team);
        monster.print();

        // 進行戰鬥
        battle(team, &monster);

        // 擊敗怪物後，給存活角色經驗值
        int expDrop = 50;
        for (size_t i = 0; i < team.size(); ++i) {
            if (team[i]->getHP() > 0) {
                team[i]->beatMonster(expDrop);
            }
        }

        // 詢問使用者是否繼續遊戲
        cout << "是否繼續遊戲？ (1. 繼續 / 0. 結束): ";
        int continueChoice;
        cin >> continueChoice;
        if (continueChoice == 0) {
            playing = false;  // 結束遊戲
        }
    }

    // 釋放動態配置的角色記憶體
    for (size_t i = 0; i < team.size(); ++i) {
        delete team[i];
    }

    return 0;  // 程式正常結束
}
