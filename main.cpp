#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

static mt19937 rng{random_device{}()};

int randInt(int a, int b) { return uniform_int_distribution<int>(a, b)(rng); }
bool chance(int percent) { return randInt(1, 100) <= percent; }

int readInt(int lo, int hi) {
    while (true) {
        cout << "> ";
        string line;
        if (!getline(cin, line)) exit(0);
        try {
            size_t pos = 0;
            int v = stoi(line, &pos);
            if (v >= lo && v <= hi) return v;
        } catch (...) {
        }
        cout << "Введіть число від " << lo << " до " << hi << ".\n";
    }
}

void pauseEnter() {
    cout << "\n(Натисніть Enter...)";
    string s;
    getline(cin, s);
}

void line(char ch = '=', int n = 44) { cout << string(n, ch) << "\n"; }

string bar(int cur, int mx, int width = 20) {
    if (mx <= 0) mx = 1;
    int filled = max(0, min(width, cur * width / mx));
    return "[" + string(filled, '#') + string(width - filled, '-') + "]";
}

vector<string> split(const string& s, char sep) {
    vector<string> out;
    string cur;
    for (char ch : s) {
        if (ch == sep) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += ch;
        }
    }
    out.push_back(cur);
    return out;
}

enum class ClassType { Warrior = 0, Mage = 1, Archer = 2 };
enum class ItemType { Weapon = 0, Armor = 1, PotionHP = 2, PotionMana = 3 };

const char* className(ClassType c) {
    switch (c) {
        case ClassType::Warrior: return "Воїн";
        case ClassType::Mage: return "Маг";
        default: return "Лучник";
    }
}

struct Item {
    string name;
    ItemType type = ItemType::Weapon;
    int value = 0;
    int price = 0;
    int classReq = -1;
};

string describeItem(const Item& it) {
    ostringstream o;
    o << it.name << " (";
    switch (it.type) {
        case ItemType::Weapon: o << "зброя, атака +" << it.value; break;
        case ItemType::Armor: o << "броня, захист +" << it.value; break;
        case ItemType::PotionHP: o << "зілля, +" << it.value << " HP"; break;
        case ItemType::PotionMana: o << "зілля, +" << it.value << " мани"; break;
    }
    o << ")";
    if (it.classReq >= 0) o << " [" << className((ClassType)it.classReq) << "]";
    return o.str();
}

vector<Item> weaponsFor(ClassType c) {
    int r = (int)c;
    switch (c) {
        case ClassType::Warrior:
            return {{"Іржавий меч", ItemType::Weapon, 2, 10, r},
                    {"Сталевий меч", ItemType::Weapon, 6, 80, r},
                    {"Меч лицаря", ItemType::Weapon, 12, 220, r},
                    {"Клинок героя", ItemType::Weapon, 20, 550, r}};
        case ClassType::Mage:
            return {{"Посох учня", ItemType::Weapon, 2, 10, r},
                    {"Посох чародія", ItemType::Weapon, 6, 80, r},
                    {"Посох мудреця", ItemType::Weapon, 12, 220, r},
                    {"Посох архімага", ItemType::Weapon, 20, 550, r}};
        default:
            return {{"Мисливський лук", ItemType::Weapon, 2, 10, r},
                    {"Складний лук", ItemType::Weapon, 6, 80, r},
                    {"Ельфійський лук", ItemType::Weapon, 12, 220, r},
                    {"Лук північного вітру", ItemType::Weapon, 20, 550, r}};
    }
}

vector<Item> allArmors() {
    return {{"Шкіряна броня", ItemType::Armor, 3, 40, -1},
            {"Кольчуга", ItemType::Armor, 6, 120, -1},
            {"Латна броня", ItemType::Armor, 10, 300, -1},
            {"Драконяча броня", ItemType::Armor, 16, 700, -1}};
}

vector<Item> allPotions() {
    return {{"Мале зілля здоров'я", ItemType::PotionHP, 40, 25, -1},
            {"Велике зілля здоров'я", ItemType::PotionHP, 100, 60, -1},
            {"Мале зілля мани", ItemType::PotionMana, 40, 30, -1},
            {"Велике зілля мани", ItemType::PotionMana, 100, 70, -1}};
}

enum QuestType { QT_KILL = 0, QT_BOSS = 1, QT_GOLD = 2, QT_CRIT = 3 };

struct Quest {
    int type;
    string title;
    string desc;
    int target;
    int rewardGold;
    int rewardExp;
    int state = 0;
    int progress = 0;
};

vector<Quest> makeQuests() {
    return {{QT_KILL, "Мисливець-початківець", "Переможіть 3 монстрів", 3, 60, 50},
            {QT_CRIT, "Точний удар", "Завдайте 5 критичних ударів", 5, 80, 60},
            {QT_GOLD, "Шукач скарбів", "Здобудьте 200 золота", 200, 100, 80},
            {QT_KILL, "Зачистка території", "Переможіть 10 монстрів", 10, 200, 150},
            {QT_BOSS, "Вбивця босів", "Переможіть 1 боса", 1, 300, 250}};
}

struct Stats {
    int battles = 0, monstersKilled = 0, bossesKilled = 0, deaths = 0, fled = 0;
    int crits = 0, damageDealt = 0, damageTaken = 0, maxHit = 0;
    int goldEarned = 0, goldSpent = 0, potionsUsed = 0, questsDone = 0;
};

const int INVENTORY_LIMIT = 20;

struct Character {
    string name;
    ClassType cls = ClassType::Warrior;
    int level = 1, exp = 0, expNext = 50;
    int maxHp = 0, hp = 0, maxMana = 0, mana = 0;
    int strength = 0, baseDef = 0, gold = 0;
    vector<Item> inv;
    bool hasWeapon = false, hasArmor = false;
    Item weapon, armor;
    Stats st;
    vector<Quest> quests = makeQuests();

    int attackPower() const { return strength + (hasWeapon ? weapon.value : 0); }
    int defense() const { return baseDef + (hasArmor ? armor.value : 0); }
    int critChance() const { return cls == ClassType::Archer ? 25 : 12; }
};

void questEvent(Character& c, int type, int amount) {
    for (auto& q : c.quests) {
        if (q.state == 1 && q.type == type) {
            q.progress = min(q.target, q.progress + amount);
            if (q.progress >= q.target) {
                q.state = 2;
                cout << ">>> Квест \"" << q.title << "\" виконано! Здайте його в меню квестів.\n";
            }
        }
    }
}

void addGold(Character& c, int amount) {
    c.gold += amount;
    c.st.goldEarned += amount;
    questEvent(c, QT_GOLD, amount);
}

void gainExp(Character& c, int amount) {
    c.exp += amount;
    while (c.exp >= c.expNext) {
        c.exp -= c.expNext;
        c.level++;
        c.expNext = 50 * c.level;
        int dHp, dMana, dStr, dDef;
        switch (c.cls) {
            case ClassType::Warrior: dHp = 20; dMana = 3; dStr = 3; dDef = 2; break;
            case ClassType::Mage: dHp = 10; dMana = 15; dStr = 2; dDef = 1; break;
            default: dHp = 14; dMana = 6; dStr = 3; dDef = 1; break;
        }
        c.maxHp += dHp;
        c.maxMana += dMana;
        c.strength += dStr;
        c.baseDef += dDef;
        c.hp = c.maxHp;
        c.mana = c.maxMana;
        line('*');
        cout << "РІВЕНЬ ПІДВИЩЕНО! Тепер у вас " << c.level << " рівень!\n";
        cout << "HP +" << dHp << ", Мана +" << dMana << ", Сила +" << dStr << ", Захист +" << dDef << "\n";
        cout << "HP і мана повністю відновлені.\n";
        line('*');
    }
}

Character createCharacter() {
    Character c;
    line();
    cout << "        СТВОРЕННЯ ПЕРСОНАЖА\n";
    line();
    cout << "Введіть ім'я героя:\n> ";
    string name;
    getline(cin, name);
    replace(name.begin(), name.end(), '|', ' ');
    if (name.empty()) name = "Герой";
    c.name = name;

    cout << "\nОберіть клас:\n";
    cout << "1. Воїн   - багато HP та захисту, вміння \"Потужний удар\"\n";
    cout << "2. Маг    - багато мани, вміння \"Вогняна куля\" (ігнорує захист)\n";
    cout << "3. Лучник - високий шанс крита, вміння \"Подвійний постріл\"\n";
    int ch = readInt(1, 3);
    c.cls = (ClassType)(ch - 1);

    switch (c.cls) {
        case ClassType::Warrior: c.maxHp = 120; c.maxMana = 20; c.strength = 12; c.baseDef = 6; break;
        case ClassType::Mage: c.maxHp = 80; c.maxMana = 80; c.strength = 9; c.baseDef = 2; break;
        default: c.maxHp = 95; c.maxMana = 40; c.strength = 11; c.baseDef = 4; break;
    }
    c.hp = c.maxHp;
    c.mana = c.maxMana;
    c.gold = 50;

    c.weapon = weaponsFor(c.cls)[0];
    c.hasWeapon = true;
    c.inv.push_back(allPotions()[0]);
    c.inv.push_back(allPotions()[0]);

    cout << "\nВітаємо, " << c.name << " (" << className(c.cls) << ")! Вас чекають пригоди!\n";
    return c;
}

void showStats(const Character& c) {
    line();
    cout << "  " << c.name << " - " << className(c.cls) << ", рівень " << c.level << "\n";
    line();
    cout << "Досвід : " << c.exp << "/" << c.expNext << "\n";
    cout << "HP     : " << c.hp << "/" << c.maxHp << " " << bar(c.hp, c.maxHp) << "\n";
    cout << "Мана   : " << c.mana << "/" << c.maxMana << " " << bar(c.mana, c.maxMana) << "\n";
    cout << "Сила   : " << c.strength << "  (атака зі зброєю: " << c.attackPower() << ")\n";
    cout << "Захист : " << c.baseDef << "  (разом з бронею: " << c.defense() << ")\n";
    cout << "Крит   : " << c.critChance() << "% (урон x2)\n";
    cout << "Золото : " << c.gold << "\n";
    cout << "Зброя  : " << (c.hasWeapon ? describeItem(c.weapon) : "немає") << "\n";
    cout << "Броня  : " << (c.hasArmor ? describeItem(c.armor) : "немає") << "\n";
    line('-');
    cout << "Битв: " << c.st.battles << " | Монстрів: " << c.st.monstersKilled
         << " | Босів: " << c.st.bossesKilled << "\n";
    cout << "Смертей: " << c.st.deaths << " | Втеч: " << c.st.fled << "\n";
    cout << "Критичних ударів: " << c.st.crits << " | Найсильніший удар: " << c.st.maxHit << "\n";
    cout << "Завдано шкоди: " << c.st.damageDealt << " | Отримано шкоди: " << c.st.damageTaken << "\n";
    cout << "Золота зароблено: " << c.st.goldEarned << " | витрачено: " << c.st.goldSpent << "\n";
    cout << "Зілль випито: " << c.st.potionsUsed << " | Квестів виконано: " << c.st.questsDone << "\n";
    line();
}

bool usePotion(Character& c) {
    vector<int> idx;
    for (int i = 0; i < (int)c.inv.size(); i++)
        if (c.inv[i].type == ItemType::PotionHP || c.inv[i].type == ItemType::PotionMana) idx.push_back(i);
    if (idx.empty()) {
        cout << "У вас немає зілль.\n";
        return false;
    }
    cout << "Оберіть зілля:\n";
    for (int i = 0; i < (int)idx.size(); i++) cout << i + 1 << ". " << describeItem(c.inv[idx[i]]) << "\n";
    cout << "0. Скасувати\n";
    int ch = readInt(0, (int)idx.size());
    if (ch == 0) return false;

    Item it = c.inv[idx[ch - 1]];
    if (it.type == ItemType::PotionHP) {
        if (c.hp >= c.maxHp) {
            cout << "HP вже повне.\n";
            return false;
        }
        int heal = min(it.value, c.maxHp - c.hp);
        c.hp += heal;
        cout << "Ви випили \"" << it.name << "\" і відновили " << heal << " HP.\n";
    } else {
        if (c.mana >= c.maxMana) {
            cout << "Мана вже повна.\n";
            return false;
        }
        int rest = min(it.value, c.maxMana - c.mana);
        c.mana += rest;
        cout << "Ви випили \"" << it.name << "\" і відновили " << rest << " мани.\n";
    }
    c.inv.erase(c.inv.begin() + idx[ch - 1]);
    c.st.potionsUsed++;
    return true;
}

void equipItem(Character& c) {
    vector<int> idx;
    for (int i = 0; i < (int)c.inv.size(); i++)
        if (c.inv[i].type == ItemType::Weapon || c.inv[i].type == ItemType::Armor) idx.push_back(i);
    if (idx.empty()) {
        cout << "У інвентарі немає зброї чи броні.\n";
        return;
    }
    cout << "Що екіпірувати?\n";
    for (int i = 0; i < (int)idx.size(); i++) cout << i + 1 << ". " << describeItem(c.inv[idx[i]]) << "\n";
    cout << "0. Скасувати\n";
    int ch = readInt(0, (int)idx.size());
    if (ch == 0) return;

    Item it = c.inv[idx[ch - 1]];
    if (it.type == ItemType::Weapon) {
        if (it.classReq >= 0 && it.classReq != (int)c.cls) {
            cout << "Ця зброя не підходить вашому класу!\n";
            return;
        }
        c.inv.erase(c.inv.begin() + idx[ch - 1]);
        if (c.hasWeapon) c.inv.push_back(c.weapon);
        c.weapon = it;
        c.hasWeapon = true;
    } else {
        c.inv.erase(c.inv.begin() + idx[ch - 1]);
        if (c.hasArmor) c.inv.push_back(c.armor);
        c.armor = it;
        c.hasArmor = true;
    }
    cout << "Ви екіпірували: " << it.name << "\n";
}

void inventoryMenu(Character& c) {
    while (true) {
        line();
        cout << "ІНВЕНТАР (" << c.inv.size() << "/" << INVENTORY_LIMIT << ")  Золото: " << c.gold << "\n";
        line('-');
        cout << "Зброя: " << (c.hasWeapon ? describeItem(c.weapon) : "немає") << "\n";
        cout << "Броня: " << (c.hasArmor ? describeItem(c.armor) : "немає") << "\n";
        line('-');
        if (c.inv.empty()) cout << "(порожньо)\n";
        for (int i = 0; i < (int)c.inv.size(); i++) cout << i + 1 << ". " << describeItem(c.inv[i]) << "\n";
        line('-');
        cout << "1. Екіпірувати зброю/броню\n2. Випити зілля\n0. Назад\n";
        int ch = readInt(0, 2);
        if (ch == 0) return;
        if (ch == 1) equipItem(c);
        if (ch == 2) usePotion(c);
    }
}

struct Monster {
    string name;
    int maxHp, hp, atk, def, exp, gold;
    bool boss = false;
};

Monster makeMonster(int plvl) {
    struct Tpl { const char* name; int hp, atk, def, exp, gold; };
    static const Tpl tpls[] = {{"Слиз", 30, 8, 1, 12, 8},    {"Гоблін", 40, 10, 2, 16, 12},
                               {"Вовк", 50, 12, 3, 20, 15},  {"Скелет", 60, 13, 4, 25, 18},
                               {"Орк", 80, 15, 6, 35, 25}};
    int idx = randInt(0, min(4, 1 + plvl));
    const Tpl& t = tpls[idx];
    int s = plvl - 1;
    Monster m;
    m.name = t.name;
    m.maxHp = m.hp = t.hp + s * 12;
    m.atk = t.atk + s * 2;
    m.def = t.def + s;
    m.exp = t.exp + s * 4;
    m.gold = t.gold + s * 3 + randInt(0, 5);
    return m;
}

struct BossInfo {
    const char* name;
    int hp, atk, def, exp, gold, reqLevel;
};

const BossInfo BOSSES[] = {{"Лісовий Троль", 200, 18, 5, 120, 150, 3},
                           {"Володар печер", 450, 28, 10, 300, 350, 6},
                           {"Дракон Вогняного Піку", 900, 42, 18, 700, 800, 10}};
const int BOSS_COUNT = 3;

Monster makeBoss(int i) {
    const BossInfo& b = BOSSES[i];
    Monster m;
    m.name = b.name;
    m.maxHp = m.hp = b.hp;
    m.atk = b.atk;
    m.def = b.def;
    m.exp = b.exp;
    m.gold = b.gold;
    m.boss = true;
    return m;
}

enum class BattleResult { Win, Lose, Flee };

int rollHit(Character& c, const Monster& m, double mult, bool ignoreDef, bool& crit) {
    double base = c.attackPower() * mult - (ignoreDef ? 0 : m.def / 2.0) + randInt(-2, 2);
    crit = chance(c.critChance());
    if (crit) base *= 2;
    return max(1, (int)base);
}

void dealDamage(Character& c, Monster& m, int dmg, bool crit) {
    m.hp = max(0, m.hp - dmg);
    c.st.damageDealt += dmg;
    c.st.maxHit = max(c.st.maxHit, dmg);
    if (crit) {
        c.st.crits++;
        cout << "   КРИТИЧНИЙ УДАР! ";
        questEvent(c, QT_CRIT, 1);
    } else {
        cout << "   ";
    }
    cout << "Ви завдали " << dmg << " шкоди.\n";
}

const char* skillName(ClassType t) {
    switch (t) {
        case ClassType::Warrior: return "Потужний удар";
        case ClassType::Mage: return "Вогняна куля";
        default: return "Подвійний постріл";
    }
}
int skillCost(ClassType t) { return t == ClassType::Mage ? 15 : 10; }

BattleResult battle(Character& c, Monster m) {
    c.st.battles++;
    line('=');
    if (m.boss) cout << "!!! БОС: " << m.name << " !!!\n";
    else cout << "Вам трапився монстр: " << m.name << "\n";
    line('=');
    int turn = 0;

    while (true) {
        turn++;
        cout << "\n--- Хід " << turn << " ---\n";
        cout << c.name << " HP " << c.hp << "/" << c.maxHp << " " << bar(c.hp, c.maxHp, 15)
             << "  Мана " << c.mana << "/" << c.maxMana << "\n";
        cout << m.name << " HP " << m.hp << "/" << m.maxHp << " " << bar(m.hp, m.maxHp, 15) << "\n";
        cout << "1. Атакувати\n";
        cout << "2. " << skillName(c.cls) << " (мана: " << skillCost(c.cls) << ")\n";
        cout << "3. Випити зілля\n";
        cout << "4. Втекти" << (m.boss ? " (неможливо від боса)" : "") << "\n";

        bool acted = false;
        while (!acted) {
            int ch = readInt(1, 4);
            if (ch == 1) {
                bool crit;
                int dmg = rollHit(c, m, 1.0, false, crit);
                dealDamage(c, m, dmg, crit);
                acted = true;
            } else if (ch == 2) {
                int cost = skillCost(c.cls);
                if (c.mana < cost) {
                    cout << "Недостатньо мани!\n";
                    continue;
                }
                c.mana -= cost;
                cout << "Ви використали \"" << skillName(c.cls) << "\"!\n";
                bool crit = false;
                if (c.cls == ClassType::Warrior) {
                    int dmg = rollHit(c, m, 1.8, false, crit);
                    dealDamage(c, m, dmg, crit);
                } else if (c.cls == ClassType::Mage) {
                    int dmg = rollHit(c, m, 2.2, true, crit);
                    dealDamage(c, m, dmg, crit);
                } else {
                    for (int k = 0; k < 2; k++) {
                        int dmg = rollHit(c, m, 1.0, false, crit);
                        dealDamage(c, m, dmg, crit);
                    }
                }
                acted = true;
            } else if (ch == 3) {
                if (usePotion(c)) acted = true;
                else cout << "Оберіть іншу дію.\n";
            } else {
                if (m.boss) {
                    cout << "Від боса не втекти!\n";
                    continue;
                }
                if (chance(50)) {
                    cout << "Ви успішно втекли!\n";
                    c.st.fled++;
                    return BattleResult::Flee;
                }
                cout << "Втекти не вдалося!\n";
                acted = true;
            }
        }

        if (m.hp <= 0) {
            cout << "\n*** " << m.name << " переможений! ***\n";
            cout << "Нагорода: " << m.exp << " досвіду, " << m.gold << " золота.\n";
            addGold(c, m.gold);
            if (m.boss) {
                c.st.bossesKilled++;
                questEvent(c, QT_BOSS, 1);
                if (c.inv.size() < INVENTORY_LIMIT) {
                    c.inv.push_back(allPotions()[1]);
                    cout << "З боса випало: Велике зілля здоров'я!\n";
                }
            } else {
                c.st.monstersKilled++;
                questEvent(c, QT_KILL, 1);
                if (chance(30) && c.inv.size() < INVENTORY_LIMIT) {
                    c.inv.push_back(allPotions()[0]);
                    cout << "З монстра випало: Мале зілля здоров'я!\n";
                }
            }
            gainExp(c, m.exp);
            return BattleResult::Win;
        }

        double dmgF = m.atk - c.defense() / 2.0 + randInt(-2, 2);
        bool special = m.boss && turn % 3 == 0;
        if (special) {
            dmgF *= 1.5;
            cout << m.name << " б'є ПОТУЖНОЮ атакою!\n";
        }
        if (chance(5)) {
            dmgF *= 1.5;
            cout << m.name << " завдає критичного удару!\n";
        }
        int dmg = max(1, (int)dmgF);
        c.hp = max(0, c.hp - dmg);
        c.st.damageTaken += dmg;
        cout << m.name << " завдає вам " << dmg << " шкоди.\n";

        if (c.hp <= 0) {
            cout << "\nВи загинули у битві...\n";
            c.st.deaths++;
            int lost = c.gold / 4;
            c.gold -= lost;
            c.hp = max(1, c.maxHp / 2);
            cout << "Вас врятували жерці: втрачено " << lost << " золота, HP відновлено наполовину.\n";
            return BattleResult::Lose;
        }
    }
}

void bossMenu(Character& c) {
    line();
    cout << "ЛОГОВА БОСІВ\n";
    line('-');
    for (int i = 0; i < BOSS_COUNT; i++) {
        cout << i + 1 << ". " << BOSSES[i].name << " (потрібен рівень " << BOSSES[i].reqLevel
             << ", HP " << BOSSES[i].hp << ")\n";
    }
    cout << "0. Назад\n";
    int ch = readInt(0, BOSS_COUNT);
    if (ch == 0) return;
    if (c.level < BOSSES[ch - 1].reqLevel) {
        cout << "Ви ще надто слабкі! Потрібен " << BOSSES[ch - 1].reqLevel << " рівень.\n";
        return;
    }
    battle(c, makeBoss(ch - 1));
}

void buyFrom(Character& c, const vector<Item>& goods) {
    line();
    cout << "Ваше золото: " << c.gold << "\n";
    for (int i = 0; i < (int)goods.size(); i++)
        cout << i + 1 << ". " << describeItem(goods[i]) << " - " << goods[i].price << " зол.\n";
    cout << "0. Назад\n";
    int ch = readInt(0, (int)goods.size());
    if (ch == 0) return;
    const Item& it = goods[ch - 1];
    if (c.gold < it.price) {
        cout << "Недостатньо золота!\n";
        return;
    }
    if ((int)c.inv.size() >= INVENTORY_LIMIT) {
        cout << "Інвентар заповнений!\n";
        return;
    }
    c.gold -= it.price;
    c.st.goldSpent += it.price;
    c.inv.push_back(it);
    cout << "Ви купили: " << it.name << "\n";
}

void sellItem(Character& c) {
    if (c.inv.empty()) {
        cout << "Вам нічого продавати.\n";
        return;
    }
    cout << "Що продати? (ціна продажу = половина вартості)\n";
    for (int i = 0; i < (int)c.inv.size(); i++)
        cout << i + 1 << ". " << describeItem(c.inv[i]) << " - " << c.inv[i].price / 2 << " зол.\n";
    cout << "0. Назад\n";
    int ch = readInt(0, (int)c.inv.size());
    if (ch == 0) return;
    int price = c.inv[ch - 1].price / 2;
    cout << "Продано: " << c.inv[ch - 1].name << " за " << price << " зол.\n";
    c.inv.erase(c.inv.begin() + (ch - 1));
    addGold(c, price);
}

void shopMenu(Character& c) {
    while (true) {
        line();
        cout << "           МАГАЗИН   (золото: " << c.gold << ")\n";
        line();
        cout << "1. Купити зброю\n2. Купити броню\n3. Купити зілля\n4. Продати предмет\n0. Вийти\n";
        int ch = readInt(0, 4);
        if (ch == 0) return;
        if (ch == 1) {
            vector<Item> w = weaponsFor(c.cls);
            w.erase(w.begin());
            buyFrom(c, w);
        } else if (ch == 2) {
            buyFrom(c, allArmors());
        } else if (ch == 3) {
            buyFrom(c, allPotions());
        } else {
            sellItem(c);
        }
    }
}

const char* questStateName(int s) {
    switch (s) {
        case 0: return "доступний";
        case 1: return "активний";
        case 2: return "ВИКОНАНО (здайте)";
        default: return "завершений";
    }
}

void questMenu(Character& c) {
    while (true) {
        line();
        cout << "                КВЕСТИ\n";
        line();
        for (int i = 0; i < (int)c.quests.size(); i++) {
            const Quest& q = c.quests[i];
            cout << i + 1 << ". " << q.title << " - " << q.desc << "\n";
            cout << "   Статус: " << questStateName(q.state);
            if (q.state == 1 || q.state == 2) cout << " (" << q.progress << "/" << q.target << ")";
            cout << " | Нагорода: " << q.rewardGold << " зол., " << q.rewardExp << " досвіду\n";
        }
        line('-');
        cout << "1. Взяти квест\n2. Здати квест\n0. Назад\n";
        int ch = readInt(0, 2);
        if (ch == 0) return;
        cout << "Номер квесту:\n";
        int n = readInt(1, (int)c.quests.size());
        Quest& q = c.quests[n - 1];
        if (ch == 1) {
            if (q.state == 0) {
                q.state = 1;
                q.progress = 0;
                cout << "Квест прийнято: " << q.title << "\n";
            } else {
                cout << "Цей квест не можна взяти зараз.\n";
            }
        } else {
            if (q.state == 2) {
                q.state = 3;
                c.st.questsDone++;
                cout << "Квест здано! Нагорода: " << q.rewardGold << " золота, " << q.rewardExp << " досвіду.\n";
                addGold(c, q.rewardGold);
                gainExp(c, q.rewardExp);
            } else {
                cout << "Цей квест ще не виконано.\n";
            }
        }
    }
}

const string SAVE_FILE = "savegame.txt";

string itemToStr(const Item& it) {
    ostringstream o;
    o << it.name << "|" << (int)it.type << "|" << it.value << "|" << it.price << "|" << it.classReq;
    return o.str();
}

bool strToItem(const string& s, Item& it) {
    auto p = split(s, '|');
    if (p.size() != 5) return false;
    try {
        it.name = p[0];
        it.type = (ItemType)stoi(p[1]);
        it.value = stoi(p[2]);
        it.price = stoi(p[3]);
        it.classReq = stoi(p[4]);
    } catch (...) {
        return false;
    }
    return true;
}

bool saveGame(const Character& c) {
    ofstream f(SAVE_FILE);
    if (!f) return false;
    f << "RPG_SAVE_V1\n";
    f << c.name << "\n";
    f << (int)c.cls << " " << c.level << " " << c.exp << " " << c.expNext << "\n";
    f << c.maxHp << " " << c.hp << " " << c.maxMana << " " << c.mana << " " << c.strength << " "
      << c.baseDef << " " << c.gold << "\n";
    const Stats& s = c.st;
    f << s.battles << " " << s.monstersKilled << " " << s.bossesKilled << " " << s.deaths << " " << s.fled
      << " " << s.crits << " " << s.damageDealt << " " << s.damageTaken << " " << s.maxHit << " "
      << s.goldEarned << " " << s.goldSpent << " " << s.potionsUsed << " " << s.questsDone << "\n";
    f << (c.hasWeapon ? itemToStr(c.weapon) : "-") << "\n";
    f << (c.hasArmor ? itemToStr(c.armor) : "-") << "\n";
    f << c.inv.size() << "\n";
    for (const auto& it : c.inv) f << itemToStr(it) << "\n";
    f << c.quests.size() << "\n";
    for (const auto& q : c.quests) f << q.state << " " << q.progress << "\n";
    return (bool)f;
}

bool loadGame(Character& out) {
    ifstream f(SAVE_FILE);
    if (!f) return false;
    Character c;
    string l;
    if (!getline(f, l) || l != "RPG_SAVE_V1") return false;
    if (!getline(f, c.name)) return false;

    int cls;
    if (!(f >> cls >> c.level >> c.exp >> c.expNext)) return false;
    if (cls < 0 || cls > 2) return false;
    c.cls = (ClassType)cls;
    if (!(f >> c.maxHp >> c.hp >> c.maxMana >> c.mana >> c.strength >> c.baseDef >> c.gold)) return false;
    Stats& s = c.st;
    if (!(f >> s.battles >> s.monstersKilled >> s.bossesKilled >> s.deaths >> s.fled >> s.crits >>
          s.damageDealt >> s.damageTaken >> s.maxHit >> s.goldEarned >> s.goldSpent >> s.potionsUsed >>
          s.questsDone))
        return false;
    getline(f, l);

    getline(f, l);
    if (l != "-") {
        if (!strToItem(l, c.weapon)) return false;
        c.hasWeapon = true;
    }
    getline(f, l);
    if (l != "-") {
        if (!strToItem(l, c.armor)) return false;
        c.hasArmor = true;
    }
    size_t n;
    if (!(f >> n) || n > 1000) return false;
    getline(f, l);
    for (size_t i = 0; i < n; i++) {
        Item it;
        if (!getline(f, l) || !strToItem(l, it)) return false;
        c.inv.push_back(it);
    }
    size_t qn;
    if (!(f >> qn)) return false;
    for (size_t i = 0; i < qn; i++) {
        int st, pr;
        if (!(f >> st >> pr)) return false;
        if (i < c.quests.size()) {
            c.quests[i].state = st;
            c.quests[i].progress = pr;
        }
    }
    out = c;
    return true;
}

void rest(Character& c) {
    const int cost = 20;
    if (c.gold < cost) {
        cout << "Нічліг коштує " << cost << " золота. У вас замало грошей.\n";
        return;
    }
    c.gold -= cost;
    c.st.goldSpent += cost;
    c.hp = c.maxHp;
    c.mana = c.maxMana;
    cout << "Ви відпочили в таверні. HP і мана повністю відновлені.\n";
}

void gameLoop(Character& c) {
    while (true) {
        line();
        cout << c.name << " | " << className(c.cls) << " | рів. " << c.level << " | HP " << c.hp << "/" << c.maxHp
             << " | Мана " << c.mana << "/" << c.maxMana << " | Золото " << c.gold << "\n";
        line();
        cout << "1. Статистика персонажа\n";
        cout << "2. Інвентар\n";
        cout << "3. Магазин\n";
        cout << "4. Битва з монстром\n";
        cout << "5. Битва з босом\n";
        cout << "6. Квести\n";
        cout << "7. Відпочити в таверні (20 зол.)\n";
        cout << "8. Зберегти гру\n";
        cout << "9. Завантажити гру\n";
        cout << "0. Вийти в головне меню\n";
        int ch = readInt(0, 9);
        switch (ch) {
            case 1: showStats(c); pauseEnter(); break;
            case 2: inventoryMenu(c); break;
            case 3: shopMenu(c); break;
            case 4: battle(c, makeMonster(c.level)); pauseEnter(); break;
            case 5: bossMenu(c); pauseEnter(); break;
            case 6: questMenu(c); break;
            case 7: rest(c); break;
            case 8:
                if (saveGame(c)) cout << "Гру збережено у " << SAVE_FILE << ".\n";
                else cout << "Помилка збереження!\n";
                break;
            case 9: {
                Character tmp;
                if (loadGame(tmp)) {
                    c = tmp;
                    cout << "Гру завантажено. З поверненням, " << c.name << "!\n";
                } else {
                    cout << "Не вдалося завантажити збереження.\n";
                }
                break;
            }
            case 0: return;
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    while (true) {
        line('=');
        cout << "         КОНСОЛЬНА RPG\n";
        line('=');
        cout << "1. Нова гра\n2. Завантажити гру\n0. Вихід\n";
        int ch = readInt(0, 2);
        if (ch == 0) {
            cout << "До зустрічі!\n";
            return 0;
        }
        Character player;
        if (ch == 1) {
            player = createCharacter();
        } else if (!loadGame(player)) {
            cout << "Збереження не знайдено або пошкоджене.\n";
            continue;
        } else {
            cout << "Гру завантажено. З поверненням, " << player.name << "!\n";
        }
        gameLoop(player);
    }
}