// M2_FoodInventory.h
// Member 2 - Food Inventory, Hygiene and Decay
// Data Structure : Doubly Linked List (ActiveFoodList)
// File Handling  : food.txt , one item per line, tag says cooked or packaged
// Feature F1 : Usable Shelf Life     Feature F4 : Risk Hygiene Tier
// Feature S2 : Storage Temperature Decay Multiplier
#ifndef M2_FOODINVENTORY_H
#define M2_FOODINVENTORY_H
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cstdio>      // FILE, fopen, fprintf, fgets - C file handling
using namespace std;

// ================= PART B : C++ OOP PART =================
// Feature S2 : where the food is kept changes how fast it spoils. a tray
// left on a counter goes off far quicker than the same tray in a cold
// room. the multiplier scales the decay rate rather than replacing it,
// so covered food in a cold room is still better off than open food in
// the same cold room.
#define STORE_ROOM    1
#define STORE_CHILLED 2
#define STORE_FROZEN  3

class StorageCondition
{
private:
    int kind;
public:
    StorageCondition() { kind = STORE_ROOM; }
    // anything we do not recognise is treated as room temperature,
    // which is the worst case, so a bad value can never make food
    // look safer than it is
    StorageCondition(int k)
    {
        kind = (k < STORE_ROOM || k > STORE_FROZEN) ? STORE_ROOM : k;
    }
    int getKind() { return kind; }
    // how much of the normal spoiling still happens
    float getFactor()
    {
        if (kind == STORE_CHILLED) return 0.5;
        if (kind == STORE_FROZEN)  return 0.2;
        return 1.0;
    }
    const char* getName()
    {
        if (kind == STORE_CHILLED) return "cold room";
        if (kind == STORE_FROZEN)  return "freezer";
        return "room temp";
    }
};

// abstract class, cannot make an object of it
class FoodItem
{
protected:
    char id[20];
    char name[40];
    char donorId[20];
    int quantity;
    int preparedHour;   // total hours value, not 0-23, so midnight is safe
    int shelfLife;
    int zone;
    StorageCondition store;     // Feature S2, held by composition
public:
    FoodItem(const char *fid, const char *fname, const char *did,
             int qty, int prep, int life, int z, int st)
        : store(st)
    {
        strcpy(id, fid);  strcpy(name, fname);  strcpy(donorId, did);
        quantity = qty;  preparedHour = prep;  shelfLife = life;  zone = z;
    }
    virtual ~FoodItem() { }
    char* getId() { return id; }
    char* getName() { return name; }
    char* getDonorId() { return donorId; }
    int getQuantity() { return quantity; }
    int getShelfLife() { return shelfLife; }
    int getZone() { return zone; }
    int getStorage() { return store.getKind(); }
    const char* getStorageName() { return store.getName(); }
    float getStorageFactor() { return store.getFactor(); }
    // pure virtual, every child must write these
    virtual float getDecayRate() = 0;
    virtual const char* getType() = 0;
    virtual void display(int currentHour) = 0;
    // every kind of food writes its own kind of line. we save how
    // OLD the food is and not the hour it was made, because the
    // simulated clock starts again at Day 1 on every run.
    virtual void saveTo(FILE *f, int currentHour) = 0;
    // Feature S2 : the storage multiplier scales the decay rate
    float getRemainingLife(int currentHour)
    {
        int age = currentHour - preparedHour;
        return shelfLife - (age * getDecayRate() * store.getFactor());
    }
    // Feature F1 : life left after the van reaches the ngo
    float getUsableLife(int currentHour, int travelHours)
    {
        return getRemainingLife(currentHour) - travelHours;
    }
    int isDeliverable(int currentHour, int travelHours)
    {
        return (getUsableLife(currentHour, travelHours) > 0) ? 1 : 0;
    }
    // Feature F4 : 1 safe, 4 unsafe
    int getRiskTier(int currentHour)
    {
        float left = getRemainingLife(currentHour);
        if (left <= 0) return 4;
        float percent = (left * 100.0) / shelfLife;
        if (percent > 60) return 1;
        if (percent > 30) return 2;
        return 3;
    }
    const char* getTierName(int currentHour)
    {
        int t = getRiskTier(currentHour);
        if (t == 1) return "TIER 1 SAFE";
        if (t == 2) return "TIER 2 CAUTION";
        if (t == 3) return "TIER 3 SERVE NOW";
        return "TIER 4 UNSAFE";
    }
};

class CookedFood : public FoodItem
{
private:
    int isCovered;
public:
    CookedFood(const char *fid, const char *fname, const char *did,
               int qty, int prep, int life, int z, int cov, int st)
        : FoodItem(fid, fname, did, qty, prep, life, z, st) { isCovered = cov; }
    // open food spoils faster than covered food
    float getDecayRate() { return (isCovered == 1) ? 1.5 : 2.0; }
    const char* getType() { return "Cooked"; }
    void saveTo(FILE *f, int currentHour)
    {
        fprintf(f, "C|%s|%s|%s|%d|%d|%d|%d|%d|%d\n", id, name, donorId,
                quantity, currentHour - preparedHour, shelfLife, zone,
                isCovered, store.getKind());
    }
    void display(int currentHour)
    {
        cout << "  [COOKED] " << id << " | " << name << " | " << quantity
             << " meals | Zone " << zone << " | Left " << getRemainingLife(currentHour)
             << " hrs | " << getTierName(currentHour)
             << (isCovered ? " | covered" : " | OPEN")
             << " | " << store.getName() << endl;
    }
};

class PackagedFood : public FoodItem
{
private:
    int isSealed;
public:
    PackagedFood(const char *fid, const char *fname, const char *did,
                 int qty, int prep, int life, int z, int seal, int st)
        : FoodItem(fid, fname, did, qty, prep, life, z, st) { isSealed = seal; }
    // packet food lasts much longer than cooked food
    float getDecayRate() { return (isSealed == 1) ? 0.5 : 1.0; }
    const char* getType() { return "Packaged"; }
    void saveTo(FILE *f, int currentHour)
    {
        fprintf(f, "P|%s|%s|%s|%d|%d|%d|%d|%d|%d\n", id, name, donorId,
                quantity, currentHour - preparedHour, shelfLife, zone,
                isSealed, store.getKind());
    }
    void display(int currentHour)
    {
        cout << "  [PACKED] " << id << " | " << name << " | " << quantity
             << " meals | Zone " << zone << " | Left " << getRemainingLife(currentHour)
             << " hrs | " << getTierName(currentHour)
             << (isSealed ? " | sealed" : " | OPENED")
             << " | " << store.getName() << endl;
    }
};

// ============== PART A : C DATA STRUCTURE PART ==============
// doubly linked list, every node knows the one before and after it
struct FoodNode
{
    FoodItem *food;
    struct FoodNode *prev;
    struct FoodNode *next;
};

struct ActiveFoodList
{
    struct FoodNode *head;
    struct FoodNode *tail;
    int count;
};

void initList(struct ActiveFoodList *list)
{
    list->head = NULL;  list->tail = NULL;  list->count = 0;
}

void addFood(struct ActiveFoodList *list, FoodItem *f)
{
    struct FoodNode *newNode = (struct FoodNode *) malloc(sizeof(struct FoodNode));
    newNode->food = f;
    newNode->next = NULL;
    newNode->prev = list->tail;
    if (list->head == NULL) list->head = newNode;
    else list->tail->next = newNode;
    list->tail = newNode;
    list->count++;
}

// O(1) delete because the node knows its neighbours
void removeNode(struct ActiveFoodList *list, struct FoodNode *p)
{
    if (p->prev == NULL) list->head = p->next;
    else p->prev->next = p->next;
    if (p->next == NULL) list->tail = p->prev;
    else p->next->prev = p->prev;
    delete p->food;
    free(p);
    list->count--;
}

struct FoodNode* findFood(struct ActiveFoodList *list, const char *id)
{
    struct FoodNode *p = list->head;
    while (p != NULL)
    {
        if (strcmp(p->food->getId(), id) == 0) return p;
        p = p->next;
    }
    return NULL;
}

void freeList(struct ActiveFoodList *list)
{
    struct FoodNode *p = list->head;
    while (p != NULL)
    {
        struct FoodNode *temp = p;
        p = p->next;
        delete temp->food;
        free(temp);
    }
    list->head = NULL;  list->tail = NULL;  list->count = 0;
}

// print head to tail, display() is virtual
void displayFoodList(struct ActiveFoodList *list, int currentHour)
{
    cout << "\n  ---- ACTIVE INVENTORY (" << list->count << " items) ----" << endl;
    if (list->count == 0) { cout << "  inventory is empty" << endl;  return; }
    struct FoodNode *p = list->head;
    while (p != NULL) { p->food->display(currentHour);  p = p->next; }
}

// Feature F4 : group the inventory into risk tiers
void showRiskTiers(struct ActiveFoodList *list, int currentHour)
{
    const char *title[4] = { "TIER 1 SAFE (above 60% life)", "TIER 2 CAUTION (30-60%)",
                             "TIER 3 SERVE NOW (below 30%)", "TIER 4 UNSAFE (expired)" };
    cout << "\n  ---- HYGIENE RISK TIERS (Feature F4) ----" << endl;
    for (int tier = 1; tier <= 4; tier++)
    {
        cout << "\n  " << title[tier - 1] << endl;
        struct FoodNode *p = list->head;
        int found = 0;
        while (p != NULL)
        {
            if (p->food->getRiskTier(currentHour) == tier)
            {
                cout << "    " << p->food->getId() << " " << p->food->getName() << endl;
                found++;
            }
            p = p->next;
        }
        if (found == 0) cout << "    none" << endl;
    }
}

// Feature F1 : which food can survive the journey
void showUsableLife(struct ActiveFoodList *list, int currentHour, int travelHours)
{
    cout << "\n  ---- USABLE SHELF LIFE (Feature F1) ----" << endl;
    cout << "  Travel time to the NGO = " << travelHours << " hours" << endl;
    struct FoodNode *p = list->head;
    while (p != NULL)
    {
        float usable = p->food->getUsableLife(currentHour, travelHours);
        cout << "  " << p->food->getId() << " " << p->food->getName()
             << " : life " << p->food->getRemainingLife(currentHour)
             << " - travel " << travelHours << " = " << usable << " hrs -> "
             << (usable > 0 ? "CAN SEND" : "REJECT") << endl;
        p = p->next;
    }
}

int removeExpired(struct ActiveFoodList *list, int currentHour)
{
    struct FoodNode *p = list->head;
    int removed = 0;
    while (p != NULL)
    {
        struct FoodNode *nextOne = p->next;      // save it before deleting
        if (p->food->getRemainingLife(currentHour) <= 0)
        {
            cout << "  Removed expired : " << p->food->getId()
                 << " " << p->food->getName() << endl;
            removeNode(list, p);
            removed++;
        }
        p = nextOne;
    }
    return removed;
}

// 2) FILE HANDLING - the inventory is lost when the program closes, so
//    it is written out as plain text, one food item per line. the first
//    field is C for cooked or P for packaged, because loading has to
//    know which class to build before it can build it.
#define FOOD_FILE "food.txt"

int saveFood(struct ActiveFoodList *list, int currentHour)
{
    FILE *f = fopen(FOOD_FILE, "w");
    if (f == NULL)
    {
        cout << "  Could not open " << FOOD_FILE << " for writing." << endl;
        return -1;
    }
    int written = 0;
    struct FoodNode *p = list->head;
    while (p != NULL)
    {
        p->food->saveTo(f, currentHour);    // virtual, cooked or packaged
        written++;
        p = p->next;
    }
    fclose(f);
    cout << "  Food     : " << written << " saved to " << FOOD_FILE << endl;
    return written;
}

int loadFood(struct ActiveFoodList *list, int currentHour)
{
    FILE *f = fopen(FOOD_FILE, "r");
    if (f == NULL)
    {
        cout << "  Food     : no " << FOOD_FILE << " to read yet." << endl;
        return -1;
    }
    char line[200];
    int added = 0, skipped = 0;
    while (fgets(line, 200, f) != NULL)
    {
        if (line[0] == '\n') continue;              // blank line
        char *tag   = strtok(line, "|");
        char *id    = strtok(NULL, "|");
        char *name  = strtok(NULL, "|");
        char *donor = strtok(NULL, "|");
        char *qty   = strtok(NULL, "|");
        char *ago   = strtok(NULL, "|");
        char *life  = strtok(NULL, "|");
        char *zone  = strtok(NULL, "|");
        char *flag  = strtok(NULL, "|");
        char *stor  = strtok(NULL, "|");     // Feature S2
        if (tag == NULL || id == NULL || name == NULL || donor == NULL ||
            qty == NULL || ago == NULL || life == NULL || zone == NULL ||
            flag == NULL || stor == NULL)
        {
            skipped++;
            continue;
        }
        if (findFood(list, id) != NULL) { skipped++;  continue; }   // already here
        // the prepared time is rebuilt from how old the food was when
        // it was saved, so reloading does not make it younger
        int prep = currentHour - atoi(ago);
        if (tag[0] == 'C')
            addFood(list, new CookedFood(id, name, donor, atoi(qty), prep,
                                         atoi(life), atoi(zone), atoi(flag),
                                         atoi(stor)));
        else if (tag[0] == 'P')
            addFood(list, new PackagedFood(id, name, donor, atoi(qty), prep,
                                           atoi(life), atoi(zone), atoi(flag),
                                           atoi(stor)));
        else { skipped++;  continue; }
        added++;
    }
    fclose(f);
    cout << "  Food     : " << added << " loaded";
    if (skipped > 0) cout << " , " << skipped << " skipped (already there or bad line)";
    cout << endl;
    return added;
}

// ================= MENU FUNCTIONS =================
void addFoodMenu(struct ActiveFoodList *list, int currentHour)
{
    char id[20], name[40], donorId[20];
    int type, qty, ago, life, zone, flag, store;
    cout << "\n  --- ADD FOOD DONATION ---" << endl;
    cout << "  Food ID  : ";  cin >> setw(20) >> id;
    if (findFood(list, id) != NULL) { cout << "  ID already used." << endl;  return; }
    cout << "  Donor ID : ";  cin >> setw(20) >> donorId;
    cin.ignore();
    cout << "  Food name : ";                      cin.getline(name, 40);
    cout << "  Type (1=Cooked 2=Packaged) : ";     cin >> type;
    cout << "  Quantity (meals) : ";               cin >> qty;
    cout << "  Prepared how many hours ago : ";    cin >> ago;
    cout << "  Shelf life (hours) : ";             cin >> life;
    cout << "  Zone (0-5) : ";                     cin >> zone;
    cout << "  Stored (1=room 2=cold room 3=freezer) : ";  cin >> store;
    // shelf life must be above zero, otherwise the tier percentage
    // divides by zero and expired food can be reported as safe
    if (type < 1 || type > 2 || qty <= 0 || ago < 0 || life <= 0 ||
        zone < 0 || zone >= 6 || store < 1 || store > 3)
        {
        cout << "  Wrong value entered, food not added." << endl;
        return;
    }
    if (type == 1)
    {
        cout << "  Kept covered (1=yes 0=no) : ";  cin >> flag;
        flag = (flag == 1) ? 1 : 0;      // anything else counts as not covered
        addFood(list, new CookedFood(id, name, donorId, qty,
                                     currentHour - ago, life, zone,
                                     flag, store));
    } else
    {
        cout << "  Packet sealed (1=yes 0=no) : ";  cin >> flag;
        flag = (flag == 1) ? 1 : 0;      // anything else counts as not sealed
        addFood(list, new PackagedFood(id, name, donorId, qty,
                                       currentHour - ago, life, zone,
                                       flag, store));
    }
    cout << "\n  Food added to inventory." << endl;
}

void checkFoodMenu(struct ActiveFoodList *list, int currentHour)
{
    char id[20];
    int travel;
    cout << "\n  --- CHECK ONE FOOD ITEM ---" << endl;
    cout << "  Food ID : ";  cin >> setw(20) >> id;
    struct FoodNode *p = findFood(list, id);
    if (p == NULL) { cout << "  Food not found." << endl;  return; }
    cout << "  Travel hours to NGO : ";  cin >> travel;
    FoodItem *f = p->food;
    cout << "\n  Name        : " << f->getName() << endl;
    cout << "  Type        : " << f->getType() << endl;
    cout << "  Decay rate  : " << f->getDecayRate() << endl;
    cout << "  Life left   : " << f->getRemainingLife(currentHour) << " hrs" << endl;
    cout << "  Usable life : " << f->getUsableLife(currentHour, travel) << " hrs" << endl;
    cout << "  Risk tier   : " << f->getTierName(currentHour) << endl;
    cout << "  Decision    : "
         << (f->isDeliverable(currentHour, travel) ? "CAN SEND" : "REJECT") << endl;
}

// sample data, prepared times counted back from now
void loadDemoFood(struct ActiveFoodList *list, int currentHour)
{
    // Feature S2 : the last number is where it is kept.
    // 1 room temperature, 2 cold room, 3 freezer.
    addFood(list, new CookedFood("F201", "Veg Biryani", "D101", 50, currentHour - 4, 8, 1, 1, 1));
    addFood(list, new CookedFood("F202", "Dal Rice", "D102", 80, currentHour - 6, 8, 3, 0, 1));
    addFood(list, new PackagedFood("F203", "Bread Packets", "D103", 100, currentHour - 8, 48, 4, 1, 1));
    addFood(list, new CookedFood("F204", "Paneer Curry", "D101", 30, currentHour - 2, 6, 1, 1, 2));
    addFood(list, new PackagedFood("F205", "Milk Cartons", "D104", 40, currentHour - 9, 24, 0, 0, 2));
    cout << "  Demo food loaded (3 cooked, 2 packaged)." << endl;
}

#endif
