#include <iostream>  // 標準輸入輸出
#include <string>    // 字串處理
#include <vector>    // 動態陣列
#include <algorithm> // 演算法函式
#include <cmath>     // 數學函式
#include <random>    // 隨機數生成
#include <limits>    // 數值限制
#include <thread>    // 用於延遲顯示
#include <chrono>    // 時間相關函式
using namespace std;

// ==========================================
// 輔助函式區 (Helper Functions)
// ==========================================
// 設定控制台編碼為 UTF-8
#ifdef _WIN32
void setupConsole() {
    system("chcp 65001 > nul"); // 設定控制台為 UTF-8 編碼
}
#else
void setupConsole() {
    // Linux/macOS 通常預設就支援 UTF-8 和 ANSI 顏色，不需額外設定
}
#endif
//  ANSI顏色代碼
#if (true) // 如果支援 ANSI 顏色
namespace Color {
    const string RESET   = "\033[0m";    // 重置
    const string BOLD    = "\033[1m";    // 粗體
    const string GRAY    = "\033[90m";   // 灰色
    const string RED     = "\033[91m";   // 紅色
    const string GREEN   = "\033[92m";   // 綠色
    const string YELLOW  = "\033[93m";   // 黃色
    const string BLUE    = "\033[94m";   // 藍色
    const string MAGENTA = "\033[1;95m"; // 紫色
    const string CYAN    = "\033[96m";   // 青色
    const string WHITE   = "\033[97m";   // 白色
}
#else
namespace Color {
    const string RESET   = "";   // 重置
    const string BOLD    = "";   // 粗體
    const string GRAY    = "";   // 灰色
    const string RED     = "";   // 紅色
    const string GREEN   = "";   // 綠色
    const string YELLOW  = "";   // 黃色
    const string BLUE    = "";   // 藍色
    const string MAGENTA = "";   // 紫色
    const string CYAN    = "";   // 青色
    const string WHITE   = "";   // 白色
}
#endif

// 隨機數生成器
inline int getRandom(int min, int max) {
    static random_device rd;                      // 隨機數種子
    static mt19937 gen(rd());                     // 梅森旋轉演算法
    uniform_int_distribution<> distrib(min, max); // 均勻分佈
    return distrib(gen);                          // 生成隨機數
}

// 延遲函式
inline void wait(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms)); // 延遲指定毫秒數
}

// 顯示提示，並清除輸入緩衝區，按下 Enter 繼續
inline void clearInput(string prompt = "按下 Enter 繼續..." , string color = Color::GRAY) {
    if (prompt != "") cout << color << prompt << Color::RESET << endl; 
    cin.clear();                                         // 清除錯誤標誌        
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略緩衝區內容
}

// 獲取有效輸入
int getValidInput(int min, int max, string prompt = ">> 請選擇: ", string color = Color::RESET) { 
    int choice;
    while (true) {
        // 顯示提示
        cout << color << prompt << Color::RESET;
        // 輸入檢查
        if (cin >> choice) {
            if (choice >= min && choice <= max) {
                clearInput("");
                return choice;
            }
        }
        // 無效輸入處理
        cout << Color::RED << "無效輸入，請重新輸入！\n" << Color::RESET;
        clearInput("");
    }
}

// 延遲顯示訊息函式
void printMessage(const string& text, const string& name = "", int delayMs = 25, string color = "") {
    // 預設顏色設定
    string finalColor = color;
    if (finalColor == "") {
        finalColor = (name == "") ? Color::CYAN : Color::YELLOW;
    }
    cout << finalColor;
    // 顯示名稱
    if (name != "") {
        cout << "【" << name << "】";
    }
    // 逐字顯示訊息
    if (delayMs > 0){
        for (char c : text) {
            cout << c << flush;
            wait(delayMs);
        }
    }   else {
        cout << text;
    }
    // 換行並重置顏色
    cout << Color::RESET << endl;
    wait(500); 
}

// ==========================================
// 遊戲狀態與環境 (Game State)
// ==========================================

// 遊戲狀態結構體
struct GameState {
    // 章節進度、玩家金錢、玩家線索數、Boss擊敗狀態
    int chapter;
    int playerMoney;
    int playerClues;
    // Boss擊敗狀態
    bool boss_Kir;
    bool boss_Vermouth;
    bool boss_Vodka;
    bool boss_Gin;
};

// 前向宣告
GameState gState; // 全局遊戲狀態 

// 地點結構體
struct Location {
    // 編號、名稱、描述、敵人屬性修正、金錢掉落修正、調查加成、解鎖章節
    int id;
    string name;
    string description;
    double enemyStatMod;
    double moneyDropMod;
    int investigationBonus;
    int requiredChapter;
};

// 地點列表
const vector<Location> LOCATIONS = {
    // {id, name, description, enemyStatMod, moneyDropMod, investigationBonus, requiredChapter}
    {0, "警視廳", "位於東京的警視廳大樓，情報中心。", 0.8, 0.8, 30, 0},
    {1, "阿笠博士家", "位於米花町，準備出發的起點。", 1.0, 1.0, 20, 0},
    {2, "太平洋巡邏艦", "前往五稜星的船上，海風強勁。", 1.2, 1.2, 10, 2},
    {3, "五稜星大廳", "巨大的海上浮標建築，戒備森嚴。", 1.4, 1.5, 5, 3}, 
    {4, "中央控制室", "諾亞方舟的核心所在，也是敵人的首要目標。", 1.6, 1.8, 0, 4},
    {5, "地下維修通道", "通往深海伺服器的捷徑，充滿高熱。", 1.8, 2.0, -10, 5}, 
    {6, "水下伺服器室", "深海 600 公尺，海水冷卻系統的關鍵。", 2.2, 2.5, -5, 6},
    {7, "外部水域", "波濤洶湧的海面。", 3.0, 5.0, 10, 7}
};

// 前向宣告
Location currentLocation; // 當前地點

// ==========================================
// 類別定義 (Classes)
// ==========================================

// 前向宣告
class Character;
Character* createRandomNPC();

// 屬性類型列舉
enum StatType { ATK, INT, LUCK }; // 攻擊、智力、運氣

// 技能類別
class Skill {
protected: 
    //名稱、描述、最大冷卻、當前冷卻
    string name;
    string description;
    int maxCooldown;
    int currentCooldown;
public:
    // 建構子與解構子
    Skill(string n, string d, int cd = 0) : name(n), description(d), maxCooldown(cd), currentCooldown(0) {}
    virtual ~Skill() {}
    // 存取函式
    string getName() const { return name; }
    string getDesc() const { return description; }
    int getMaxCD() const { return maxCooldown; }
    int getCurrentCD() const { return currentCooldown; }
    bool isReady() const { return currentCooldown <= 0; }
    // 冷卻管理
    void startCooldown() { currentCooldown = maxCooldown; }
    void reduceCooldown() { if(currentCooldown > 0) currentCooldown--; }
    void resetCooldown() { currentCooldown = 0; }
    // 使用技能
    virtual int use(Character* user, vector<Character*>& team) = 0;
};

// 道具類別
class Item {
protected:
    // 道具名稱、描述、價格
    string name;
    string description;
    int price;
public:
    // 建構子與解構子
    Item(string n, int p, string d) : name(n), price(p), description(d) {}
    virtual ~Item() {}
    // 存取函式
    string getName() const { return name; }
    int getPrice() const { return price; }
    string getDesc() const { return description; }
    // 使用道具
    virtual bool apply(Character* target) = 0;
};

// 角色類別
class Character {
protected:
    // 基準經驗值、名稱、職業名稱、是否為玩家、生命值、最大生命值、等級、經驗值、力量、智力、運氣、技能列表、暫時增益、升級函式
    static const int EXP_LV =100;
    string name;
    string className; 
    bool isPlayer; 
    int hp, maxHP, level, exp, power, knowledge, luck;      
    vector<Skill*> skills; 
    int tempBuff = 0; 
    void levelUp(int hInc, int pInc, int kInc, int lInc);
public:
    // 建構子與解構子
    Character(string n, string cls, int lv, int h, int po, int kn, int lu, bool isPly = false);
    virtual ~Character();
    // 存取函式 
    virtual void print(); 
    virtual int getHP() const { return hp; }
    virtual int getMaxHP() const { return maxHP; }
    virtual int getAttack() const { return power + tempBuff; }
    virtual int getKnowledge() const { return knowledge; }
    virtual int getLuck() const { return luck; }
    virtual string getName() const { return name; }
    virtual int getSpeed() const { return luck; } 
    virtual bool getIsPlayer() const { return isPlayer; } 
    virtual int getTempBuff() const { return tempBuff; }
    virtual string getQuote(string action) { return ""; } 
    const vector<Skill*>& getSkills() const { return skills; } 
    // 狀態修改函式
    virtual void beatMonster(int exp) = 0;
    virtual void setHP(int val) { hp = val; if(hp > maxHP) hp = maxHP; if(hp < 0) hp = 0; }
    virtual void addBuff(int val) { tempBuff += val; }
    virtual void clearBuff() { tempBuff = 0; }
    // 冷卻管理
    void tickCooldowns() { for(auto s : skills) s->reduceCooldown(); }
    void resetCooldowns() { for(auto s : skills) s->resetCooldown(); }
    // 技能使用介面
    void addSkill(Skill* skill) { skills.push_back(skill); }
    virtual int performSkill(int skillIdx, vector<Character*>& team);
    virtual int useRandomSkill(vector<Character*>& team); 
    
};
// 建構子實作
Character::Character(string n, string cls, int lv, int h, int po, int kn, int lu, bool isPly) {
    name = n; className = cls; level = lv; exp = pow(lv - 1, 2) * EXP_LV;
    hp = h; maxHP = h; power = po; knowledge = kn; luck = lu; isPlayer = isPly;
}
// 解構子實作
Character::~Character() {
    for (Skill* s : skills) delete s;
    skills.clear();
}
// 等級提升實作
void Character::levelUp(int hInc, int pInc, int kInc, int lInc) {
    level++;
    exp -= pow(level - 1, 2) * EXP_LV;
    hp += hInc; maxHP += hInc; power += pInc; knowledge += kInc; luck += lInc;
    cout << Color::GREEN << Color::BOLD << ">>> " + name + " 升級了！ (Lv." + to_string(level) + ")\n" << Color::RESET;
    wait(500);
}
// 角色資訊顯示實作
void Character::print() {
    cout << Color::BOLD << name << Color::RESET << " [" << className << "] Lv." << level 
         << " HP:" << (hp > maxHP * 0.3 ? Color::GREEN : Color::RED) << hp << "/" << maxHP << Color::RESET
         << " 攻:" << getAttack() << " 智:" << knowledge << " 運:" << luck << "\n";
}
// 技能使用介面實作
int Character::performSkill(int skillIdx, vector<Character*>& team) {
    if (skillIdx < 0 || skillIdx >= skills.size()) return 0;
    Skill* s = skills[skillIdx];
    string quote = getQuote("SKILL");
    if (quote != "") printMessage(quote, name);
    printMessage(name + " 使用了技能：" + s->getName() + "！", "", 30, Color::MAGENTA);
    int result = s->use(this, team);
    s->startCooldown(); 
    if (result > 0 && getRandom(1, 100) <= luck) {
        cout << Color::RED << Color::BOLD << "CRITICAL HIT! 爆擊！\n" << Color::RESET;
        result = (int)(result * 1.5);
    }
    return result;
}
// 隨機技能使用介面實作
int Character::useRandomSkill(vector<Character*>& team) {
    if (skills.empty()) return 0;
    vector<int> readyIndices;
    for(size_t i=0; i<skills.size(); ++i) if(skills[i]->isReady()) readyIndices.push_back(i);
    if (readyIndices.empty()) return -1; 
    int idx = readyIndices[getRandom(0, readyIndices.size() - 1)];
    return performSkill(idx, team);
}

// ==========================================
// 技能與道具實作
// ==========================================

// 攻擊型技能類別
class AttackSkill : public Skill {
    StatType stat; double multiplier; int baseDmg;
public:
    AttackSkill(string n, string d, StatType s, double m, int b, int cd) : Skill(n, d, cd), stat(s), multiplier(m), baseDmg(b) {}
    int use(Character* user, vector<Character*>& team) override {
        int val = (stat == ATK) ? user->getAttack() : ((stat == INT) ? user->getKnowledge() : user->getLuck());
        return (int)(val * multiplier + baseDmg);
    }
};
// 治療型技能類別
class HealSkill : public Skill {
    int baseHeal; double intMod;
public:
    HealSkill(string n, string d, int base, double mod, int cd) : Skill(n, d, cd), baseHeal(base), intMod(mod) {}
    int use(Character* user, vector<Character*>& team) override {
        int amount = baseHeal + (int)(user->getKnowledge() * intMod);
        cout << Color::GREEN << ">>> 全體隊員恢復了 " << amount << " 點生命值！" << Color::RESET << "\n";
        for(auto* member : team) if (member->getHP() > 0) member->setHP(member->getHP() + amount);
        return 0; 
    }
};
// 恢復型道具類別
class RestoreItem : public Item {
    int amount;
public:
    RestoreItem(string n, int p, string d, int amt) : Item(n, p, d), amount(amt) {}
    bool apply(Character* target) override {
        if (target->getHP() <= 0) { cout << Color::RED << "無法對已陣亡角色使用！\n" << Color::RESET; return false; }
        target->setHP(target->getHP() + amount);
        cout << Color::GREEN << target->getName() << " 恢復了 " << amount << " 點生命！\n" << Color::RESET;
        return true;
    }
};
// 復活型道具類別
class ReviveItem : public Item {
public:
    ReviveItem(string n, int p, string d) : Item(n, p, d) {}
    bool apply(Character* target) override {
        if (target->getHP() > 0) { cout << "該角色仍然存活。\n"; return false; }
        target->setHP(target->getMaxHP() / 2);
        cout << Color::GREEN << target->getName() << " 復活了！\n" << Color::RESET;
        return true;
    }
};

// ==========================================
// 背包與商店
// ==========================================

// 背包欄位結構體
struct InventorySlot { Item* item; int count; };
// 背包與商店容器
vector<InventorySlot> inventory;
vector<Item*> shopItems;             
// 新增道具至背包
void addToInventory(Item* itemRef) {
    for (auto& slot : inventory) {
        if (slot.item->getName() == itemRef->getName()) { slot.count++; return; }
    }
    inventory.push_back({itemRef, 1});
}

// ==========================================
// 職業與角色 (Characters)
// ==========================================

// Gadgeteer (道具專家 - 柯南)
class Gadgeteer : public Character {
public:
    Gadgeteer(string n, int lv=1) : Character(n, "名偵探", lv, lv*60, lv*5, lv*12, lv*8, true) {
        addSkill(new AttackSkill("腳力增強鞋", "踢出強力的物品", ATK, 2.0, 10, 2));
        addSkill(new AttackSkill("麻醉手錶", "精準射擊", INT, 1.5, 20, 4));
        addSkill(new AttackSkill("領結變聲器", "擾亂敵人", INT, 1.2, 0, 3));
    }
    void beatMonster(int exp) override { this->exp += exp; while (this->exp >= pow(this->level, 2) * 100) levelUp(60, 5, 12, 8); }
    string getQuote(string action) override {
        if (action == "ATTACK") return "可惡... 看招！";
        if (action == "SKILL") return "這招如何？";
        if (action == "WIN") return "真相只有一個！";
        return "";
    }
};

// Fighter (格鬥家 - 小蘭、平次、京極真、赤井秀一、安室透、和葉)
class Fighter : public Character {
public:
    Fighter(string n, string type, int lv=1) : Character(n, "格鬥家", lv, lv*100, lv*10, lv*3, lv*5, false) {
        if (type == "Karate") { addSkill(new AttackSkill("空手道劈擊", "重擊", ATK, 1.8, 0, 1)); addSkill(new AttackSkill("迴旋踢", "連續踢擊", ATK, 2.2, 0, 2)); }
        else if (type == "Kendo") { addSkill(new AttackSkill("劍道突刺", "精準", ATK, 1.5, 10, 1)); addSkill(new AttackSkill("居合斬", "拔刀", ATK, 2.5, 0, 3)); }
        else if (type == "Super") { addSkill(new AttackSkill("正拳突刺", "極高傷", ATK, 3.0, 0, 3)); addSkill(new AttackSkill("迴旋踢", "踢擊", ATK, 2.2, 0, 2)); }
        else if (type == "Sniper") { addSkill(new AttackSkill("銀色子彈", "狙擊", ATK, 3.5, 0, 4)); addSkill(new AttackSkill("截拳道", "近身", ATK, 2.0, 0, 2)); } 
        else if (type == "SecretPolice") { addSkill(new AttackSkill("零之執行", "猛攻", ATK, 2.8, 0, 3)); addSkill(new AttackSkill("博擊", "連打", ATK, 1.5, 0, 1)); }
        else if (type == "Aikido") { addSkill(new AttackSkill("合氣道摔", "防守反擊", ATK, 2.0, 0, 2)); addSkill(new AttackSkill("護身符", "幸運一擊", LUCK, 1.5, 20, 3)); }
    }
    void beatMonster(int exp) override { this->exp += exp; while (this->exp >= pow(this->level, 2) * 100) levelUp(100, 10, 3, 5); }
    string getQuote(string action) override {
        if (name == "毛利蘭") {
            if (action == "ATTACK") return "哈啊——！";
            if (action == "SKILL") return "我不會輸的！";
            if (action == "WIN") return "大家沒事吧？";
        } else if (name == "服部平次") {
            if (action == "ATTACK") return "看劍！";
            if (action == "SKILL") return "工藤，要上了！";
            if (action == "WIN") return "這就是大阪偵探的實力！";
        } else if (name == "京極真") {
            if (action == "ATTACK") return "喝！";
            if (action == "SKILL") return "為了園子小姐！";
            if (action == "WIN") return "修練還不夠...";
        } else if (name == "赤井秀一") {
            if (action == "ATTACK") return "哼。";
            if (action == "SKILL") return "墮落吧...";
            if (action == "WIN") return "Target clear.";
        } else if (name == "安室透") {
            if (action == "ATTACK") return "不會讓你逃掉的。";
            if (action == "SKILL") return "我的戀人是這個國家！";
            if (action == "WIN") return "任務完成。";
        } else if (name == "遠山和葉") {
            if (action == "ATTACK") return "看招！";
            if (action == "SKILL") return "不准碰平次！";
            if (action == "WIN") return "平次，我也很強吧！";
        }
        return "";
    }
};

// Support (後勤 - 灰原、博士、園子、優作)
class Support : public Character {
public:
    Support(string n, string type, int lv=1) : Character(n, "後勤", lv, lv*50, lv*3, lv*15, lv*6, false) {
        if (type == "Science") { addSkill(new HealSkill("應急處置", "治療", 50, 3.0, 3)); addSkill(new AttackSkill("化學知識", "智力傷害", INT, 2.0, 0, 2)); }
        else if (type == "Inventor") { addSkill(new AttackSkill("冷謎語", "精神傷", INT, 1.0, 10, 1)); addSkill(new HealSkill("應急處置", "治療", 50, 3.0, 3)); }
        else if (type == "Rich") { addSkill(new AttackSkill("鈔能力", "金錢攻擊", LUCK, 3.0, 0, 2)); addSkill(new HealSkill("應急處置", "治療", 50, 3.0, 3)); luck+=20; }
        else if (type == "Novelist") { addSkill(new AttackSkill("世界級推理", "看穿一切", INT, 3.0, 0, 3)); addSkill(new HealSkill("冷靜分析", "恢復", 60, 2.0, 2)); }
    }
    void beatMonster(int exp) override { this->exp += exp; while (this->exp >= pow(this->level, 2) * 100) levelUp(50, 3, 15, 6); }
    string getQuote(string action) override {
        if (name == "灰原哀") {
            if (action == "ATTACK") return "讓開。";
            if (action == "SKILL") return "真是拿你們沒辦法...";
            if (action == "WIN") return "結束了呢。";
        } else if (name == "阿笠博士") {
            if (action == "ATTACK") return "我也來戰鬥！";
            if (action == "SKILL") return "試試我的新發明！";
            if (action == "WIN") return "發明大成功！";
        } else if (name == "鈴木園子") {
            if (action == "ATTACK") return "走開啦！";
            if (action == "SKILL") return "這可是鈴木財團的力量！";
            if (action == "WIN") return "阿真，我贏了！";
        } else if (name == "工藤優作") {
            if (action == "ATTACK") return "還不賴。";
            if (action == "SKILL") return "原來如此...";
            if (action == "WIN") return "一切都在預料之中。";
        }
        return "";
    }
};

// Trickster (特殊 - 基德、小五郎、有希子)
class Trickster : public Character {
public:
    Trickster(string n, string type, int lv=1) : Character(n, "特殊", lv, lv*80, lv*6, lv*8, lv*15, false) {
        if (type == "Thief") { addSkill(new AttackSkill("撲克牌槍", "運氣傷", LUCK, 2.0, 10, 2)); addSkill(new AttackSkill("化學炸彈", "爆炸", INT, 2.0, 0, 2)); }
        else if (type == "Sleep") { addSkill(new AttackSkill("過肩摔", "反擊", ATK, 1.5, 30, 2)); addSkill(new AttackSkill("沉睡推理", "爆發", INT, 2.5, 0, 4)); }
        else if (type == "Actress") { addSkill(new AttackSkill("易容術", "迷惑敵人", INT, 2.0, 10, 2)); addSkill(new AttackSkill("暗夜男爵夫人", "神秘攻擊", LUCK, 2.5, 0, 3)); }
    }
    void beatMonster(int exp) override { this->exp += exp; while (this->exp >= pow(this->level, 2) * 100) levelUp(80, 6, 8, 15); }
    string getQuote(string action) override {
        if (name == "怪盜基德") {
            if (action == "ATTACK") return "這不過是魔術罷了。";
            if (action == "SKILL") return "Ladies and Gentlemen!";
            if (action == "WIN") return "再會了，名偵探。";
        } else if (name == "毛利小五郎") {
            if (action == "ATTACK") return "看我的柔道！";
            if (action == "SKILL") return "呼... (沉睡)";
            if (action == "WIN") return "哈哈哈哈！真不愧是我！";
        } else if (name == "工藤有希子") {
            if (action == "ATTACK") return "哼哼，被騙到了吧？";
            if (action == "SKILL") return "好戲上場囉！";
            if (action == "WIN") return "這可是好萊塢級的演技！";
        }
        return "";
    }
};

// ==========================================
// 劇情系統 (Story)
// ==========================================

namespace Story {
    void triggerChapter0() {
        gState.chapter = 0;
        printMessage("\n=== 序章：警視廳的暗流 ===", "", 30, Color::YELLOW);
        printMessage("雨夜的東京，警視廳大樓燈火通明。", "", 30, Color::CYAN);
        wait(1000);
        printMessage("位於太平洋中心的設施「五稜星」遭受了未知的網路攻擊。", "黑田兵衛");
        printMessage("他們的目標很明確，是那個剛上線的核心系統——「諾亞方舟」。", "安室透");
        printMessage("如果那個組織得到了這個系統，後果不堪設想。", "赤井秀一");
        printMessage("我們必須在他們行動之前，死守住「五稜星」。", "黑田兵衛");
        printMessage(">> 新地點解鎖：阿笠博士家", "", 30, Color::GREEN);
        printMessage("試著搜查阿笠博士家", "神秘聲音", 0, Color::GRAY);
    }

    void triggerChapter1() {
        gState.chapter = 1;
        printMessage("\n=== 第一章：鈴木財團的邀請函 ===", "", 30, Color::YELLOW);
        printMessage("噹噹！這就是前往太平洋中心「五稜星」的特邀嘉賓證！", "鈴木園子");
        printMessage("如果那個傳聞是真的，黑衣組織絕對不會放過這個機會。", "柯南");
        printMessage("喂，工藤。你有感覺到吧？那種令人作嘔的氣息。", "灰原哀");
        printMessage("嗯，我們得小心行事。", "柯南");
        printMessage("或許繼續搜查會有新發現", "神秘聲音", 0, Color::GRAY);
    }

    void triggerChapter2() {
        gState.chapter = 2;
        printMessage("\n=== 第二章：駛向太平洋 ===", "", 30, Color::YELLOW);
        printMessage("載著柯南一行人的船隻，正劃破太平洋的波浪。", "", 30, Color::CYAN);
        wait(1000);
        printMessage("戒備森嚴啊。連美軍的潛艦都在附近海域待命。", "柯南");
        printMessage("畢竟這裡現在握有全世界的隱私鑰匙嘛，柯南君。", "安室透");
        printMessage("安室先生？你也來了...", "柯南");
        printMessage(">> 新地點解鎖：太平洋巡邏艦", "", 30, Color::GREEN);
        printMessage("也許太平洋巡邏艦上會有新發現", "神秘聲音", 0, Color::GRAY);
    }

    void triggerChapter3() {
        gState.chapter = 3;
        printMessage("\n=== 第三章：諾亞方舟的凝視 ===", "", 30, Color::YELLOW);
        printMessage("進入設施內部，巨大的全息投影螢幕占據了整個牆面。", "", 30, Color::CYAN);
        printMessage("偵測到訪客。開始進行身分驗證。", "諾亞方舟");
        printMessage("江戶川柯南，小學一年級。", "諾亞方舟");
        printMessage("(呼... 沒顯示工藤新一，看來有限制權限...)", "柯南");
        printMessage(">> 新地點解鎖：五稜星大廳", "", 30, Color::GREEN);
        printMessage("五稜星大廳閃過神秘的黑影", "神秘聲音", 0, Color::GRAY);
    }

    void triggerChapter4() {
        gState.chapter = 4;
        printMessage("\n=== 第四章：黑衣的入侵者 ===", "", 30, Color::YELLOW);
        printMessage("基爾，內部情況如何？", "琴酒");
        printMessage("我已經將「後門」程式植入了維修系統。", "基爾");
        printMessage("只要炸毀冷卻系統的閘門，我們就能趁機奪取控制權。", "伏特加");
        printMessage("動手。", "琴酒");
        printMessage(">> 新地點解鎖：中央控制室", "", 30, Color::GREEN);
        printMessage("中央控制室似乎有異常", "神秘聲音", 0, Color::GRAY);
    }

    void triggerChapter5() {
        gState.chapter = 5;
        printMessage("\n=== 第五章：伺服器過熱 ===", "", 30, Color::YELLOW);
        printMessage("C區冷卻水管破裂！伺服器溫度急劇升高！", "技術員");
        printMessage("灰原！妳那邊能看到系統狀況嗎？", "柯南");
        printMessage("不行，有人正在外部強行駭入！是組織的駭客！", "灰原哀");
        printMessage("他們想讓伺服器過熱重啟。絕對不能讓他們得逞！", "柯南");
        printMessage(">> 新地點解鎖：地下維修通道", "", 30, Color::GREEN);
        printMessage("地下維修通道好像有聲音", "神秘聲音", 0, Color::GRAY);
    }
    

    void triggerChapter6() {
        gState.chapter = 6;
        printMessage("\n=== 第六章：伺服器攻防戰 ===", "", 30, Color::YELLOW);
        printMessage("哎呀，小偵探，你來得太晚了。", "苦艾酒");
        printMessage("苦艾酒！妳果然混進來了。", "柯南");
        printMessage("想奪走諾亞方舟？別小看我開發的防禦邏輯！", "灰原哀");
        printMessage("伏特加，發射魚雷，直接把控制室炸了！", "琴酒");
        printMessage(">> 新地點解鎖：水下伺服器室", "", 30, Color::GREEN);
        printMessage("水下伺服器室好像有腳步聲", "神秘聲音", 0, Color::GRAY);
    }

    void triggerChapter7() {
        gState.chapter = 7;
        printMessage("\n=== 第七章：深海的逆轉 ===", "", 30, Color::YELLOW);
        printMessage("魚雷來了！把冷卻系統切換到「緊急排放」模式！", "柯南");
        printMessage("但是那樣會把海水全部灌進機房...", "阿笠博士");
        printMessage("沒關係！諾亞方舟會保護自己的！", "柯南");
        wait(1000);
        printMessage("偵測到惡意入侵者。啟動反擊協議。", "諾亞方舟");
        printMessage("什麼？！系統反過來控制了潛艇的導航？", "琴酒");
        printMessage("看來我們低估了這個AI的能力。", "伏特加");
    }

    void triggerChapter8() {
        gState.chapter = 8;
        printMessage("\n=== 終章：黎明前的五稜星 ===", "", 50, Color::YELLOW);
        printMessage("琴酒的潛艇在被國際刑警包圍前，強行切斷系統控制逃逸了。", "", 30, Color::CYAN);
        wait(1000);

        printMessage("又讓他們逃掉了啊。", "柯南");
        printMessage("但也讓他們吃了不少苦頭。聲紋特徵已經記錄下來了。", "赤井秀一");
        printMessage("這次算我們欠你一次，柯南君。", "安室透");
        printMessage("諾亞方舟似乎主動刪除了關於我們身分的檔案...", "灰原哀");
        printMessage("看來那個AI，比我們想像的還要在乎「人心」啊。", "柯南");
        
        wait(1000);
        printMessage("=============================", "", 100, Color::YELLOW);
        printMessage("    THANK YOU FOR PLAYING    ", "", 100, Color::YELLOW);
        printMessage("=============================", "", 100, Color::YELLOW);
        printMessage("雖然黑衣組織撤退了，但還有許多謎題等待解決...", "", 50, Color::GRAY);
        printMessage("輸入 0 結束遊戲...", "", 100, Color::GRAY);
    }
}

// ==========================================
// 戰鬥與怪物系統
// ==========================================

// 怪物類型列舉
enum MonsterType { NORMAL, ELITE, BOSS }; // 普通、精英、頭目

// 怪物類別
class Monster {
public:
    // 名稱、生命值、最大生命值、攻擊力、金錢掉落、類別
    string name;
    int hp, maxHp, attack, moneyDrop;
    MonsterType type;
    // 建構子
    Monster(string n, int h, int a, MonsterType t, int money) : name(n), hp(h), maxHp(h), attack(a), type(t), moneyDrop(money) {}
    // 存取函式
    void print() {
        string color = (type == BOSS) ? Color::RED : (type == ELITE ? Color::MAGENTA : Color::RESET);
        cout << Color::BOLD << "敵人遭遇: " << color << name << Color::RESET 
             << " (HP: " << hp << "/" << maxHp << ", ATK: " << attack << ")\n";
    }
    int getHP() { return hp; }
    // 狀態修改函式
    void setHP(int newHP) { hp = newHP; if(hp < 0) hp = 0;}
};

// 生成怪物函式
Monster generateMonster(const vector<Character*>& team) {
    // 計算隊伍平均戰力
    int totalPwr = 0;
    for (auto c : team) totalPwr += c->getAttack() + c->getHP()/10;
    int avgStr = totalPwr / team.size();
    double envMod = currentLocation.enemyStatMod;
    // 基礎屬性計算
    int baseHP = (100 + (avgStr * 4)) * envMod;
    int baseAtk = (15 + (avgStr / 3)) * envMod;
    int baseMoney = 50 * currentLocation.moneyDropMod; 
    // BOSS 生成邏輯
    // Ch3: 基爾 (Kir)
    if (getRandom(1, 100) <= 80 && currentLocation.id == 3 && !gState.boss_Kir) { 
        return Monster("基爾 (Kir)", baseHP*2.5, baseAtk*1.3, BOSS, baseMoney*3);
    }
    // Ch4: 苦艾酒 (Vermouth)
    if (getRandom(1, 100) <= 75 && currentLocation.id == 4 && !gState.boss_Vermouth) { 
        return Monster("苦艾酒 (Vermouth)", baseHP*3.0, baseAtk*1.5, BOSS, baseMoney*5);
    }
    // Ch5: 伏特加 (Vodka)
    if (getRandom(1, 100) <= 70 && currentLocation.id == 5 && !gState.boss_Vodka) { 
        return Monster("伏特加 (Vodka)", baseHP*3.5, baseAtk*1.6, BOSS, baseMoney*5);
    }
    // Ch6: 琴酒 (Gin) - 最終BOSS
    if (getRandom(1, 100) <= 70 && currentLocation.id == 6) { 
        return Monster("琴酒 (Gin)", baseHP*4.5, baseAtk*2.0, BOSS, baseMoney*10);
    }

    // 小怪與菁英生成
    int roll = getRandom(1, 100);
    if (roll > 80) { // 20% 機率遭遇菁英
        vector<string> elites = {"組織菁英狙擊手", "重裝蛙人隊長", "駭客首領"};
        string name = elites[getRandom(0, elites.size() - 1)];
        return Monster(name, baseHP*1.6, baseAtk*1.3, ELITE, baseMoney*2);
    }
    
    vector<string> normals = { "組織外圍成員", "被駭入的保全機器人", "武裝蛙人", "不明潛入者" };
    return Monster(normals[getRandom(0, normals.size()-1)], baseHP, baseAtk, NORMAL, baseMoney);
}

// 顯示戰鬥狀態函式
void printBattleStatus(const vector<Character*>& team, Monster* monster) {
    cout << Color::WHITE << "\n══════════════════════════════════════════════════" << Color::RESET << endl;
    string mColor = (monster->type == BOSS) ? Color::RED : Color::MAGENTA;
    cout << "【敵方】 " << mColor << Color::BOLD << monster->name << Color::RESET << "\n";
    cout << "  HP: " << monster->getHP() << "/" << monster->maxHp << " (ATK: " << monster->attack << ")\n";
    cout << "\n【我方】\n";
    for(auto* c : team) {
         if (c->getHP() <= 0) cout << "  " << Color::GRAY << c->getName() << " (無法戰鬥)" << Color::RESET << "\n";
         else cout << "  " << Color::BOLD << c->getName() << Color::RESET << " HP: " << c->getHP() << "/" << c->getMaxHP() << "\n";
    }
    cout << Color::WHITE << "══════════════════════════════════════════════════" << Color::RESET << endl;
}

// ==========================================
// 全域重置與管理
// ==========================================

// 前向宣告
void openMenu(vector<Character*>& team, vector<Character*>& reserve);
bool useItemMenu(vector<Character*>& team);
void openShop();

// 重新開始遊戲 (初始化所有狀態)
void resetGame(vector<Character*>& team, vector<Character*>& reserve) {
    printMessage("\n系統啟動中...", "", 50, Color::BLUE);

    // 清空角色
    for(auto* c : team) delete c;
    for(auto* c : reserve) delete c;
    team.clear();
    reserve.clear();

    // 重置全域狀態
    gState = {0, 200, 0, false, false, false};
    currentLocation = LOCATIONS[1];
    inventory.clear();
    
    // 初始化商店
    for(auto* i : shopItems) delete i;
    shopItems.clear();
    shopItems.push_back(new RestoreItem("鰻魚飯", 300, "恢復 100 HP", 100));
    shopItems.push_back(new RestoreItem("阿笠博士特製藥", 500, "恢復 200 HP", 200));
    shopItems.push_back(new RestoreItem("波羅麵包", 100, "恢復 50 HP", 50));
    shopItems.push_back(new ReviveItem("急救箱", 600, "復活並恢復 50% HP"));

    // 重新建立主角與隊友
    
    printMessage("\n系統正在載入使用者資料...\n", "", 20, Color::BLUE);
    team.push_back(new Gadgeteer("江戶川柯南", 1));
    wait(500);

    printMessage("正在隨機連線隊友...\n", "", 20, Color::BLUE);
    for(int i=0; i<3; ++i) {
        Character* npc = createRandomNPC();
        // 簡單去重
        bool exists = false;
        for(auto* m : team) if(m->getName() == npc->getName()) exists = true;
        if(exists) { delete npc; i--; } 
        else { team.push_back(npc); printMessage(">>> " + npc->getName() + " 加入了隊伍！", "", 20, Color::GREEN); }
    }

    // 初始道具
    addToInventory(shopItems[0]); 
    inventory[0].count = 3; 

    Story::triggerChapter0();
    wait(1000);
}

// 隨機事件
void triggerRandomEvent(vector<Character*>& team, vector<Character*>& reserve) {
    if (getRandom(1, 100) > 60) return;
    
    // 可觸發事件列表
    vector<string> events;
    if(currentLocation.id > 1) events.push_back("SHARK");
    if(currentLocation.id > 1) events.push_back("SYSTEM_ERROR");
    if(currentLocation.id > 0) events.push_back("LUNCH");
    if(currentLocation.id > 0) events.push_back("RAN_KARATE");
    if(currentLocation.id > 0) events.push_back("AGASA_QUIZ");
    if(events.empty()) return;

    printMessage("\n[隨機事件]", "", 0, Color::MAGENTA);
    string type = events[getRandom(0, events.size()-1)];

    if (type == "SHARK") {
        printMessage("透過玻璃窗看到巨大的鯊魚游過...", "");
    } else if (type == "SYSTEM_ERROR") {
        printMessage("系統出現短暫的雜訊...", "諾亞方舟");
        printMessage("別擔心，只是防火牆攔截了一次攻擊。", "諾亞方舟");
    } else if (type == "LUNCH") {
        printMessage("元太肚子餓了，吵著要吃鰻魚飯...", "");
        gState.playerMoney -= 30; if(gState.playerMoney < 0) gState.playerMoney = 0;
        printMessage("只好花錢買點東西吃 (金錢-30)", "", 20, Color::RED);
    } else if (type == "RAN_KARATE") {
        printMessage("有人想偷襲！哈啊——！", "毛利蘭");
        printMessage("小蘭一腳踢飛了可疑的無人機。", "");
        gState.playerClues++;
        printMessage("從殘骸中發現了晶片 (線索+1)", "", 20, Color::GREEN);
    } else if (type == "AGASA_QUIZ") {
        printMessage("現在是博士的猜謎時間！", "阿笠博士");
        printMessage("答對了！獎勵大家恢復體力！(全員HP+50)", "", 20, Color::GREEN);
        for(auto* c : team) if(c->getHP() > 0) c->setHP(c->getHP() + 50);
    }
    wait(1000);
}

// 戰鬥函式
void battle(vector<Character*>& team, Monster* monster) {
    printMessage("=== 戰鬥開始 ===", "", 30, Color::RED);

    // 戰鬥前劇情
    if (monster->name == "基爾 (Kir)") printMessage("對不起了，我不能在這裡暴露身分...", "基爾");
    else if (monster->name == "苦艾酒 (Vermouth)") printMessage("A secret makes a woman woman...", "苦艾酒");
    else if (monster->name == "伏特加 (Vodka)") printMessage("老大說了，今天一定要拿下你們！", "伏特加");
    else if (monster->name == "琴酒 (Gin)") printMessage("哼，一群老鼠。", "琴酒");

    // 削弱機制
    if (monster->type == BOSS && gState.playerClues >= 5) {
            printMessage("利用掌握的情報，看穿了 " + monster->name + " 的破綻！", "", 20, Color::GREEN);
            gState.playerClues -= 5;
            monster->attack = (int)(monster->attack * 0.7);
            monster->hp = (int)(monster->hp * 0.7);
            monster->maxHp = monster->hp;
    }
    if (monster->type == ELITE && gState.playerClues >= 3) {
            printMessage("利用掌握的情報，看穿了 " + monster->name + " 的破綻！", "", 20, Color::GREEN);
            gState.playerClues -= 3;
            monster->attack = (int)(monster->attack * 0.8);
            monster->hp = (int)(monster->hp * 0.8);
            monster->maxHp = monster->hp;
    }
    

    // 戰鬥初始化
    for(auto* c : team) { c->clearBuff(); c->resetCooldowns(); }
    // 戰鬥迴圈
    int round = 1;
    while (monster->getHP() > 0) {
        // 顯示狀態
        printBattleStatus(team, monster);
        cout << Color::BLUE << "--- Round " << round << " ---" << Color::RESET << endl;
        // 玩家回合
        for (auto* member : team) {
            // 檢查存活
            if (member->getHP() > 0 && monster->getHP() > 0) {
                cout << "輪到 " << Color::BOLD << member->getName() << Color::RESET << "\n";
                int damage = 0;
                // 玩家選擇行動
                if (member->getIsPlayer()) {
                    bool validAction = false;
                    // 行動選單
                    while (!validAction) {
                        cout << "1. 普通攻擊\n";
                        const auto& skills = member->getSkills();
                        // 列出技能
                        for(size_t i=0; i<skills.size(); ++i) {
                            string status = "";
                            string color = Color::RESET;
                            if (!skills[i]->isReady()) {
                                status = " (冷卻中 " + to_string(skills[i]->getCurrentCD()) + ")";
                                color = Color::GRAY;
                            } else {
                                status = " (CD:" + to_string(skills[i]->getMaxCD()) + ")";
                            }
                            cout << (i + 2) << ". " << color << "技能: " << skills[i]->getName() << status << Color::RESET << "\n";
                        }
                        // 列出道具選項
                        int itemOpt = skills.size() + 2;
                        cout << itemOpt << ". 使用道具\n";
                        // 取得有效輸入
                        int choice = getValidInput(1, itemOpt);
                        // 處理選擇
                        if (choice == 1) { // 普通攻擊 
                            damage = member->getAttack();
                            damage = getRandom((int)(damage*0.8), (int)(damage*1.2));
                            if (getRandom(1, 100) < 40) {
                                string quote = member->getQuote("ATTACK");
                                if (quote != "") printMessage(quote, member->getName());
                            }
                            printMessage(member->getName() + " 進行攻擊！");
                            validAction = true;
                        } else if (choice == itemOpt) { // 使用道具 
                            if (useItemMenu(team)) validAction = true;
                            else cout << "取消使用，請重新選擇行動。\n";
                            damage = 0; 
                        } else { // 使用技能
                            int skillIdx = choice - 2;
                            if (skills[skillIdx]->isReady()) {
                                damage = member->performSkill(skillIdx, team);
                                validAction = true;
                            } else {
                                cout << Color::RED << "該技能冷卻中！請選擇其他行動。\n" << Color::RESET;
                            }
                        }
                    }
                // 電腦隊友行動
                } else {
                    wait(300);
                    if(getRandom(1,10)>5) damage = member->useRandomSkill(team);
                    else { damage = member->getAttack(); printMessage(member->getName() + " 攻擊！"); }
                }
                // 計算傷害並套用
                if (damage > 0) {
                    monster->setHP(monster->getHP() - damage);
                    printMessage("造成 " + to_string(damage) + " 傷害！");
                }
                wait(200);
            }
        }
        // 戰鬥結束判定
        if (monster->getHP() <= 0) {
            printMessage("\n" + monster->name + " 被擊敗了！", "", 50, Color::GREEN);
            
            // 戰鬥勝利語音
            for (auto* member : team) {
                if (member->getHP() > 0 && getRandom(1, 100) < 40) {
                    string quote = member->getQuote("WIN");
                    if (quote != "") printMessage(quote, member->getName());
                    break; 
                }
            }
            // 章節觸發檢查
            if (monster->name == "基爾 (Kir)") { 
                gState.boss_Kir = true; 
                Story::triggerChapter4(); 
            } else if (monster->name == "苦艾酒 (Vermouth)") { 
                gState.boss_Vermouth = true; 
                Story::triggerChapter5(); 
            } else if (monster->name == "伏特加 (Vodka)") { 
                gState.boss_Vodka = true; 
                Story::triggerChapter6(); 
            } else if (monster->name == "琴酒 (Gin)") { 
                gState.boss_Gin = true; 
                Story::triggerChapter7();
                currentLocation = LOCATIONS[7]; 
                Story::triggerChapter8();
            }
            
            // 戰利品發放
            gState.playerMoney += monster->moneyDrop;
            for(auto* member : team) if(member->getHP() > 0) member->beatMonster(monster->type == BOSS ? 2000 : 150);
            return;
        }

        // 怪物回合
        if(team.size() > 0) {
            // 篩選存活角色
            vector<Character*> aliveTeam;
            for (auto* c : team) {
                if (c->getHP() > 0) aliveTeam.push_back(c);
            }
            // 隨機選擇一名存活角色攻擊
            if (!aliveTeam.empty()) {
                printMessage(monster->name + " 反擊！", "", 20, Color::MAGENTA);
                int targetIdx = getRandom(0, aliveTeam.size()-1);
                Character* target = aliveTeam[targetIdx];
                // 閃避判定: 1-100 隨機數 < 角色速度(幸運)
                if (getRandom(1, 100) < target->getSpeed()) {
                    printMessage(target->getName() + " 靈巧地閃過了攻擊！", "", 20, Color::GREEN);
                } else {
                    target->setHP(target->getHP() - monster->attack);
                    printMessage(target->getName() + " 受到 " + to_string(monster->attack) + " 傷害！");
                }
            }
        }

        // 回合結束處理
        for(auto* c : team) c->tickCooldowns();
        
        // 全滅判定
        bool allDead = true;
        for(auto* c : team) if(c->getHP() > 0) allDead = false;
        if(allDead) {
            printMessage("GAME OVER... 諾亞方舟被組織奪走了...", "", 50, Color::RED);
            // 失敗直接重來
            return;
        }
        round++;
    }
}

// 產生隨機 NPC
Character* createRandomNPC() {
    int roll = getRandom(1, 13);
    switch(roll) {
        // Fighter
        case 1: return new Fighter("毛利蘭", "Karate", 1);
        case 2: return new Fighter("服部平次", "Kendo", 1);
        case 3: return new Fighter("京極真", "Super", 1);
        case 4: return new Fighter("赤井秀一", "Sniper", 1);
        case 5: return new Fighter("安室透", "SecretPolice", 1);
        case 6: return new Fighter("遠山和葉", "Aikido", 1);
        // Support
        case 7: return new Support("灰原哀", "Science", 1);
        case 8: return new Support("阿笠博士", "Inventor", 1);
        case 9: return new Support("鈴木園子", "Rich", 1);
        case 10: return new Support("工藤優作", "Novelist", 1);
        // Trickster
        case 11: return new Trickster("怪盜基德", "Thief", 1);
        case 12: return new Trickster("毛利小五郎", "Sleep", 1);
        case 13: return new Trickster("工藤有希子", "Actress", 1);
    }
    return new Fighter("毛利蘭", "Karate", 1);
}

// 搜查周邊
void investigate(vector<Character*>& team, vector<Character*>& reserve) {
    printMessage("=== 開始搜查周邊 ===", "", 20, Color::CYAN);
    // 成功率計算
    int successRate = 50 + currentLocation.investigationBonus;
    if(getRandom(1,100) < successRate) {
        gState.playerClues += getRandom(1, 2);
        printMessage("發現了關鍵線索！(總計: " + to_string(gState.playerClues) + ")", "", 20, Color::GREEN);

    } else {
        printMessage("一無所獲...", "", 20, Color::GRAY);
    }

    // 特殊事件：遇見新夥伴
    if (getRandom(1, 100) <= 5) { 
        printMessage("\n[特殊事件] 好像看到了熟悉的身影...", "", 0, Color::MAGENTA);
        wait(1000);
        
        Character* newChar = nullptr;
        for(int attempt=0; attempt<5; ++attempt) {
            Character* candidate = createRandomNPC();
            bool exists = false;
            for(auto* m : team) if(m->getName() == candidate->getName()) exists = true;
            for(auto* m : reserve) if(m->getName() == candidate->getName()) exists = true;
            
            if(!exists) {
                newChar = candidate;
                break;
            } else {
                delete candidate;
            }
        }

        if (newChar) {
            printMessage("發現了 " + newChar->getName() + " 正在此處調查！", "", 20, Color::GREEN);
            if(team.size() < 4) {
                team.push_back(newChar);
                printMessage(">>> " + newChar->getName() + " 加入了隊伍！", "", 20, Color::GREEN);
            } else {
                reserve.push_back(newChar);
                printMessage(">>> 隊伍已滿，" + newChar->getName() + " 先前往待命區等候。", "", 20, Color::GREEN);
            }
        } else {
            printMessage("但對方似乎只是路人... (沒發現新夥伴)", "", 20, Color::GRAY);
        }
    }

    triggerRandomEvent(team, reserve);
}

// 地點移動
void changeLocation() {
    printMessage("=== 移動地點 ===", "", 0, Color::CYAN);
    // 列出地點選項
    for(size_t i=0; i<LOCATIONS.size(); ++i) {
        string locked = (gState.chapter < LOCATIONS[i].requiredChapter) ? " (未解鎖)" : "";
        string color = (gState.chapter < LOCATIONS[i].requiredChapter) ? Color::GRAY : Color::RESET;
        cout << color << i+1 << ". " << LOCATIONS[i].name << locked << "\n" << Color::RESET;
    }
    int choice = getValidInput(1, LOCATIONS.size());
    // 檢查解鎖條件
    if (gState.chapter >= LOCATIONS[choice-1].requiredChapter) {
        currentLocation = LOCATIONS[choice-1];
        printMessage("移動到了 " + currentLocation.name, "", 20, Color::GREEN);
    } else {
        printMessage("該區域尚未解鎖！", "", 20, Color::RED);
    }
}

// 隊伍與道具管理選單
void openMenu(vector<Character*>& team, vector<Character*>& reserve) {
    while(true) {
        printMessage("\n=== 隊伍與道具管理 ===", "", 0, Color::CYAN);
        // 列出隊伍成員
        cout << Color::YELLOW << "[出戰]" << Color::RESET << endl;
        for(size_t i=0; i<team.size(); ++i) { cout << " " << (i+1) << ". "; team[i]->print(); }
        if (!reserve.empty()) {
            cout << Color::YELLOW << "[待命]" << Color::RESET << endl;
            for(size_t i=0; i<reserve.size(); ++i) { cout << " " << (i+1) << ". "; reserve[i]->print(); }
        }
        // 選單選項
        cout << "1.使用道具\n2.替換成員\n0.返回\n";
        int choice = getValidInput(0, 2);
        if (choice == 0) break;
        if (choice == 1) useItemMenu(team); 
        else if (choice == 2) {
            // 替換成員邏輯
            if (reserve.empty()) { printMessage("無待命成員！", "", 0, Color::RED); continue; }
            cout << "換下編號(0取消): "; int outIdx = getValidInput(0, team.size()); if(outIdx==0) continue;
            if(team[outIdx-1]->getIsPlayer()) { printMessage("隊長不可替換！", "", 0, Color::RED); continue; }
            cout << "換上編號(0取消): "; int inIdx = getValidInput(0, reserve.size()); if(inIdx==0) continue;
            Character* outC = team[outIdx-1];
            Character* inC = reserve[inIdx-1];
            team.erase(team.begin()+outIdx-1); reserve.erase(reserve.begin()+inIdx-1);
            team.push_back(inC); reserve.push_back(outC);
            printMessage("隊伍變更！", "", 0, Color::GREEN);
        }
    }
}

// 商店系統
void openShop() {
    printMessage("=== 五稜星補給站 ===", "", 20, Color::YELLOW);
    while(true) {
        // 列出商品
        cout << Color::CYAN << "\n持有金錢: " << gState.playerMoney << " 円" << Color::RESET << endl;
        cout << "--------------------------------\n";
        for(size_t i=0; i<shopItems.size(); ++i) cout << i+1 << ". " << shopItems[i]->getName() << " - " << shopItems[i]->getPrice() << "円 (" << shopItems[i]->getDesc() << ")\n";
        cout << "0. 離開商店\n";
        int choice = getValidInput(0, shopItems.size(), "請選擇購買商品: ");
        if (choice == 0) break;
        // 購買邏輯
        Item* selected = shopItems[choice-1];
        if (gState.playerMoney >= selected->getPrice()) {
            gState.playerMoney -= selected->getPrice();
            addToInventory(selected); 
            printMessage("購買了 " + selected->getName() + "！", "", 20, Color::GREEN);
        } else {
            printMessage("金錢不足！", "", 10, Color::RED);
        }
    }
}

// 使用道具選單
bool useItemMenu(vector<Character*>& team) {
    // 列出背包道具
    if (inventory.empty()) { printMessage("背包是空的！", "", 10, Color::RED); return false; }
    cout << Color::YELLOW << "=== 背包 ===" << Color::RESET << endl;
    for (size_t i = 0; i < inventory.size(); ++i) cout << i + 1 << ". " << inventory[i].item->getName() << " (x" << inventory[i].count << ")\n";
    cout << "0. 取消\n";
    int choice = getValidInput(0, inventory.size(), "選擇要使用的道具: ");
    if (choice == 0) return false;
    // 選擇目標
    InventorySlot& slot = inventory[choice - 1];
    Item* itemToUse = slot.item;
    cout << "選擇目標:\n";
    for(size_t i=0; i<team.size(); ++i) cout << i+1 << ". " << team[i]->getName() << " (HP: " << team[i]->getHP() << "/" << team[i]->getMaxHP() << ")\n"; 
    int targetIdx = getValidInput(1, team.size()) - 1;
    bool success = itemToUse->apply(team[targetIdx]);
    // 使用後處理
    if (success) {
        slot.count--;
        if (slot.count <= 0) inventory.erase(inventory.begin() + (choice - 1));
        wait(500);
        return true;
    }
    return false;
}

// ==========================================
// 主程式 (Main Loop)
// ==========================================

int main() {
    setupConsole(); // 設定編碼為 UTF-8 (Windows)
    
    // 隊伍與待命成員
    vector<Character*> team;
    vector<Character*> reserve;

    // 遊戲主迴圈
    bool appRunning = true;
    while(appRunning) {
        // 初始化遊戲
        resetGame(team, reserve);

        // 遊戲內迴圈
        bool playing = true;
        while (playing) {
            // 章節觸發檢查
            if(gState.chapter == 0 && gState.playerClues >= 1) { 
                gState.playerClues -= 1; 
                Story::triggerChapter1();
            }
            if(gState.chapter == 1 && gState.playerClues >= 3 && currentLocation.id == 1) { 
                gState.playerClues -= 3;
                Story::triggerChapter2();
            }
            if(gState.chapter == 2 && gState.playerClues >= 5 && currentLocation.id == 2) { 
                gState.playerClues -= 5;
                Story::triggerChapter3();
            }

            // 顯示狀態與選單 
            cout << Color::CYAN << "\n==================================" << Color::RESET << endl;
            cout << Color::CYAN << "[地點]: " << currentLocation.name << " | [章節]: " << gState.chapter << endl;
            cout << Color::CYAN << "[金錢]: " << gState.playerMoney << " 円 | [線索]: " << gState.playerClues << Color::RESET << endl;
            cout << "1.戰鬥\n2.移動\n3.商店\n4.隊伍\n5.搜查\n0.退出遊戲" << Color::RESET << endl;
            int action = getValidInput(0, 5);
            // 處理選單行動
            if (action == 1) {
                Monster monster = generateMonster(team);
                monster.print();
                battle(team, &monster);
                
                // 檢查是否全滅
                bool allDead = true;
                for(auto* c : team) if(c->getHP() > 0) allDead = false;
                if(allDead) playing = false;
            } 
            else if (action == 2) { changeLocation(); triggerRandomEvent(team, reserve); } 
            else if (action == 3) { openShop(); }
            else if (action == 4) { openMenu(team, reserve); }
            else if (action == 5) { investigate(team, reserve); }
            else if (action == 0) { playing = false; }
        }

        // 結算畫面與重玩詢問
        cout << "\n==================================\n";
        cout << "是否要重新開始遊戲？\n";
        cout << "1. 重新開始遊戲 (初始化所有進度)\n";
        cout << "0. 離開程式\n";
        cout << "==================================\n";
        
        int choice = getValidInput(0, 1);
        if (choice == 0) appRunning = false;
        // 否則繼續迴圈重玩
    }

    // 清理記憶體
    for(auto* c : team) delete c;
    for(auto* c : reserve) delete c;
    for(auto* i : shopItems) delete i; 

    return 0;
}
